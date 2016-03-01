#pragma once

#include <Traits/bit_traits.h>

//
// default_basis_big_endian
//
// if true, then multivector basis defaults to big endian, for instance
//     7 ~ (e0 ^ e1 ^ e2)
// if false, multivector basis defaults to little endian, for instance
//     7 ~ (e2 ^ e1 ^ e0)
//
static const bool default_basis_big_endian = true;

template<size_t first, size_t second, bool big_endian = default_basis_big_endian>
struct alternating_traits
{
private:
	typedef bit_traits<first>  first_traits;
	typedef bit_traits<second> second_traits;
	enum : size_t
	{
		second_population        = second_traits::population_count,
		second_permute_bit_index = (big_endian ? 0 : second_population - 1),
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

template<size_t bit_set>
struct reversion_conjugacy_traits
{
private:
	typedef bit_traits<bit_set> traits;
public:
	enum : int
	{
		sign = (traits::population_count & 2) == 2 ? -1 : +1,
	};
};

template<size_t bit_set>
struct grade_conjugacy_traits
{
private:
	typedef bit_traits<bit_set> traits;
public:
	enum : int
	{
		sign = (traits::population_count & 1) == 1 ? -1 : +1,
	};
};

template<size_t bit_set>
struct clifford_adjoint_conjugacy_traits
{
private:
	typedef bit_traits<bit_set> traits;
public:
	enum : int
	{
		sign = grade_conjugacy_traits<bit_set>::sign * reversion_conjugacy_traits<bit_set>::sign,
	};
};

template<size_t bit_set, size_t mask, bool big_endian = default_basis_big_endian>
struct hodge_conjugacy_traits
{
private:
	enum : size_t
	{
		parallel_projection      = (bit_set & mask),
		perpendicular_projection = (bit_set & ~parallel_projection),
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
};
