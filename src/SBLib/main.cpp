#include <Mathematics/binomial_coefficient.h>
#include <Mathematics/vector.h>
#include <Traits/clifford_traits.h>
#include <Traits/bit_traits.h>

#define USING_STATIC_FOR_EACH_OUTPUT 0 // setting this to 1 will use very heavy static code (test)
#define USING_TEST_MASK ~0u

#define USING_STANDARD_BASIS 0
#define PRINT_DIM_0 1
#define PRINT_DIM_1 1
#define PRINT_DIM_2 1
#define PRINT_DIM_3 1
#define PRINT_DIM_4 1
#define PRINT_DIM_5 1
#define PRINT_DIM_6 1
#define PRINT_DIM_7 1

//
// combinations_t
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
struct combinations_t
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
				inherited_count          = combinations_t<inherited_space_mask>::select<inherited_rank_size>::count,
				constructed_count        = combinations_t<inherited_space_mask>::select<rank_size - 1>::count,
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
				inherited_value      = combinations_t<inherited_space_mask>::select<inherited_rank_size>::get<inherited_index>(),

				constructed_index    = is_constructed ? constructed_count - (count - index - 1) - 1 : 0,
				constructed_value    = last_bit | combinations_t<inherited_space_mask>::select<rank - 1>::get<constructed_index>(),

				value = use_conjugate ? value_from_conjugate :
				        is_inherited_index ? inherited_value :
				        is_constructed ? constructed_value : 0,
			};
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
			return bit_traits<space_mask>::get_bit<index>();
		}
	};

private:
	//template<size_t combination_index>
	//struct select_combination : select< select_combination<combination_index - 1>::next_rank_size >
	//{
	//	enum : size_t
	//	{
	//		value = get< select_combination<combination_index - 1>::next_index >(),

	//		current_index  = combination_index,
	//		next_rank_size = (dimension_size > 0) ? 1 : rank_size,
	//		next_index     = (current_index + 1 >= count) ? 0 : current_index + 1,
	//	};
	//};

	//template<>
	//struct select_combination<0> : select<0>
	//{
	//	enum : size_t
	//	{
	//		value = get<0>(),

	//		current_index  = 0,
	//		next_rank_size = (dimension_size > 0) ? 1 : rank_size,
	//		next_index     = (current_index + 1 >= count) ? 0 : current_index + 1,
	//	};
	//};
	//template<>
	//struct select_combination<count - 1> //: select<dimension_size>::select_combination<0>
	//{
	//	enum : size_t
	//	{
	//		current_index = 0,
	//	};
	//};

//public:
//	template<size_t index>
//	constexpr static size_t get()
//	{
//		static_assert(index < count, "Invalid combination");
//		return select_combination<index>::value;
//	}
};

