#pragma once

namespace SBLib::Algorithms
{
namespace
{
	enum : size_t { max_counter_token_count = 128, max_counter_token_count_vs2017 = ((512 - 1) & ~15), };
	static_assert( max_counter_token_count <= max_counter_token_count_vs2017, "VC 2017 compiler won't let you use that much counters. You should use __COUNTER__ instead." );

	template<size_t counter_value = max_counter_token_count> struct counter_token {};
	template<size_t counter_value> struct counter_constant : std::integral_constant<size_t, counter_value>
	{
		inline constexpr friend size_t get_counter_value(counter_token<counter_value>)
		{
			return counter_value;
		}
	};

	template<int try_counter_value>
	inline constexpr size_t get_counter_value(counter_token<try_counter_value>, size_t value = get_counter_value(counter_token<try_counter_value - 1>()))
	{
		return value;
	}
	inline constexpr size_t get_counter_value(counter_token<0>)
	{
		return 0;
	}
} // anonymous namespace

template<int current_value = get_counter_value(counter_token<>())>
inline constexpr size_t get_counter()
{
	constexpr auto value = counter_constant<current_value + 1>::value - 1; // force compiler to define next counter_constant
	static_assert(value + 1 <= max_counter_token_count, "Max counter token reached. Increase max_counter_token_count if you need more.");
	return value;
}
} // namespace SBLib::Algorithms
namespace SBLib { using namespace Algorithms; }
