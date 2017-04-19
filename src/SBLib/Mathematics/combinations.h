#pragma once
#include <Algorithms/static_for_each.h>
#include <Mathematics/binomial_coefficient.h>
#include <Traits/bit_traits.h>

namespace SBLib::Mathematics
{
//
// combinations
//
// By construction, duals are component-to-compoment using dual rank (or dual indices for self-dual ranks) and
// is thus "Hodge-natural". Moreover, components are all-increasing in low and self-dual ranks while all-decreasing for high ranks
// (because of required conjugation duality).
//
// For instance, in dimension 3, vector order is (e0 e1 e2) and pseudo-vector order is (e1^e2 e0^e2 e0^e1).
//
// This also builds Spin(3) naturally in quaternion basis as, with 1+ijk the scalar + pseudo-vector Cartesian basis of Euclidian 3-space,
// we get the quaternion algebra (i = e1^e2, j = e0^e2, k = e0^e1) :
//		i� = j� = k� = ijk = -1� = -1
// and
//		ij = -ji = k,
//		jk = -kj = i,
//		ki = -ik = j.
//
template<size_t bit_mask>
struct combinations
{
	enum : size_t
	{
		space_mask     = bit_mask,
		dimension_size = bit_count(space_mask),
		count          = (1 << dimension_size),
	};
};

template<size_t space_mask, size_t rank_size>
struct select_combinations
{
	enum : size_t
	{
		space_mask     = space_mask,
		dimension_size = bit_count(space_mask),
		rank_size      = rank_size,
		count          = binomial_coefficient<dimension_size, rank_size>::value,
	};
};

template<size_t index, size_t space_mask, size_t rank_size>
constexpr size_t get( select_combinations<space_mask, rank_size> ) noexcept
{
	using type = select_combinations<space_mask, rank_size>;
	static_assert(rank_size <= type::dimension_size, "Invalid rank");
	static_assert(index < type::count, "Invalid combination");
	enum : size_t
	{
		last_bit                 = get_bit<type::dimension_size - 1>(space_mask),
		inherited_space_mask     = space_mask & ~last_bit,
		inherited_rank_size      = (rank_size < type::dimension_size) ? rank_size : 0,
		inherited_count          = select_combinations<inherited_space_mask, inherited_rank_size>::count,
		constructed_count        = select_combinations<inherited_space_mask, rank_size - 1>::count,
	};
	enum : bool
	{
		is_high_rank           = (2 * rank_size) > type::dimension_size,
		is_self_conjugate_rank = (2 * rank_size) == type::dimension_size,
		is_high_index          = index >= (type::count + 1)  / 2,

		use_conjugate            = is_high_rank || (is_self_conjugate_rank && is_high_index),
		is_inherited_index       = !use_conjugate && (index < inherited_count),
		is_inherited_conjugate   = is_inherited_index && (2 * rank_size > type::dimension_size - 1),
		is_constructed           = !use_conjugate && (index >= type::count - constructed_count),
	};
	static_assert(use_conjugate || (inherited_count + constructed_count == type::count), "Not all combinations are generated!");
	static_assert(use_conjugate || is_inherited_index || is_constructed, "Not all combinations are generated!");
	enum : size_t
	{
		conjugate_rank_size  = use_conjugate ? type::dimension_size - rank_size : 0,
		conjugate_index      = is_high_rank  ? index : use_conjugate ? type::count - index - 1 : 0,
		value_from_conjugate = space_mask & ~get<conjugate_index>( select_combinations<space_mask, conjugate_rank_size>() ),

		inherited_conjugate  = is_inherited_index ? index : 0,
		inherited_index      = is_inherited_index ? is_inherited_conjugate ? inherited_count - index - 1 : index : 0,
		inherited_value      = get<inherited_index>( select_combinations<inherited_space_mask, inherited_rank_size>() ),

		constructed_index    = is_constructed ? constructed_count - (type::count - index - 1) - 1 : 0,
		constructed_value    = last_bit | get<constructed_index>( select_combinations<inherited_space_mask, rank_size - 1>() ),

		value = use_conjugate ? value_from_conjugate :
			    is_inherited_index ? inherited_value :
			    is_constructed ? constructed_value : 0,
	};
	return value;
}
template<size_t index, size_t space_mask>
constexpr size_t get( select_combinations<space_mask, 0> ) noexcept
{
	using type = select_combinations<space_mask, 0>;
	static_assert(0 <= type::dimension_size, "Invalid rank");
	static_assert(index < type::count, "Invalid combination");
	return 0;
}

template<size_t subspace_mask, size_t space_mask, size_t rank_size>
constexpr size_t get_components_index( select_combinations<space_mask, rank_size> ) noexcept
{
	using type = select_combinations<space_mask, rank_size>;
	static_assert(rank_size <= type::dimension_size, "Invalid rank");
	static_assert((subspace_mask & space_mask) == subspace_mask, "Invalid combination");
	static_assert(bit_count(subspace_mask) == rank_size, "Invalid combination");
	enum : size_t
	{
		last_bit = get_bit<type::dimension_size - 1>(space_mask),
		inherited_space_mask = space_mask & ~last_bit,
		inherited_rank_size  = (rank_size < type::dimension_size) ? rank_size : 0,
		inherited_count      = select_combinations<inherited_space_mask, inherited_rank_size>::count,
		constructed_count    = select_combinations<inherited_space_mask, rank_size - 1>::count,
	};
	enum : bool
	{
		is_high_rank             = (2 * rank_size) > type::dimension_size,
		use_conjugate            = is_high_rank,
		is_inherited_index       = !use_conjugate && (subspace_mask & last_bit) == 0,
		is_inherited_conjugate   = (2 * rank_size) > type::dimension_size - 1,
		is_constructed           = !use_conjugate && (subspace_mask & last_bit) == last_bit,
	};
	static_assert(use_conjugate || (inherited_count + constructed_count == type::count), "Not all combinations are generated!");
	static_assert(use_conjugate || is_inherited_index || is_constructed, "Not all combinations are generated!");
	enum : size_t
	{
		conjugate_rank_size  = use_conjugate ? type::dimension_size - rank_size : 0,
		conjugate_mask       = use_conjugate ? space_mask & ~subspace_mask : 0,
		value_from_conjugate = get_components_index<conjugate_mask>( select_combinations<space_mask, conjugate_rank_size>() ),

		inherited_rank       = is_inherited_index ? inherited_rank_size : 0,
		inherited_mask       = is_inherited_index ? subspace_mask : 0,
		inherited_raw_value  = get_components_index<inherited_mask>( select_combinations<inherited_space_mask, inherited_rank>() ),
		inherited_value      = is_inherited_conjugate ? select_combinations<inherited_space_mask, inherited_rank>::count - inherited_raw_value - 1: inherited_raw_value,

		constructed_rank     = is_constructed ? (rank_size - 1) : 0,
		constructed_submask  = is_constructed ? (subspace_mask & ~last_bit) : 0,
		constructed_value    = inherited_count + get_components_index<constructed_submask>( select_combinations<inherited_space_mask, constructed_rank>() ),

		value = use_conjugate ? value_from_conjugate :
			    is_inherited_index ? inherited_value :
			    is_constructed ? constructed_value : ~0uLL,
	};
	static_assert(value < type::count, "Invalid index");
	return value;
}
template<size_t subspace_mask, size_t space_mask>
constexpr size_t get_components_index( select_combinations<space_mask, 0> ) noexcept
{
	using type = select_combinations<space_mask, 0>;
	static_assert(0 <= type::dimension_size, "Invalid rank");
	static_assert(bit_count(subspace_mask) == 0, "Invalid combination");
	return 0;
}

//enum : size_t
//{
//	value = 0,// select_combinations::template get<index>(),
//};
//template<size_t subspace_mask>
//constexpr static size_t get_components_index()
//{
//	static_assert(0 <= dimension_size, "Invalid rank");
//	static_assert(bit_count(subspace_mask) == rank_size, "Invalid combination");
//	return 0;
//}


//enum : size_t
//{
//	value = SBLib::select_combinations<space_mask, rank_size, index>::template get<index>(),
//};
//
//
//template<size_t space_mask>
//struct select_combinations<space_mask, 0>
//{
//	enum : size_t
//	{
//		space_mask     = space_mask,
//		dimension_size = bit_count(space_mask),
//		rank_size      = 0,
//		count          = binomial_coefficient<dimension_size, rank_size>::value,
//	};
//	//template<size_t index>
//	//constexpr static size_t get()
//	//{
//	//	static_assert(0 <= dimension_size, "Invalid rank");
//	//	static_assert(index < count, "Invalid combination");
//	//	return 0;
//	//}
//	//enum : size_t
//	//{
//	//	value = 0,// select_combinations::template get<index>(),
//	//};
//	//template<size_t subspace_mask>
//	//constexpr static size_t get_components_index()
//	//{
//	//	static_assert(0 <= dimension_size, "Invalid rank");
//	//	static_assert(bit_count(subspace_mask) == rank_size, "Invalid combination");
//	//	return 0;
//	//}
//};
//template<size_t space_mask>
//struct select_combinations<space_mask, 1>
//{
//	enum : size_t
//	{
//		space_mask     = space_mask,
//		dimension_size = bit_count(space_mask),
//		rank_size      = 1,
//		count          = binomial_coefficient<dimension_size, rank_size>::value,
//	};
//	//template<size_t index>
//	//constexpr static size_t get()
//	//{
//	//	static_assert(1 <= dimension_size, "Invalid rank");
//	//	static_assert(index < count, "Invalid combination");
//	//	enum : size_t
//	//	{
//	//		value = get_bit<index>(space_mask),
//	//	};
//	//	return value;
//	//}
//	//enum : size_t
//	//{
//	//	value = get_bit<index>(space_mask),//select_combinations::template get<index>(),
//	//};
//	//template<size_t subspace_mask>
//	//constexpr static size_t get_components_index()
//	//{
//	//	static_assert(0 <= dimension_size, "Invalid rank");
//	//	static_assert(bit_count(subspace_mask) == rank_size, "Invalid combination");
//	//	enum : size_t
//	//	{
//	//		value = get_bit_component<subspace_mask>(space_mask),
//	//	};
//	//	static_assert(value < count, "Invalid index");
//	//	static_assert(get_bit<value>(space_mask) == subspace_mask, "Invalid index");
//	//	return value;
//	//}
//};


template<class traits>
struct get_combination_helper
{
	template<size_t index>
	static constexpr size_t get()
	{
		return SBLib::get<index>( traits() );
	}
};
template<typename combinations_traits_t> struct for_each_combination : static_for_each<0, combinations_traits_t::count, get_combination_helper<combinations_traits_t>, increment_index_helper> {};
} // namespace SBLib::Mathematics
namespace SBLib { using namespace SBLib::Mathematics; }