template<size_t space_mask, size_t rank_size, size_t index = 0>
struct select_combinations : combinations_t<space_mask>::select<rank_size>
{
	enum
	{
		value = get<index>(),
	};
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
template<typename combinations_traits_t> struct for_each_combination : static_for_each<0, binomial_coefficient<combinations_traits_t::dimension_size, combinations_traits_t::rank_size>::value, get_combination_helper<combinations_traits_t>, increment_index_helper<combinations_traits_t>> {};

//template<typename scalar_t, size_t subspace_mask0, ...>
//struct multivector_t
//{
//
//};


#if 1 // help collapsing
//
//
// Tests...
//
//
#include <sstream>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <functional>
#include <vector>

template<typename type_t>
struct latex_t;
template<typename type_t>
struct raw_t;

template<typename field_t, size_t dimension>
struct latex_t<vector_t<field_t, dimension>>
{
	static constexpr std::string prefix() { return "\\left("; }
	static constexpr std::string delimiter() { return ",\\,"; }
	static constexpr std::string postfix() { return "\\right)"; }
};
template<typename field_t, size_t dimension>
struct raw_t<vector_t<field_t, dimension>>
{
	static constexpr std::string prefix() { return "{"; }
	static constexpr std::string delimiter() { return ", "; }
	static constexpr std::string postfix() { return "}"; }
};

#if USING_STATIC_FOR_EACH_OUTPUT
template<template<typename> class output_traits>
struct output_vector_component
{
	template<size_t bit_mask, size_t loop>
	struct do_action
	{
		template<typename field_type, size_t space_mask>
		do_action(std::ostream& out, const vector_t<field_type, space_mask>& vec)
		{
			using type_t = vector_t<field_type, space_mask>;
			using output_traits_t = output_traits<type_t>;
			const std::string delimiter = (loop == 0) ? "" : output_traits_t::delimiter();
			out << delimiter << "e" << bit_traits<space_mask>::get_mask_index<bit_mask>() << " = " << vec.get<bit_mask>();
		}
	};
};
#endif // #if USING_STATIC_FOR_EACH_OUTPUT

template<template<typename> class output_traits, typename field_type, size_t space_mask>
std::string to_string(const vector_t<field_type, space_mask>& vec)
{
	using type_t = vector_t<field_type, space_mask>;
	using space_traits_t = bit_traits<space_mask>;
	using output_traits_t = output_traits<type_t>;

	std::stringstream ss;
	ss << output_traits_t::prefix();

#if USING_STATIC_FOR_EACH_OUTPUT
	for_each_bit<space_traits_t>::iterate<output_vector_component<output_traits>::do_action>(ss, vec);
#else // #if USING_STATIC_FOR_EACH_OUTPUT
	std::string delimiter;
	for (size_t index = 0; index < vec.dimension_size; ++index)
	{
		ss << delimiter << vec.components[index];
		delimiter = output_traits_t::delimiter();
	}
#endif // #if USING_STATIC_FOR_EACH_OUTPUT

	ss << output_traits_t::postfix();

	return std::move( ss.str() );
}

template<typename type_t> using out_t = raw_t<type_t>;

//////////////////////////

class RegisteredFunctor
{
public:
	static bool eval(size_t id)
	{
		const bool found = ( id < functionMap.size() );
		if (found)
			functionMap[id]();
		return found;
	}
	static size_t size()
	{
		return functionMap.size();
	}
protected:
	using fct_type = void();
	RegisteredFunctor(fct_type fct) : id(functionMap.size())
	{
		functionMap.push_back(fct);
	}
	const  size_t id;
private:
	static std::vector<std::function<fct_type>> functionMap;
};
std::vector<std::function<typename RegisteredFunctor::fct_type>> RegisteredFunctor::functionMap;
#endif // #if 1 // help collapsing

class test_combination : public RegisteredFunctor
{
	enum
	{
		e0 = (1 << 0), e1 = (1 << 1),
		e2 = (1 << 2), e3 = (1 << 3),
		e4 = (1 << 4), e5 = (1 << 5),
		e6 = (1 << 6), e7 = (1 << 7),
		e8 = (1 << 8), e9 = (1 << 9),
		e10 = (1 << 10), e11 = (1 << 11),
		e12 = (1 << 12), e13 = (1 << 13),
		e14 = (1 << 14), e15 = (1 << 15),
		e16 = (1 << 16), e17 = (1 << 17),

#if USING_STANDARD_BASIS
		E0 = 0,
		E1 = (e0),
		E2 = (e0 | e1),
		E3 = (e0 | e1 | e2),
		E4 = (e0 | e1 | e2 | e3),
		E5 = (e0 | e1 | e2 | e3 | e4),
		E6 = (e0 | e1 | e2 | e3 | e4 | e5),
		E7 = (e0 | e1 | e2 | e3 | e4 | e5 | e6),
#else // #if USING_STANDARD_BASIS
		E0 = 0,
		E1 = (e2),
		E2 = (e2 | e3),
		E3 = (e2 | e3 | e5),
		E4 = (e2 | e3 | e5 | e7),
		E5 = (e2 | e3 | e5 | e7 | e11),
		E6 = (e2 | e3 | e5 | e7 | e11 | e13),
		E7 = (e2 | e3 | e5 | e7 | e11 | e13 | e17),
#endif // #if USING_STANDARD_BASIS
	};

	template<size_t space_mask>
	struct output_combination
	{
	private:
		template<size_t bit_mask, size_t loop>
		struct do_action_internal
		{
			do_action_internal(std::ostream& out)
			{
				const std::string delimiter = (loop == 0) ? "" : "^";
				out << delimiter << "e" << bit_traits<space_mask>::get_mask_index<bit_mask>();
			}
		};

	public:
		template<size_t value, size_t loop>
		struct do_action
		{
			do_action(std::ostream& out)
			{
				using traits = bit_traits<value>;
				out << "\t\t(" << value << "):\t";
				for_each_bit<traits>::iterate<do_action_internal>(out);
				out << std::endl;
			}
		};
		template<size_t loop>
		struct do_action<0, loop>
		{
			do_action(std::ostream& out)
			{
				out << "\t\t(0):\t1" << std::endl;
			}
		};
	};

