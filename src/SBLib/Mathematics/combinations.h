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
// is thus "Hodge-natural". Moreover, compoments are all-increasing in low and self-dual ranks while all-decreasing for high ranks
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
		dimension_size = bit_traits<space_mask>::population_count,
		count          = (1 << dimension_size),
	};
	template<size_t rank>
	struct select
	{
		enum : size_t
		{
			space_mask     = bit_mask,
			dimension_size = bit_traits<space_mask>::population_count,
			rank_size      = rank,
			count          = binomial_coefficient<dimension_size, rank_size>::value,
		};
		template<size_t index>
		constexpr static size_t get()
		{
			static_assert(rank <= dimension_size, "Invalid rank");
			static_assert(index < count, "Invalid combination");
			enum : size_t
			{
				last_bit                 = bit_traits<space_mask>::get_bit<dimension_size - 1>(),
				inherited_space_mask     = space_mask & ~last_bit,
				inherited_rank_size      = (rank_size < dimension_size) ? rank_size : 0,
				inherited_count          = combinations<inherited_space_mask>::select<inherited_rank_size>::count,
				constructed_count        = combinations<inherited_space_mask>::select<rank_size - 1>::count,
			};
			enum : bool
			{
				is_high_rank           = (2 * rank_size) > dimension_size,
				is_self_conjugate_rank = (2 * rank_size) == dimension_size,
				is_high_index          = index >= (count + 1)  / 2,

				use_conjugate            = is_high_rank || (is_self_conjugate_rank && is_high_index),
				is_inherited_index       = !use_conjugate && (index < inherited_count),
				is_inherited_conjugate   = is_inherited_index && (2 * rank_size > dimension_size - 1),
				is_constructed           = !use_conjugate && (index >= count - constructed_count),
			};
			static_assert(use_conjugate || (inherited_count + constructed_count == count), "Not all combinations are generated!");
			static_assert(use_conjugate || is_inherited_index || is_constructed, "Not all combinations are generated!");
			enum : size_t
			{
				conjugate_rank_size  = use_conjugate ? dimension_size - rank_size : 0,
				conjugate_index      = is_high_rank  ? index : use_conjugate ? count - index - 1 : 0,
				value_from_conjugate = space_mask & ~select<conjugate_rank_size>::get<conjugate_index>(),

				inherited_conjugate  = is_inherited_index ? index : 0,
				inherited_index      = is_inherited_index ? is_inherited_conjugate ? inherited_count - index - 1 : index : 0,
				inherited_value      = combinations<inherited_space_mask>::select<inherited_rank_size>::get<inherited_index>(),

				constructed_index    = is_constructed ? constructed_count - (count - index - 1) - 1 : 0,
				constructed_value    = last_bit | combinations<inherited_space_mask>::select<rank - 1>::get<constructed_index>(),

				value = use_conjugate ? value_from_conjugate :
				        is_inherited_index ? inherited_value :
				        is_constructed ? constructed_value : 0,
			};
			return value;
		}

		template<size_t subspace_mask>
		constexpr static size_t get_components_index()
		{
			static_assert(rank <= dimension_size, "Invalid rank");
			static_assert((subspace_mask & space_mask) == subspace_mask, "Invalid combination");
			static_assert(bit_traits<subspace_mask>::population_count == rank_size, "Invalid combination");
			enum : size_t
			{
				last_bit = bit_traits<space_mask>::get_bit<dimension_size - 1>(),
				inherited_space_mask = space_mask & ~last_bit,
				inherited_rank_size  = (rank_size < dimension_size) ? rank_size : 0,
				inherited_count      = combinations<inherited_space_mask>::select<inherited_rank_size>::count,
				constructed_count    = combinations<inherited_space_mask>::select<rank_size - 1>::count,
			};
			enum : bool
			{
				is_high_rank             = (2 * rank_size) > dimension_size,
				use_conjugate            = is_high_rank,
				is_inherited_index       = !use_conjugate && (subspace_mask & last_bit) == 0,
				is_inherited_conjugate   = (2 * rank_size) > dimension_size - 1,
				is_constructed           = !use_conjugate && (subspace_mask & last_bit) == last_bit,
			};
			static_assert(use_conjugate || (inherited_count + constructed_count == count), "Not all combinations are generated!");
			static_assert(use_conjugate || is_inherited_index || is_constructed, "Not all combinations are generated!");
			enum : size_t
			{
				conjugate_rank_size  = use_conjugate ? dimension_size - rank_size : 0,
				conjugate_mask       = use_conjugate ? space_mask & ~subspace_mask : 0,
				value_from_conjugate = select<conjugate_rank_size>::get_components_index<conjugate_mask>(),

				inherited_rank       = is_inherited_index ? inherited_rank_size : 0,
				inherited_mask       = is_inherited_index ? subspace_mask : 0,
				inherited_raw_value  = combinations<inherited_space_mask>::select<inherited_rank>::get_components_index<inherited_mask>(),
				inherited_value      = is_inherited_conjugate ? combinations<inherited_space_mask>::select<inherited_rank>::count - inherited_raw_value - 1: inherited_raw_value,

				constructed_rank     = is_constructed ? (rank - 1) : 0,
				constructed_submask  = is_constructed ? (subspace_mask & ~last_bit) : 0,
				constructed_value    = inherited_count + combinations<inherited_space_mask>::select<constructed_rank>::get_components_index<constructed_submask>(),

				value = use_conjugate ? value_from_conjugate :
				        is_inherited_index ? inherited_value :
				        is_constructed ? constructed_value : ~0uLL,
			};
			static_assert(value < count, "Invalid index");
			return value;
		}
	};
	template<>
	struct select<0>
	{
		enum : size_t
		{
			space_mask     = bit_mask,
			dimension_size = bit_traits<space_mask>::population_count,
			rank_size      = 0,
			count          = binomial_coefficient<dimension_size, rank_size>::value,
		};
		template<size_t index>
		constexpr static size_t get()
		{
			static_assert(0 <= dimension_size, "Invalid rank");
			static_assert(index < count, "Invalid combination");
			return 0;
		}
		template<size_t subspace_mask>
		constexpr static size_t get_components_index()
		{
			static_assert(0 <= dimension_size, "Invalid rank");
			static_assert(bit_traits<subspace_mask>::population_count == rank_size, "Invalid combination");
			return 0;
		}
	};
	template<>
	struct select<1>
	{
		enum : size_t
		{
			space_mask     = bit_mask,
			dimension_size = bit_traits<space_mask>::population_count,
			rank_size      = 1,
			count          = binomial_coefficient<dimension_size, rank_size>::value,
		};
		template<size_t index>
		constexpr static size_t get()
		{
			static_assert(1 <= dimension_size, "Invalid rank");
			static_assert(index < count, "Invalid combination");
			enum : size_t
			{
				value = bit_traits<space_mask>::get_bit<index>(),
			};
			return value;
		}
		template<size_t subspace_mask>
		constexpr static size_t get_components_index()
		{
			static_assert(0 <= dimension_size, "Invalid rank");
			static_assert(bit_traits<subspace_mask>::population_count == rank_size, "Invalid combination");
			enum : size_t
			{
				value = bit_traits<space_mask>::get_bit_component<subspace_mask>(),
			};
			static_assert(value < count, "Invalid index");
			static_assert(bit_traits<space_mask>::get_bit<value>() == subspace_mask, "Invalid index");
			return value;
		}
	};
};

template<size_t space_mask, size_t rank_size, size_t index = 0>
struct select_combinations : combinations<space_mask>::select<rank_size>
{
	enum
	{
		value = get<index>(),
	};
	static_assert(get_components_index<value>() == index, "Incorrect calculated component index");
};

template<class traits>
struct get_combination_helper
{
private:
	template<size_t index> using get_combination_t = select_combinations<traits::space_mask, traits::rank_size, index>;
	template<size_t index>
	struct get_helper
	{
		enum : size_t
		{
			value = get_combination_t<index>::value,
		};
	};

public:
	template<size_t index>
	static constexpr size_t get()
	{
		return get_helper<index>::value;
	}
};
template<typename combinations_traits_t> struct for_each_combination : static_for_each<0, combinations_traits_t::count, get_combination_helper<combinations_traits_t>, increment_index_helper> {};
} // namespace SBLib::Mathematics
namespace SBLib { using namespace Mathematics; }
