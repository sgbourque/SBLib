#pragma once

#include <Traits/bit_traits.h>

// Clifford Traits
//
// Herein, multivector blades are represented as ordered bitsets in Euclidian space.
// For instance scalar is 0, vectors are 1-bits numbers, bivectors are 2-bits, etc.
// Default ordering is big endian : that is, for instance, (e0^e1) is selected over (e1^e0).
//
// c.f., clifford_traits_tests.cpp for examples.
//


//
// default_basis_big_endian
// Defines default ordering of blades.
// For instance, if true, then multivector basis defaults to big endian so
//     7 ~ (e0 ^ e1 ^ e2)
// while if false, multivector basis defaults to little endian, so we would rather have
//     7 ~ (e2 ^ e1 ^ e0) = -(e0 ^ e1 ^ e2)
//
static const bool default_basis_big_endian = true;


//
// alternating_traits
// Calculates the residual sign and bit_set the wedge product of two blades.
// If the wedge product is zero, both sign and bit_set are 0.
//
// For instance, the result of (e1^e0) = -(e0^e1) is reflected by :
//	alternating_traits<(1 << 1), (1 << 0)>::sign    == -1;
//	alternating_traits<(1 << 1), (1 << 0)>::bit_set ==  (1 << 0) | (1 << 0);
// and
//	alternating_traits<(1 << 0), (1 << 1)>::sign    == +1;
//	alternating_traits<(1 << 0), (1 << 1)>::bit_set ==  (1 << 0) | (1 << 0);
//
// A more complex exemple would be (e0^e2^e4)^(e1^e3) = -(e0^e1^e2^e3^e4) :
//	alternating_traits<(1 << 0)|(1 << 2)|(1 << 4), (1 << 1)|(1 << 3)>::sign    == -1;
//	alternating_traits<(1 << 0)|(1 << 2)|(1 << 4), (1 << 1)|(1 << 3)>::bit_set == (1 << 0)|(1 << 1)|(1 << 2)|(1 << 3)|(1 << 4);
//
template<size_t first, size_t second, bool big_endian = default_basis_big_endian>
struct alternating_traits
{
private:
	typedef bit_traits<first>  first_traits;
	typedef bit_traits<second> second_traits;
	enum : size_t
	{
		second_grade             = second_traits::population_count,
		second_permute_bit_index = (big_endian ? 0 : second_grade - 1),
		second_permute_bit       = second_traits::get_bit<second_permute_bit_index>::value,

		low_bit_mask  = (second_permute_bit << 1) - 1,
		high_bit_mask = ~low_bit_mask,
		permute_bit_mask              = (first & (big_endian ? high_bit_mask : low_bit_mask)),
		permute_bit_permutation_count = bit_traits<permute_bit_mask>::population_count,

		next_first  = (first | second_permute_bit),
		next_second = (second & ~second_permute_bit),
	};
	enum : int
	{
		last_bit_sign = (permute_bit_permutation_count & 1) != 0 ? -1 : +1,
		residual_sign = alternating_traits<next_first, next_second, big_endian>::sign,
	};
public:
	enum : int
	{
		sign = (first & second) != 0 ? 0 : last_bit_sign * residual_sign,
	};
	enum : size_t
	{
		bit_set = (first & second) != 0 ? 0 : (first | second),
	};
};
template<size_t first, bool big_endian>
struct alternating_traits<first, 0, big_endian>
{
public:
	enum : int
	{
		sign = +1,
	};
	enum : size_t
	{
		bit_set = first,
	};
};
template<size_t second, bool big_endian>
struct alternating_traits<0, second, big_endian>
{
public:
	enum : int
	{
		sign = +1,
	};
	enum : size_t
	{
		bit_set = second,
	};
};
template<bool big_endian>
struct alternating_traits<0, 0, big_endian>
{
public:
	enum : int
	{
		sign = +1,
	};
	enum : size_t
	{
		bit_set = 0,
	};
};


//
// reversion_conjugacy_traits
// Calculate the residual sign after a full blade reversion conjugacy operation B -> B^T (e.g., by passing from big to little endian).
// For instance, reversion of (e0^e1^e2) gives (e2^e1^e0) = -(e0^e1^e2) so reversion_conjugacy_traits<7> == -1.
//
template<size_t in_bit_set, bool big_endian = default_basis_big_endian>
struct reversion_conjugacy_traits
{
private:
	typedef bit_traits<in_bit_set> traits;
public:
	enum : size_t
	{
		grade = traits::population_count,
	};
	enum : int
	{
		sign = (grade & 2) == 2 ? -1 : +1,
	};
};


//
// grade_conjugacy_traits
// Calculate the residual sign after a full parity transformation e -> -e on a blade B -> B*.
//
template<size_t in_bit_set, bool big_endian = default_basis_big_endian>
struct grade_conjugacy_traits
{
private:
	typedef bit_traits<in_bit_set> traits;
public:
	enum : size_t
	{
		grade = traits::population_count,
	};
	enum : int
	{
		sign = (grade & 1) == 1 ? -1 : +1,
	};
};


//
// clifford_adjoint_conjugacy_traits
// Calculate the residual sign after a Clifford adjoint operation on a blade B -> B^t = (B*)^T = (B^T)*.
// The Clifford adjoint is the result of both blade reversion and grade conjugation.
//
template<size_t in_bit_set, bool big_endian = default_basis_big_endian>
struct clifford_adjoint_conjugacy_traits
{
private:
	typedef bit_traits<in_bit_set> traits;
public:
	enum : size_t
	{
		grade = traits::population_count,
	};
	enum : int
	{
		sign = grade_conjugacy_traits<in_bit_set, big_endian>::sign * reversion_conjugacy_traits<in_bit_set, big_endian>::sign,
	};
};


//
// hodge_conjugacy_traits
// Given a blade and a space, calculates the hodge dual orthogonal to the projected blade onto that space : B -> *B.
// If the space does not completely embeds the blade, all orthogonal parts of the blade are considered dual to scalars.
// For instance, if (e0,e1,e2) spans the space, the hodge duals of e0, e1 and e2 are (in big endian) :
//	*e0 = +(e1^e2),
//	*e1 = -(e0^e2),
//	*e2 = +(e0^e1).
// In general, hodge duals satisfies B ^ *B = *1.
//
template<size_t in_bit_set, size_t mask, bool big_endian = default_basis_big_endian>
struct hodge_conjugacy_traits
{
private:
	enum : size_t
	{
		parallel_projection      = (in_bit_set & mask),
		perpendicular_projection = (in_bit_set & ~parallel_projection),
		hodge_complement         = (mask & ~parallel_projection),
	};
	enum : int
	{
		hodge_sign         = alternating_traits<parallel_projection, hodge_complement, big_endian>::sign,
		perpendicular_sign = alternating_traits<perpendicular_projection, parallel_projection, big_endian>::sign,
		projection_sign    = +1,//alternating_traits<perpendicular_projection, hodge_complement, big_endian>::sign,
	};
public:
	enum : int
	{
		sign = hodge_sign * perpendicular_sign * projection_sign,
	};
	enum : size_t
	{
		//bit_set = (perpendicular_projection | hodge_complement),
		bit_set = hodge_complement,
	};
	enum : size_t
	{
		grade = bit_traits<bit_set>::population_count,
	};
	static_assert( grade == bit_traits<mask>::population_count - bit_traits<parallel_projection>::population_count, "Incorrect grade!" );
};