	test_combination() : RegisteredFunctor(fct) {}
	static void fct()
	{
		// ( 0 )
		// ( 0 )( 1 )
		// ( 0 )( 1 <|> 2 )( 3 )
		// ( 0 )( 1 2 | 4 )<>( 6 5 | 3 )( 7 )
		// ( 0 )( 1 2 4 | 8 )( 9 10 12 <|> 6 5 3 )( 14 13 11 | 7 )( 15 )
		// ...
		enum
		{
			//fix_compile5_2_0 = select_combinations<E5, 2, 0>::value,
			//fix_compile5_2_1 = select_combinations<E5, 2, 1>::value,
			//fix_compile5_2_2 = select_combinations<E5, 2, 2>::value,
			//fix_compile5_2_3 = select_combinations<E5, 2, 3>::value,
			//fix_compile5_2_4 = select_combinations<E5, 2, 4>::value,

			fix_compile5_2_5 = select_combinations<E5, 2, 5>::value,
			fix_compile5_2_6 = select_combinations<E5, 2, 6>::value,
			//fix_compile5_2_7 = select_combinations<E5, 2, 7>::value,
			fix_compile5_2_8 = select_combinations<E5, 2, 8>::value,
			fix_compile5_2_9 = select_combinations<E5, 2, 9>::value,

			//fix_compile6_2_0 = select_combinations<E6, 2, 0>::value,
			//fix_compile6_2_1 = select_combinations<E6, 2, 1>::value,
		};

#if PRINT_DIM_0
		std::cout << "Dim = 0:" << std::endl;
		std::cout << "\tRank = 0:" << std::endl;
		for_each_combination<select_combinations<E0, 0>>::iterate<output_combination<E0>::do_action>(std::cout);
#endif // #if PRINT_DIM_0
#if PRINT_DIM_1
		std::cout << "Dim = 1:" << std::endl;
		std::cout << "\tRank = 0:" << std::endl;
		for_each_combination<select_combinations<E1, 0>>::iterate<output_combination<E1>::do_action>(std::cout);
		std::cout << "\tRank = 1:" << std::endl;
		for_each_combination<select_combinations<E1, 1>>::iterate<output_combination<E1>::do_action>(std::cout);
#endif // #if PRINT_DIM_1
#if PRINT_DIM_2
		std::cout << "Dim = 2:" << std::endl;
		std::cout << "\tRank = 0:" << std::endl;
		for_each_combination<select_combinations<E2, 0>>::iterate<output_combination<E2>::do_action>(std::cout);
		std::cout << "\tRank = 1:" << std::endl;
		for_each_combination<select_combinations<E2, 1>>::iterate<output_combination<E2>::do_action>(std::cout);
		std::cout << "\tRank = 2:" << std::endl;
		for_each_combination<select_combinations<E2, 2>>::iterate<output_combination<E2>::do_action>(std::cout);
#endif // #if PRINT_DIM_2
#if PRINT_DIM_3
		std::cout << "Dim = 3:" << std::endl;
		std::cout << "\tRank = 0:" << std::endl;
		for_each_combination<select_combinations<E3, 0>>::iterate<output_combination<E3>::do_action>(std::cout);
		std::cout << "\tRank = 1:" << std::endl;
		for_each_combination<select_combinations<E3, 1>>::iterate<output_combination<E3>::do_action>(std::cout);
		std::cout << "\tRank = 2:" << std::endl;
		for_each_combination<select_combinations<E3, 2>>::iterate<output_combination<E3>::do_action>(std::cout);
		std::cout << "\tRank = 3:" << std::endl;
		for_each_combination<select_combinations<E3, 3>>::iterate<output_combination<E3>::do_action>(std::cout);
#endif // #if PRINT_DIM_3
#if PRINT_DIM_4
		std::cout << "Dim = 4:" << std::endl;
		std::cout << "\tRank = 0:" << std::endl;
		for_each_combination<select_combinations<E4, 0>>::iterate<output_combination<E4>::do_action>(std::cout);
		std::cout << "\tRank = 1:" << std::endl;
		for_each_combination<select_combinations<E4, 1>>::iterate<output_combination<E4>::do_action>(std::cout);
		std::cout << "\tRank = 2:" << std::endl;
		for_each_combination<select_combinations<E4, 2>>::iterate<output_combination<E4>::do_action>(std::cout);
		std::cout << "\tRank = 3:" << std::endl;
		for_each_combination<select_combinations<E4, 3>>::iterate<output_combination<E4>::do_action>(std::cout);
		std::cout << "\tRank = 4:" << std::endl;
		for_each_combination<select_combinations<E4, 4>>::iterate<output_combination<E4>::do_action>(std::cout);
#endif // #if PRINT_DIM_4
#if PRINT_DIM_5
		std::cout << "Dim = 5:" << std::endl;
		std::cout << "\tRank = 0:" << std::endl;
		for_each_combination<select_combinations<E5, 0>>::iterate<output_combination<E5>::do_action>(std::cout);
		std::cout << "\tRank = 1:" << std::endl;
		for_each_combination<select_combinations<E5, 1>>::iterate<output_combination<E5>::do_action>(std::cout);
		std::cout << "\tRank = 2:" << std::endl;
		for_each_combination<select_combinations<E5, 2>>::iterate<output_combination<E5>::do_action>(std::cout);
		std::cout << "\tRank = 3:" << std::endl;
		for_each_combination<select_combinations<E5, 3>>::iterate<output_combination<E5>::do_action>(std::cout);
		std::cout << "\tRank = 4:" << std::endl;
		for_each_combination<select_combinations<E5, 4>>::iterate<output_combination<E5>::do_action>(std::cout);
		std::cout << "\tRank = 5:" << std::endl;
		for_each_combination<select_combinations<E5, 5>>::iterate<output_combination<E5>::do_action>(std::cout);
#endif // #if PRINT_DIM_5
#if PRINT_DIM_6
		std::cout << "Dim = 6:" << std::endl;
		std::cout << "\tRank = 0:" << std::endl;
		for_each_combination<select_combinations<E6, 0>>::iterate<output_combination<E6>::do_action>(std::cout);
		std::cout << "\tRank = 1:" << std::endl;
		for_each_combination<select_combinations<E6, 1>>::iterate<output_combination<E6>::do_action>(std::cout);
		std::cout << "\tRank = 2:" << std::endl;
		for_each_combination<select_combinations<E6, 2>>::iterate<output_combination<E6>::do_action>(std::cout);
		std::cout << "\tRank = 3:" << std::endl;
		for_each_combination<select_combinations<E6, 3>>::iterate<output_combination<E6>::do_action>(std::cout);
		std::cout << "\tRank = 4:" << std::endl;
		for_each_combination<select_combinations<E6, 4>>::iterate<output_combination<E6>::do_action>(std::cout);
		std::cout << "\tRank = 5:" << std::endl;
		for_each_combination<select_combinations<E6, 5>>::iterate<output_combination<E6>::do_action>(std::cout);
		std::cout << "\tRank = 6:" << std::endl;
		for_each_combination<select_combinations<E6, 6>>::iterate<output_combination<E6>::do_action>(std::cout);
#endif // #if PRINT_DIM_6
#if PRINT_DIM_7
		std::cout << "Dim = 7:" << std::endl;
		std::cout << "\tRank = 0:" << std::endl;
		for_each_combination<select_combinations<E7, 0>>::iterate<output_combination<E7>::do_action>(std::cout);
		std::cout << "\tRank = 1:" << std::endl;
		for_each_combination<select_combinations<E7, 1>>::iterate<output_combination<E7>::do_action>(std::cout);
		std::cout << "\tRank = 2:" << std::endl;
		for_each_combination<select_combinations<E7, 2>>::iterate<output_combination<E7>::do_action>(std::cout);
		std::cout << "\tRank = 3:" << std::endl;
		for_each_combination<select_combinations<E7, 3>>::iterate<output_combination<E7>::do_action>(std::cout);
		std::cout << "\tRank = 4:" << std::endl;
		for_each_combination<select_combinations<E7, 4>>::iterate<output_combination<E7>::do_action>(std::cout);
		std::cout << "\tRank = 5:" << std::endl;
		for_each_combination<select_combinations<E7, 5>>::iterate<output_combination<E7>::do_action>(std::cout);
		std::cout << "\tRank = 6:" << std::endl;
		for_each_combination<select_combinations<E7, 6>>::iterate<output_combination<E7>::do_action>(std::cout);
		std::cout << "\tRank = 7:" << std::endl;
		for_each_combination<select_combinations<E7, 7>>::iterate<output_combination<E7>::do_action>(std::cout);
#endif // #if PRINT_DIM_7
	}
	static test_combination instance;
};
#if (USING_TEST_MASK & 1) != 0
test_combination test_combination::instance;
#endif // #if (USING_TEST_MASK & 1) != 0


class test_vector : public RegisteredFunctor
{
	enum
	{
		e0 = (1 << 0), e1 = (1 << 1),
		e2 = (1 << 2), e3 = (1 << 3),
		e4 = (1 << 4), e5 = (1 << 5),
		e6 = (1 << 6), e7 = (1 << 7),
		e8 = (1 << 8), e9 = (1 << 9),
		e10 = (1 << 10), e11 = (1 << 11),
		e12 = (1 << 12), e13 = (1 << 13),
		e14 = (1 << 14), e15 = (1 << 15),
	};
	using vector_type1 = vector_t<float, e0 | e2 | e7 | e15>;
	using vector_type2 = vector_t<float, e0 | e2 | e7 | e13 | e15>;
	using vector_type3 = vector_t<long double, e0 | e1 | e2 | e7 | e13 | e15>;
	static_assert(sizeof(vector_type1) == vector_type1::dimension_size * sizeof(vector_type1::scalar_type), "vector size is incorrect...");
	//static_assert(sizeof(vector_type2) == vector_type2::dimension_size * sizeof(vector_type2::scalar_type), "vector size is incorrect...");
	static_assert(sizeof(vector_type3) == vector_type3::dimension_size * sizeof(vector_type3::scalar_type), "vector size is incorrect...");

