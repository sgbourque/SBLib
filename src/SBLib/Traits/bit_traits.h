#pragma once
#include <Algorithms/static_for_each.h>
#include <type_traits>
namespace SBLib::Traits
{
namespace
{
	template<class T> constexpr T integer_log2(T value) noexcept { return (value > 1) ? 1 + integer_log2((value >> 1)) : 0; }
	template<class T> constexpr T half_bits_mask(T iteration) noexcept { return (T(1) << (T(1) << iteration)) - T(1); }
	template<class T> constexpr T parallel_bits_mask(T iteration) noexcept
	{
		static_assert(std::is_unsigned_v<T>, "bit mask must be unsigned");
		constexpr T all_bits = T(~T(0));
		T parallel_mask = T(T(1) << (iteration + 1)) < sizeof(T) * CHAR_BIT ? T(all_bits / half_bits_mask(iteration + 1)) : T(1);
		return half_bits_mask(iteration) * parallel_mask;
	}
	template<class T>
	constexpr T parallel_bit_count(T mask, T iteration) noexcept
	{
		const auto parallel_mask = parallel_bits_mask(iteration);
		return ((mask >> (T(0) << iteration)) & parallel_mask) + ((mask >> (T(1) << iteration)) & parallel_mask);
	}
} // anonymous namespace

template<class T>
constexpr auto bit_count(T mask) noexcept -> std::make_unsigned_t<T>
{
	using type = std::make_unsigned_t<T>;
	type bit_mask = static_cast<type>(mask);
	constexpr auto max_iteration = integer_log2( sizeof(T) * CHAR_BIT );
	for ( type iteration = 0; iteration < max_iteration; ++iteration )
		bit_mask = parallel_bit_count(bit_mask, iteration);
	return bit_mask;
}


template< size_t bit_index, typename T, typename = std::enable_if_t<bit_index == 0> >
constexpr size_t get_bit( T bit_mask ) noexcept
{
	return (bit_mask & ~(bit_mask - 1));
}
template< size_t bit_index, typename T >
constexpr size_t get_bit( T bit_mask, std::enable_if_t<bit_index != 0, void*> = nullptr ) noexcept
{
	return get_bit<bit_index - 1>( bit_mask - (bit_mask & ~(bit_mask - 1)) );
}

template< size_t bit_value, typename T >
static constexpr T get_bit_component( T bit_mask )
{
	static_assert( std::is_convertible_v<T, size_t> );
	T first_bit = T(bit_mask & ~(bit_mask - 1));
	return (bit_value == first_bit) ? T(0) : T(get_bit_component<bit_value>( T(bit_mask & ~first_bit) ) + 1);
}

template< typename T >
static constexpr T get_bit_index( T bit_value )
{
	return bit_count(bit_value - 1);
}

template<size_t bit_mask> struct bit_traits : std::integral_constant<size_t, bit_mask> {};

template<size_t bit_mask>
struct get_bit_helper
{
	template<size_t index>
	static constexpr size_t get()
	{
		return get_bit<index>(bit_mask);
	}
};
template<size_t bit_mask>
struct get_bit_component_helper
{
public:
	template<size_t bit>
	static constexpr size_t get()
	{
		return get_bit_component<bit>(bit_mask);
	}
};
template<size_t bit_mask>
struct get_bit_index_helper
{
public:
	template<size_t bit>
	static constexpr size_t get()
	{
		static_assert( (bit & (bit - 1)) == 0, "invalid bit" );
		return get_bit_index(bit);
	}
};
template<size_t bit_mask>
struct next_bit_helper
{
	template<size_t bit>
	static constexpr size_t increment()
	{
		enum : size_t { next_index = get_bit_component<bit>(bit_mask) + 1, };
		return get_bit<next_index>(bit_mask);
	}
};

template<size_t bit_mask> struct for_each_bit : static_for_each<0, bit_count(bit_mask), get_bit_helper<bit_mask>> {};
template<size_t bit_mask> struct for_each_bit_compoment : static_for_each<get_bit<0>(bit_mask), get_bit<bit_count(bit_mask)>(bit_mask), get_bit_component_helper<bit_mask>, next_bit_helper<bit_mask>> {};
template<size_t bit_mask> struct for_each_bit_index : static_for_each<get_bit<0>(bit_mask), get_bit<bit_count(bit_mask)>(bit_mask), get_bit_index_helper<bit_mask>, next_bit_helper<bit_mask>> {};
} // namespace SBLib::Traits
namespace SBLib { using namespace Traits; }
