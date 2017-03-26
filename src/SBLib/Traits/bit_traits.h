#pragma once
#include <Algorithms/static_for_each.h>
namespace SBLib::Traits
{
template<size_t bit_mask>
struct bit_traits
{
private:
	enum : size_t
	{
		first_bit        = (((bit_mask - 1) & ~bit_mask) + 1),
		remaining_bits   = (bit_mask & ~first_bit),
	};
public:
	enum : size_t
	{
		bit_mask         = bit_mask,
		population_count = 1 + bit_traits<remaining_bits>::population_count,
	};

	template<size_t index>
	static constexpr size_t get_bit()
	{
		return bit_traits<remaining_bits>::get_bit<index - 1>();
	}
	template<>
	static constexpr size_t get_bit<0>()
	{
		return first_bit;
	}

	template<size_t bit_value>
	static constexpr size_t get_bit_component()
	{
		return bit_traits<remaining_bits>::get_bit_component<bit_value>() + 1;
	}
	template<>
	static constexpr size_t get_bit_component<first_bit>()
	{
		return 0;
	}

	template<size_t bit_value>
	static constexpr size_t get_bit_index()
	{
		return bit_traits<bit_value - 1>::population_count;
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

	template<size_t index>
	static constexpr size_t get_bit()
	{
		return 0;
	}
	template<size_t bit_value>
	static constexpr size_t get_bit_component()
	{
		return 0;
	}
	template<size_t index>
	static constexpr size_t get_bit_index()
	{
		return 0;
	}
};

template<size_t bit_mask>
struct get_bit_helper
{
	template<size_t index>
	static constexpr size_t get()
	{
		return bit_traits<bit_mask>::get_bit<index>();
	}
};
template<size_t bit_mask>
struct get_bit_component_helper
{
public:
	template<size_t bit>
	static constexpr size_t get()
	{
		return bit_traits<bit_mask>::get_bit_component<bit>();
	}
};
template<size_t bit_mask>
struct get_bit_index_helper
{
public:
	template<size_t bit>
	static constexpr size_t get()
	{
		return bit_traits<bit_mask>::get_bit_index<bit>();
	}
};
template<size_t bit_mask>
struct next_bit_helper
{
	template<size_t bit>
	static constexpr size_t increment()
	{
		enum : size_t { next_index = bit_traits<bit_mask>::get_bit_component<bit>() + 1, };
		return bit_traits<bit_mask>::get_bit<next_index>();
	}
};

template<size_t bit_mask> struct for_each_bit : static_for_each<0, bit_traits<bit_mask>::population_count, get_bit_helper<bit_mask>> {};
template<size_t bit_mask> struct for_each_bit_compoment : static_for_each<bit_traits<bit_mask>::get_bit<0>(), bit_traits<bit_mask>::get_bit<bit_traits<bit_mask>::population_count>(), get_bit_component_helper<bit_mask>, next_bit_helper<bit_mask>> {};
template<size_t bit_mask> struct for_each_bit_index : static_for_each<bit_traits<bit_mask>::get_bit<0>(), bit_traits<bit_mask>::get_bit<bit_traits<bit_mask>::population_count>(), get_bit_index_helper<bit_mask>, next_bit_helper<bit_mask>> {};
} // namespace SBLib::Traits
namespace SBLib { using namespace Traits; }