	test_vector() : RegisteredFunctor(fct) {}
	static void fct()
	{
		vector_type1 test1{ -1.0f,-1.0f,-1.0f,-1.0f }; // sets all 4 components
		vector_type1 test2;
		vector_type2 test3{ -1.0f,-1.0f }; // only sets components for e0 and e2, all other being 0.
		vector_type2 test4;
		vector_type3 test5{ -1.0f,-1.0f }; // only sets components for e0 and e1, all other being 0.
		vector_type3 test6 = test1;
		vector_type1 test7 = test5.project<vector_type1>(); // only e0 component is set to a non-zero value as e1 is not is vector_type1
		vector_type1::scalar_type coeff1, coeff2;

		const std::string input_filename = "../../tmp/test_input.in";
		int data = std::cin.get();
		if (data == 'd')
			std::remove(input_filename.c_str());
		else
			std::cin.unget();

		{
			auto file = std::fstream(input_filename, std::ios_base::in | std::ios_base::_Nocreate);

			static const std::string version_string = "version";
			static const size_t version = 0;
			if (file.is_open())
			{
				std::string file_version_string;
				size_t file_version;
				file >> file_version_string >> file_version;
				if (file_version_string != version_string || file_version != version)
					file.close();
			}
			else
			{
				std::cout << "Please enter 10 numbers : ";
			}

			std::istream& in = file.is_open() ? file : std::cin;

			in >> coeff1 >> coeff2;
			in >> test1.get<e0>();
			in >> test1.get<e2>();
			in >> test1.get<e7>();
			in >> test1.get<e15>();

			in >> test2.get<e0>();
			in >> test2.get<e2>();
			in >> test2.get<e7>();
			in >> test2.get<e15>();

			if (!file.is_open())
			{
				file.open(input_filename, std::ios_base::out);
				if (file.is_open())
				{
					file << version_string << " " << version << std::endl;
					file << coeff1 << " " << coeff2;
					file << " " << test1.cget<e0>();
					file << " " << test1.cget<e2>();
					file << " " << test1.cget<e7>();
					file << " " << test1.cget<e15>();
					file << " " << test2.cget<e0>();
					file << " " << test2.cget<e2>();
					file << " " << test2.cget<e7>();
					file << " " << test2.cget<e15>();
				}
			}
			test3.get<e0>() = test1.cget<e0>();
			//test3.get<e1>()  = test1.cget<e1>(); // this would be fine and would not have any effect! not doing it just to check explicitly data set in constructor
			test3.get<e2>() = test1.cget<e2>();
			test3.get<e7>() = test1.cget<e7>();
			//test3.get<e13>() = test1.get<e13>(); // this would be fine and would not have any effect! not doing it just to check for unitialized data...
			test3.get<e15>() = test1.cget<e15>();
			test4.get<e0>() = test2.cget<e0>();
			//test4.get<e1>() = test2.cget<e1>(); // this would be fine and would not have any effect! not doing it just to check for unitialized data...
			test4.get<e2>() = test2.cget<e2>();
			test4.get<e7>() = test2.cget<e7>();
			test4.get<e13>() = test2.cget<e13>(); // this is fine even if test1 does not have any e13 component!
			test4.get<e15>() = test2.cget<e15>();

			test5.get<e0>() = test1.cget<e0>();
			//test5.get<e1>()  = test1.cget<e1>(); // this would be fine and would not have any effect! not doing it just to check explicitly data set in constructor
			test5.get<e2>() = test1.cget<e2>();
			test5.get<e7>() = test1.cget<e7>();
			//test3.get<e13>() = test1.get<e13>(); // this would be fine and would not have any effect! not doing it just to check for unitialized data...
			test5.get<e15>() = test1.cget<e15>();
			test6.get<e0>() = test2.cget<e0>();
			//test4.get<e1>() = test2.cget<e1>(); // this would be fine and would not have any effect! not doing it just to check for unitialized data...
			test6.get<e2>() = test2.cget<e2>();
			test6.get<e7>() = test2.cget<e7>();
			test6.get<e13>() = test2.cget<e13>(); // this is fine even if test1 does not have any e1 component!
			test6.get<e15>() = test2.cget<e15>();
		}

		vector_type3::scalar_type coeff1d = coeff1, coeff2d = coeff2;
		auto test_result1 = coeff1 * test1 * coeff2 + test2;
		auto test_result2 = coeff1 * test3 * coeff2 + test4;
		auto test_result3 = coeff1d * test5 * coeff2d + test6;
		//auto test_result4 = coeff1 * test1 * coeff2 + test4; // this will fail compilation (vector types are incompatible) ... eventually this should be fixed as it all fits into destination
		auto test_result4 = test_result1 + test_result1;

		// checking both const and non-const accessors
		std::cout << "("
			<< test_result1.cget<e0>() << ", "
			<< test_result1.get<e2>() << ", "
			<< test_result1.get<e7>() << ", "
			<< test_result1.get<e15>()
			<< ")" << std::endl;

		std::cout << "0 = "
			<< test_result1.cget<0>() << " = "
			<< test_result1.get<e1 | e2>() << " = "
			<< test_result1.get<e3>()
			<< std::endl;

		std::cout << "test_result1: " << to_string<out_t>(test_result1) << std::endl;
		std::cout << "test_result2: " << to_string<out_t>(test_result2) << std::endl;
		std::cout << "test_result3: " << to_string<out_t>(test_result3) << std::endl;
		std::cout << "test_result4: " << to_string<out_t>(test_result4) << std::endl;
		test_result4 = test_result1;
		test_result4 += test_result4;
		test_result4 *= coeff1;
		test_result4 /= coeff2;
		std::cout << "test_result1 (copied on 4): " << to_string<out_t>(test_result1) << std::endl;
		std::cout << "test_result4 (modified 1): " << to_string<out_t>(test_result4) << std::endl;
		std::cout << to_string<out_t>(test7) << std::endl;

		std::cout << "... run test '" << instance.id << "d' to delete input file..." << std::endl;
	}

