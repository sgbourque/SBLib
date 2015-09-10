#pragma once

//
// binomial_coefficient
//
template<int dimension_size, int rank_size>
struct binomial_coefficient
{
private:
	// binomial reflection identities
	enum
	{
		omega_0 = rank_size - dimension_size - 1,
		omega_1 = -1 - rank_size,
		omega_2 = -1 - dimension_size,
		sigma_0 = +1,
		sigma_1 = (rank_size & 1) == 0 ? +1 : -1,
		sigma_2 = ((dimension_size - rank_size) & 1) == 0 ? +1 : -1,
		
		minimal_dimension = (omega_0 >= 0) ? omega_0   : omega_1,
		minimal_rank      = (omega_0 >= 0) ? rank_size : omega_2,
		minimal_sign      = (omega_0 >= 0) ? sigma_1   : sigma_2,
		n    = (dimension_size >= 0) ? dimension_size : minimal_dimension,
		k    = (dimension_size >= 0) ? rank_size : minimal_rank,
		sign = (dimension_size >= 0) ? sigma_0   : minimal_sign,

		zero_filter = (n >= 0) && ((k < 0) || (k > n)) ? 0 : +1,
	};

public:
	// using Pascal triangle construction
	enum
	{
		value = zero_filter * sign * (
		           binomial_coefficient<n - 1, k - 1>::value
		         + binomial_coefficient<n - 1, k    >::value
		        )
	};
};

template<>
struct binomial_coefficient<0, 0>
{
	enum { value = 1 };
};

template<int dimension_size>
struct binomial_coefficient<dimension_size, 0>
{
	enum { value = 1 };
};

template<int dimension_size>
struct binomial_coefficient<dimension_size, dimension_size>
{
	enum { value = 1 };
};

template<>
struct binomial_coefficient<-1, 0>
{
public:
	enum { value = 1 };
};

template<>
struct binomial_coefficient<-1, -1>
{
public:
	enum { value = 1 };
};

template<int rank_size>
struct binomial_coefficient<-1, rank_size>
{
private:
	enum
	{
		rank_mod_2 = (rank_size & 1) == 0 ? +1 : 0,
	};
public:
	enum
	{
		value = (rank_size >= 0) ? 2 * rank_mod_2 - 1 : 1 - 2 * rank_mod_2,
	};
};
