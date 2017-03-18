#pragma once
#include <Algorithms/static_for_each.h>

template<size_t bit_set>
struct bit_traits
{
private:
	enum : size_t
	{
		first_bit        = (((bit_set - 1) & ~bit_set) + 1),
		remaining_bits   = (bit_set & ~first_bit),
	};
public:
	enum : size_t
	{
		value            = bit_set,
		population_count = 1 + bit_traits<remaining_bits>::population_count,
	};

	template<size_t index>
	struct get_bit
	{
		enum : size_t
		{
			value = bit_traits<remaining_bits>::get_bit<index - 1>::value,
		};
	};
	template<>
	struct get_bit<0>
	{
		enum : size_t
		{
			value = first_bit,
		};
	};

	template<size_t bit>
	struct bit_index
	{
		enum : size_t
		{
			value = 1 + bit_traits<remaining_bits>::bit_index<bit>::value,
		};
	};
	template<>
	struct bit_index<first_bit>
	{
		enum : size_t
		{
			value = 0,
		};
	};

	template<size_t index>
	static constexpr size_t get_set_bit()
	{
		return get_bit<index>::value;
	}
	template<size_t bit>
	static constexpr size_t get_bit_index()
	{
		return bit_index<bit>::value;
	}
};
template<>
struct bit_traits<0>
{
public:
	enum : size_t
	{
		value = 0,
		population_count = 0,
	};

	template<size_t>
	struct get_bit
	{
		enum : size_t
		{
			value = 0,
		};
	};
	template<size_t bit>
	struct bit_index
	{
	};

	template<size_t index>
	static constexpr size_t get_set_bit()
	{
		return set_bit<index>::value;
	}
	template<size_t bit>
	static constexpr size_t get_bit_index()
	{
		return bit_index<bit>::value;
	}
};

template<class traits>
struct get_bit_helper
{
	template<size_t index>
	struct get_helper
	{
		enum : size_t
		{
			value = traits::get_bit<index>::value,
		};
	};

	template<size_t index>
	static constexpr size_t get()
	{
		return get_helper<index>::value;
	}
};
template<class traits>
struct increment_bit_index_helper
{
	template<size_t index>
	struct increment_helper
	{
		enum : size_t
		{
			value = index + 1,
		};
	};

	template<size_t index>
	static constexpr size_t increment()
	{
		return increment_helper<index>::value;
	}
};

template<class traits>
struct get_bit_index_helper
{
	template<size_t bit>
	struct get_helper
	{
		enum : size_t
		{
			value = traits::bit_index<bit>::value,
		};
	};

	template<size_t bit>
	static constexpr size_t get()
	{
		return get_helper<bit>::value;
	}
};
template<class traits>
struct next_bit_helper
{
	template<size_t bit>
	struct increment_helper
	{
	private:
		enum : size_t
		{
			next_index = traits::bit_index<bit>::value + 1,
		};
	public:
		enum : size_t
		{
			value = traits::get_bit<next_index>::value,
		};
	};

	template<size_t bit>
	static constexpr size_t increment()
	{
		return increment_helper<bit>::value;
	}
};

template<typename bit_traits_t> struct for_each_bit : static_for_each<0, bit_traits_t::population_count, get_bit_helper<bit_traits_t>, increment_bit_index_helper<bit_traits_t>> {};
template<typename bit_traits_t> struct for_each_bit_index
{
	template<template<size_t, size_t> class fct_type, typename... type_t>
	static void iterate(type_t&&... types)
	{
		static_for_each<bit_traits_t::get_bit<0>::value, bit_traits_t::get_bit<bit_traits_t::population_count>::value, get_bit_index_helper<bit_traits_t>, next_bit_helper<bit_traits_t>>::iterate<fct_type>(types...);
	}
};