	static test_vector instance;
};
#if (USING_TEST_MASK & 2)  != 0
test_vector test_vector::instance;
#endif // #if (USING_TEST_MASK & 2) != 0


//////////////////////////////////////////////////////////////////////////////

class test_clifford_algebra : public RegisteredFunctor
{
	enum
	{
		e0 = (1 << 0),
		e1 = (1 << 1),
		e2 = (1 << 2),
		e3 = (1 << 3),
		e4 = (1 << 4),
		e5 = (1 << 5),
	};
	typedef bit_traits<0xAB> traits;

	template<size_t value, size_t loop>
	struct output_bit_mask
	{
		output_bit_mask(std::ostream& out)
		{
			static const char* const delimiter = ((loop == 0) ? "" : ",");
			out << delimiter << value;
		}
	};
	template<size_t value, size_t loop>
	struct output_bit_index
	{
		output_bit_index(std::ostream& out)
		{
			static const char* const delimiter = ((loop == traits::get_bit<0>()) ? "" : ",");
			out << delimiter << value;
		}
	};


	test_clifford_algebra() : RegisteredFunctor(fct) {}
	static void fct()
	{
		std::cout << traits::value << " (" << traits::population_count << ")";
		std::cout << " -> (";
		for_each_bit<traits>::iterate<output_bit_mask>(std::cout);
		std::cout << ")";
		std::cout << " ~ (";
		for_each_bit_index<traits>::iterate<output_bit_index>(std::cout);
		std::cout << ")" << std::endl;

		std::cout << "((e0 ^ e2) ^ e1) \n\t= "
			<< alternating_traits<(e0 ^ e2), e1>::sign
			<< " * (e0 ^ e1 ^ e2) \n\t= "
			<< reversion_conjugacy_traits<(e0 ^ e2)>::sign * alternating_traits<(e2 ^ e0), e1, false>::sign
			<< " * (e2 ^ e1 ^ e0)"
			<< std::endl;

		std::cout << "((e5 ^ e3 ^ e2) ^ (e4 ^ e0)) \n\t= "
			<< reversion_conjugacy_traits<(e5 ^ e3 ^ e2)>::sign * reversion_conjugacy_traits<(e4 ^ e0)>::sign * alternating_traits<(e2 ^ e3 ^ e5), (e0 ^ e4)>::sign
			* reversion_conjugacy_traits<(e5 ^ e4 ^ e3 ^ e2 ^ e0)>::sign
			<< " * (e5 ^ e4 ^ e3 ^ e2 ^ e0) \n\t= "
			<< alternating_traits<(e5 ^ e3 ^ e2), (e4 ^ e0), false>::sign * reversion_conjugacy_traits<(e5 ^ e4 ^ e3 ^ e2 ^ e0)>::sign
			<< " * (e0 ^ e2 ^ e3 ^ e4 ^ e5)"
			<< std::endl;

		std::cout << "((e5 ^ e3 ^ e2) ^ (e5 ^ e1)) \n\t= "
			<< reversion_conjugacy_traits<(e5 ^ e3 ^ e2)>::sign * reversion_conjugacy_traits<(e5 ^ e1)>::sign * alternating_traits<(e2 ^ e3 ^ e5), (e1 ^ e5)>::sign
			* reversion_conjugacy_traits<(e5 ^ e5 ^ e3 ^ e2 ^ e1)>::sign
			<< " * (e5 ^ e5 ^ e3 ^ e2 ^ e1) \n\t= "
			<< alternating_traits<(e5 ^ e3 ^ e2), (e5 ^ e1), false>::sign * reversion_conjugacy_traits<(e5 ^ e5 ^ e3 ^ e2 ^ e1)>::sign
			<< " * (e1 ^ e2 ^ e3 ^ e5 ^ e5)"
			<< std::endl;
	}

	static test_clifford_algebra instance;
};
#if (USING_TEST_MASK & 4) != 0
test_clifford_algebra test_clifford_algebra::instance;
#endif // #if (USING_TEST_MASK & 4) != 0


//////////////////////////////////////////////////////////////////////////////

class test_multivector_space : public RegisteredFunctor
{
	//
	// module_t
	//
	template<typename field_type, size_t dimension_size>
	struct module_t
	{
		typedef field_type field_t;
		enum { dimension = dimension_size };

		module_t()
			: components()
		{}
		module_t(std::array<field_t, dimension>&& coords)
			: components(coords)
		{}

		std::array<field_t, dimension> components;
	};

	//
	// uniform_multimodule_t
	//
	template<typename module_type, size_t rank_size>
	struct uniform_multimodule_t
		: module_t<typename module_type::field_t,
				   binomial_coefficient<module_type::dimension, rank_size>::value>
	{
		enum { rank = rank_size };

		uniform_multimodule_t()
			: module_t()
		{}
		uniform_multimodule_t(std::array<field_t, dimension>&& coords)
			: module_t(std::move(coords))
		{}
	};
	template<typename module_type>
	using scalar_type = uniform_multimodule_t<module_type, 0>;
	template<typename module_type>
	using vector_type = uniform_multimodule_t<module_type, 1>;
	template<typename module_type>
	using pseudovector_type = uniform_multimodule_t<module_type, module_type::dimension - 1>;
	template<typename module_type>
	using pseudoscalar_type = uniform_multimodule_t<module_type, module_type::dimension>;


	//
	// outermodule_t
	//
	/*
	template<typename module_type>
	struct outermodule_t
	//	: module_t<typename module_type::field_t, (1 << module_type::dimension)>
	{
		outermodule_t()
			: module_t()
		{}
		outermodule_t(std::array<field_t, dimension>&& coords)
			: module_t(std::move(coords))
		{}
	};
	*/

	template<template<typename> class traits, typename field_type, size_t dimension_size>
	static std::string to_string(const module_t<field_type, dimension_size>& vec)
	{
		typedef vector_t<field_type, dimension_size> type_t;
		typedef traits<type_t> traits_t;

		std::string delimiter;
		std::stringstream ss;
		ss << traits_t::prefix();
		for (const auto& value : vec.components)
			(ss << delimiter << value), (delimiter = traits_t::delimiter());
		ss << traits_t::postfix();

		return ss.str();
	}
	/*
	template<template<typename> class traits, typename module_type, size_t rank_size>
	static std::string to_string(const uniform_multimodule_t<module_type, rank_size>& multivec)
	{
		typedef uniform_multimodule_t<module_type, rank_size> uniform_multimodule_type;

		enum { dimension = uniform_multimodule_type::dimension };
		typedef typename uniform_multimodule_type::field_t    field_type;

		typedef module_t<field_type, dimension>       module_type;
		return to_string<traits>(static_cast<const module_type&>(multivec));
	}
	*/

	test_multivector_space() : RegisteredFunctor(fct) {}
	static void fct()
	{
		module_t<float, 1> bin0({
			binomial_coefficient<0,0>::value,
		});
		module_t<float, 2> bin1({
			binomial_coefficient<1, 0>::value,
			binomial_coefficient<1, 1>::value,
		});
		module_t<float, 3> bin2({
			binomial_coefficient<2, 0>::value,
			binomial_coefficient<2, 1>::value,
			binomial_coefficient<2, 2>::value,
		});
		module_t<float, 4> bin3({
			binomial_coefficient<3, 0>::value,
			binomial_coefficient<3, 1>::value,
			binomial_coefficient<3, 2>::value,
			binomial_coefficient<3, 3>::value,
		});
		module_t<float, 5> bin4({
			binomial_coefficient<4, 0>::value,
			binomial_coefficient<4, 1>::value,
			binomial_coefficient<4, 2>::value,
			binomial_coefficient<4, 3>::value,
			binomial_coefficient<4, 4>::value,
		});
		module_t<float, 6> bin5({
			binomial_coefficient<5, 0>::value,
			binomial_coefficient<5, 1>::value,
			binomial_coefficient<5, 2>::value,
			binomial_coefficient<5, 3>::value,
			binomial_coefficient<5, 4>::value,
			binomial_coefficient<5, 5>::value,
		});
		module_t<float, 7> bin6({
			binomial_coefficient<6, 0>::value,
			binomial_coefficient<6, 1>::value,
			binomial_coefficient<6, 2>::value,
			binomial_coefficient<6, 3>::value,
			binomial_coefficient<6, 4>::value,
			binomial_coefficient<6, 5>::value,
			binomial_coefficient<6, 6>::value,
		});
		module_t<float, 8> bin7({
			binomial_coefficient<7, 0>::value,
			binomial_coefficient<7, 1>::value,
			binomial_coefficient<7, 2>::value,
			binomial_coefficient<7, 3>::value,
			binomial_coefficient<7, 4>::value,
			binomial_coefficient<7, 5>::value,
			binomial_coefficient<7, 6>::value,
			binomial_coefficient<7, 7>::value,
		});
		module_t<float, 9> bin8({
			binomial_coefficient<8, 0>::value,
			binomial_coefficient<8, 1>::value,
			binomial_coefficient<8, 2>::value,
			binomial_coefficient<8, 3>::value,
			binomial_coefficient<8, 4>::value,
			binomial_coefficient<8, 5>::value,
			binomial_coefficient<8, 6>::value,
			binomial_coefficient<8, 7>::value,
			binomial_coefficient<8, 8>::value,
		});
		module_t<float, 10> bin9({
			binomial_coefficient<9, 0>::value,
			binomial_coefficient<9, 1>::value,
			binomial_coefficient<9, 2>::value,
			binomial_coefficient<9, 3>::value,
			binomial_coefficient<9, 4>::value,
			binomial_coefficient<9, 5>::value,
			binomial_coefficient<9, 6>::value,
			binomial_coefficient<9, 7>::value,
			binomial_coefficient<9, 8>::value,
			binomial_coefficient<9, 9>::value,
		});
		std::cout
			<< to_string<out_t>(bin0) << std::endl
			<< to_string<out_t>(bin1) << std::endl
			<< to_string<out_t>(bin2) << std::endl
			<< to_string<out_t>(bin3) << std::endl
			<< to_string<out_t>(bin4) << std::endl
			<< to_string<out_t>(bin5) << std::endl
			<< to_string<out_t>(bin6) << std::endl
			<< to_string<out_t>(bin7) << std::endl
			<< to_string<out_t>(bin8) << std::endl
			<< to_string<out_t>(bin9) << std::endl;

		typedef uniform_multimodule_t<decltype(bin5), 4> uniform_multimodule_type;
		uniform_multimodule_type multivec6_4({
			uniform_multimodule_type::dimension, decltype(bin5)::dimension, 4
		});
		std::cout << decltype(multivec6_4)::dimension << std::endl;
		std::cout << to_string<out_t>(multivec6_4) << std::endl;
	}
	static test_multivector_space instance;
};
#if (USING_TEST_MASK & 8) != 0
test_multivector_space test_multivector_space::instance;
#endif // #if (USING_TEST_MASK & 8) != 0

int main()
{
	bool quit = false;
	size_t testCase = ~0uLL;
	do
	{
		if (RegisteredFunctor::size() > 1)
		{
			testCase = ~0uLL;
			std::cout << "Enter test case [0 - " << RegisteredFunctor::size() - 1 << "] or 'q' to quit : ";
			std::cin >> testCase;
			while (!std::cin.good())
			{
				std::cin.clear();
				int data = std::cin.get();
				quit = (data == 'q');
			}
		}
		else
		{
			testCase = ~testCase;
			quit = (testCase != 0);
		}

		if (quit)
			std::cout << "bye";
		else if (!RegisteredFunctor::eval(testCase))
			std::cout << "... test not found, please try again!" << std::endl;

		std::cout << std::endl;
	}
	while (std::cin.good() && !quit);

	if (RegisteredFunctor::size() < 2)
		std::cin.get();
}
