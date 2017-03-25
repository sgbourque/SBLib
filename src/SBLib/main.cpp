#include <Mathematics/binomial_coefficient.h>
#include <Mathematics/combinations.h>
#include <Mathematics/multivector.h>
#include <Traits/clifford_traits.h>
#include <Traits/bit_traits.h>

#define USING_STATIC_FOR_EACH_OUTPUT	1 // setting this to 1 will use very heavy static code (test)
#define USING_TEST_MASK					~0u
#define USING_STANDARD_BASIS        	0


//
// Wedge product
//
template<size_t subspace_mask, size_t loop>
struct wedge_product_helper
{
private:
	template<size_t subspace_mask2, size_t loop2>
	struct wedge_product_internal
	{
		template<typename scalar_t>
		struct assign
		{
			template<int sign> static constexpr void alternate_multiply(scalar_t, const scalar_t&, const scalar_t&) {}; // nothing to do
		};
		template<typename scalar_t>
		struct assign<scalar_t&>
		{
			template<int sign> static constexpr void alternate_multiply    (scalar_t& result, const scalar_t& u, const scalar_t& v); // should not be called
			template<>         static constexpr void alternate_multiply<+1>(scalar_t& result, const scalar_t& u, const scalar_t& v) { result += u * v; }
			template<>         static constexpr void alternate_multiply<-1>(scalar_t& result, const scalar_t& u, const scalar_t& v) { result -= u * v; }
		};

	public:
		template<typename scalar_t, size_t space_mask0, size_t space_mask2, size_t rank_size0, size_t rank_size2>
		wedge_product_internal(multivector_t<scalar_t, space_mask0, rank_size0>& result, const scalar_t& u, const multivector_t<scalar_t, space_mask2, rank_size2>& v)
		{
			using traits = alternating_traits<subspace_mask, subspace_mask2>;
			using ref_type = decltype( result.get<(subspace_mask ^ subspace_mask2)>() );
			assign<ref_type>::alternate_multiply<traits::sign>(result.get<(subspace_mask ^ subspace_mask2)>(), u, v.get<subspace_mask2>());
		}
	};

public:
	template<typename scalar_t, size_t space_mask0, size_t space_mask1, size_t space_mask2, size_t rank_size0, size_t rank_size1, size_t rank_size2>
	wedge_product_helper(multivector_t<scalar_t, space_mask0, rank_size0>& result, const multivector_t<scalar_t, space_mask1, rank_size1>& u, const multivector_t<scalar_t, space_mask2, rank_size2>& v)
	{
		for_each_combination< select_combinations<space_mask2, rank_size2> >::iterate<wedge_product_internal>(result, u.get<subspace_mask>(), v);
	}
};
//
// Generic version
//
template<typename scalar_t, size_t space_mask1, size_t space_mask2, size_t rank_size1, size_t rank_size2>
auto wedge_product(const multivector_t<scalar_t, space_mask1, rank_size1>& u, const multivector_t<scalar_t, space_mask2, rank_size2>& v)
{
	multivector_t<scalar_t, (space_mask1 | space_mask2), (rank_size1 + rank_size2)> result;
	for_each_combination< select_combinations<space_mask1, rank_size1> >::iterate<wedge_product_helper>(result, u, v);
	return std::move(result);
}

//
// HACK ALERT : temporary test code for optimization check.
// Once optimized, double hodge on dot product cause about 44% performance loss (13 instructions instead of 9), which is bad.
// This needs to be fixed.
//
// Also, hodge isn't even defined yet so using it here is ackward...
//
template<typename scalar_t, size_t space_mask>
auto wedge_product(const vector_t<scalar_t, space_mask>& u, const multivector_t<scalar_t, space_mask, vector_t<scalar_t, space_mask>::dimension_size - 1>& v)
{
	using multivec_t = multivector_t<scalar_t, space_mask, vector_t<scalar_t, space_mask>::dimension_size>;
	multivec_t result(multivec_t::UNINITIALIZED);
	result.components.container[0] = DirectX::XMVector3Dot(u.components.container[0], (*v).components.container[0]);
	for (size_t batch = 1; batch != u.components.container.size(); ++batch)
		result.components.container[0] = DirectX::XMVectorAdd(result.components.container[0], DirectX::XMVector3Dot(u.components.container[batch], (*v).components.container[batch]));
	return std::move(result);
}
//
// HACK ALERT : Only valid on 3 dimension (test only)
//
template<typename scalar_t, size_t space_mask>
auto wedge_product(const vector_t<scalar_t, space_mask>& u, const vector_t<scalar_t, space_mask>& v)
{
	using multivec_t = multivector_t<scalar_t, space_mask, 1>;
	multivec_t result(multivec_t::UNINITIALIZED);
	result.components.container[0] = DirectX::XMVector3Cross(u.components.container[0], v.components.container[0]);
	return std::move(*result);
}
template<typename scalar_t, size_t space_mask1, size_t space_mask2, size_t rank_size1, size_t rank_size2>
auto operator ^(const multivector_t<scalar_t, space_mask1, rank_size1>& u, const multivector_t<scalar_t, space_mask2, rank_size2>& v)
{
	return std::move(wedge_product(u, v));
}


//
// Hodge dual
//
template<size_t subspace_mask, size_t loop>
struct hodge_conjugate_helper
{
private:
	using scalar_t = float;
	template<typename scalar_t>
	struct assign
	{
		template<int sign> static constexpr void conjugate(scalar_t, const scalar_t&) {}; // result is 0 so nothing to do
	};
	template<typename scalar_t>
	struct assign<scalar_t&>
	{
		template<int sign> static constexpr void conjugate(scalar_t& result, const scalar_t& u); // should not be called
		template<>         static constexpr void conjugate<+1>(scalar_t& result, const scalar_t& u) { result = +u; }
		template<>         static constexpr void conjugate<-1>(scalar_t& result, const scalar_t& u) { result = -u; }
	};

public:
	template<typename scalar_t, size_t space_mask0, size_t space_mask1, size_t rank_size0, size_t rank_size1>
	hodge_conjugate_helper(multivector_t<scalar_t, space_mask0, rank_size0>& result, const multivector_t<scalar_t, space_mask1, rank_size1>& u)
	{
		using traits = hodge_conjugacy_traits<subspace_mask, space_mask0>;
		using ref_type = decltype(result.get<traits::bit_set>());
		static_assert((subspace_mask & space_mask0) == subspace_mask, "Cannot calculate Hodge dual over non-embedding space. Please project onto target space first.");
		static_assert((subspace_mask ^ traits::bit_set) == space_mask0, "Incorrect dual space.");
		assign<ref_type>::conjugate<traits::sign>(result.get<traits::bit_set>(), u.get<subspace_mask>());
	}
};
//
// Generic version
//
//template<typename scalar_t, size_t space_mask, size_t rank_size>
//auto hodge_conjugate(const multivector_t<scalar_t, space_mask, rank_size>& u)
//{
//	multivector_t<scalar_t, space_mask, vector_t<scalar_t, space_mask>::dimension_size - rank_size> result;
//	for_each_combination< select_combinations<space_mask, rank_size> >::iterate<hodge_conjugate_helper>(result, u);
//	return std::move(result);
//}

//
// HACK ALERT : temporary test code for optimization check. This won't work at all in generic cases (it works in dim 3)
// Once optimized, double hodge on dot product cause about 44% performance loss (13 instructions instead of 9), which is bad.
// This needs to be fixed.
//
template<typename scalar_t, size_t space_mask, size_t rank_size>
auto hodge_conjugate(const multivector_t<scalar_t, space_mask, rank_size>& u)
{
	using multivec_t = multivector_t<scalar_t, space_mask, bit_traits<space_mask>::population_count - rank_size>;
	static const DirectX::XMVECTOR sign{ 1.0f, -1.0f, 1.0f, 1.0f };
	multivec_t result(multivec_t::UNINITIALIZED);
	result.components.container[0] = DirectX::XMVectorMultiply(u.components.container[0], sign);
	return std::move(result);
}
template<typename scalar_t, size_t space_mask, size_t rank_size>
auto operator *(const multivector_t<scalar_t, space_mask, rank_size>& u)
{
	return std::move(hodge_conjugate(u));
}


template<typename scalar_t, size_t space_mask1, size_t space_mask2, size_t rank_size1, size_t rank_size2>
auto CrossProduct(const multivector_t<scalar_t, space_mask1, rank_size1>& u, const multivector_t<scalar_t, space_mask2, rank_size2>& v)
{
	return std::move( *(u ^ v) );
}
template<typename scalar_t, size_t space_mask1, size_t space_mask2, size_t rank_size1, size_t rank_size2>
auto InnerProduct(const multivector_t<scalar_t, space_mask1, rank_size1>& u, const multivector_t<scalar_t, space_mask2, rank_size2>& v)
{
	return std::move( *(u ^ *v) );
}

//////////////////////////////////////////////////////////////////////////////

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
template<typename type_t>
struct algebraic_t;

template<typename field_t, size_t mask, size_t rank>
struct latex_t<multivector_t<field_t, mask, rank>>
{
	static constexpr const char* prefix() { return "\\left("; }
	static constexpr const char* delimiter() { return ",\\,"; }
	static constexpr const char* postfix() { return "\\right)"; }
};
template<typename field_t, size_t mask, size_t rank>
struct raw_t<multivector_t<field_t, mask, rank>>
{
	static constexpr const char* prefix() { return "{"; }
	static constexpr const char* delimiter() { return ", "; }
	static constexpr const char* postfix() { return "}"; }
};
template<typename field_t, size_t mask, size_t rank>
struct algebraic_t<multivector_t<field_t, mask, rank>>
{
	static constexpr const char* prefix() { return "("; }
	static constexpr const char* delimiter() { return " + "; }
	static constexpr const char* postfix() { return ")"; }
};

#if USING_STATIC_FOR_EACH_OUTPUT
template<template<typename> class output_traits>
struct output_multivector_components
{
private:
	template<typename out_traits_t>
	struct outer_basis_traits
	{
		static constexpr const char* symbol() { return "e"; }
		static constexpr const char* accessor_prefix()  { return ""; }
		static constexpr const char* accessor_postfix() { return ""; }
		static constexpr const char* delimiter() { return "^"; }
	};
public:
	template<size_t bit_mask, size_t loop>
	struct do_action
	{
	private:
		template<class outer_basis_traits>
		struct output_basis
		{
			template<size_t bit_index, size_t loop>
			struct do_action
			{
				do_action(std::ostream& out)
				{
					enum { first_bit = bit_traits<bit_mask>::get_bit<0>(), };
					const std::string delimiter = (loop == first_bit) ? "" : outer_basis_traits::delimiter();
					out << delimiter << outer_basis_traits::symbol() << outer_basis_traits::accessor_prefix() << bit_index << outer_basis_traits::accessor_postfix();
				}
			};
		};

	public:
		template<typename scalar_t, size_t space_mask, size_t rank_size>
		do_action(std::ostream& out, const multivector_t<scalar_t, space_mask, rank_size>& V)
		{
			using type_t = multivector_t<scalar_t, space_mask, rank_size>;
			using output_traits_t = output_traits<type_t>;
			const std::string delimiter = (loop == 0) ? "" : output_traits_t::delimiter();
			const std::string prefix    = (bit_traits<bit_mask>::population_count <= 1) ? " " : output_traits_t::prefix();
			const std::string postfix   = (bit_traits<bit_mask>::population_count <= 1) ? "" : output_traits_t::postfix();
			out << delimiter << V.get<bit_mask>() << prefix;
			for_each_bit_index<bit_mask>::iterate<output_basis<outer_basis_traits<output_traits_t>>::do_action>(out);
			out << postfix;
		}

		template<typename scalar_t, size_t space_mask>
		do_action(std::ostream& out, const multivector_t<scalar_t, space_mask, 0>& V)
		{
			out << V.get<bit_mask>();
		}
	};
};
#endif // #if USING_STATIC_FOR_EACH_OUTPUT

template<template<typename> class output_traits, typename field_type, size_t space_mask, size_t rank_size>
std::string to_string(const multivector_t<field_type, space_mask, rank_size>& V)
{
	using type_t = multivector_t<field_type, space_mask, rank_size>;
	using output_traits_t = output_traits<type_t>;

	std::stringstream ss;
	ss << output_traits_t::prefix();

#if USING_STATIC_FOR_EACH_OUTPUT
	for_each_combination<select_combinations<space_mask, rank_size>>::iterate<output_multivector_components<output_traits>::do_action>(ss, V);
#else // #if USING_STATIC_FOR_EACH_OUTPUT
	std::string delimiter;
	for (size_t index = 0; index < V.dimension_size; ++index)
	{
		ss << delimiter << V.components[index];
		delimiter = output_traits_t::delimiter();
	}
#endif // #if USING_STATIC_FOR_EACH_OUTPUT

	ss << output_traits_t::postfix();
	return std::move( ss.str() );
}

#if USING_STATIC_FOR_EACH_OUTPUT
template<typename type_t> using out_t = algebraic_t<type_t>;
#else // #if USING_STATIC_FOR_EACH_OUTPUT
template<typename type_t> using out_t = raw_t<type_t>;
#endif // #if USING_STATIC_FOR_EACH_OUTPUT

template<typename field_type, size_t space_mask, size_t rank_size>
std::ostream& operator <<(std::ostream& out, const multivector_t<field_type, space_mask, rank_size>& V)
{
	return out << to_string<out_t>(V);
}

template<typename field_type, size_t space_mask, size_t rank_size>
std::istream& operator >>(std::istream& in, multivector_t<field_type, space_mask, rank_size>& V)
{
	for (size_t index = 0; index < V.dimension_size; ++index)
		in >> V.components[index];
	return in;
}

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

//////////////////////////////////////////////////////////////////////////////

class test_combination : public RegisteredFunctor
{
	enum
	{
		e0  = (1 << 0),  e1  = (1 << 1),
		e2  = (1 << 2),  e3  = (1 << 3),
		e4  = (1 << 4),  e5  = (1 << 5),
		e6  = (1 << 6),  e7  = (1 << 7),
		e8  = (1 << 8),  e9  = (1 << 9),
		e10 = (1 << 10), e11 = (1 << 11),
		e12 = (1 << 12), e13 = (1 << 13),
		e14 = (1 << 14), e15 = (1 << 15),
		e16 = (1 << 16), e17 = (1 << 17),

#if USING_STANDARD_BASIS
		E0 = 0,
		E1 = (E1 | e0),
		E2 = (E1 | e1),
		E3 = (E2 | e2),
		E4 = (E3 | e3),
		E5 = (E4 | e4),
		E6 = (E5 | e5),
		E7 = (E6 | e6),
#else // #if USING_STANDARD_BASIS
		E0 = 0,
		E1 = (E0 | e2),
		E2 = (E1 | e3),
		E3 = (E2 | e5),
		E4 = (E3 | e7),
		E5 = (E4 | e11),
		E6 = (E5 | e13),
		E7 = (E6 | e17),
#endif // #if USING_STANDARD_BASIS

		SPACE_PRINT_DIMENSION_MASK = (e0|e1|e2|e3|e4|e7),
	};

	template<size_t dimension> static constexpr size_t get_space();
	template<> static constexpr size_t get_space<0>() { return E0; }
	template<> static constexpr size_t get_space<1>() { return E1; }
	template<> static constexpr size_t get_space<2>() { return E2; }
	template<> static constexpr size_t get_space<3>() { return E3; }
	template<> static constexpr size_t get_space<4>() { return E4; }
	template<> static constexpr size_t get_space<5>() { return E5; }
	template<> static constexpr size_t get_space<6>() { return E6; }
	template<> static constexpr size_t get_space<7>() { return E7; }

	template<size_t space_mask>
	class output_combinations
	{
	private:
		template<size_t rank_size, size_t loop_index>
		class output_rank
		{
		private:
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
						out << delimiter << "e" << bit_traits<space_mask>::get_bit_index<bit_mask>();
					}
				};

			public:
				template<size_t value, size_t loop>
				struct do_action
				{
					do_action(std::ostream& out)
					{
						using traits = bit_traits<value>;
						enum : size_t
						{
							index            = combinations<space_mask>::select<traits::population_count>::get_components_index<value>(),

							equiv_space_mask = (1uLL << bit_traits<space_mask>::population_count) - 1,
							equiv_value      = combinations<equiv_space_mask>::select<traits::population_count>::get<index>(),
						};
						using equiv_traits = bit_traits<equiv_value>;
						out << "\t\t";
						std::stringstream ss;
						for_each_bit<traits::bit_mask>::iterate<do_action_internal>(ss);
						out << std::left;
						out << std::setw(24) << ss.str();
						ss = std::stringstream();
						out << std::setw(1) << " ~ ";
						for_each_bit<equiv_traits::bit_mask>::iterate<do_action_internal>(ss);
						out << std::setw(24) << ss.str();
						ss.clear();
						out << std::setw(1) << "\t(" << equiv_value << ")\t";
						out << std::endl;
					}
				};
				template<size_t loop>
				struct do_action<0, loop>
				{
					static_assert(loop == 0, "Invalid loop");
					do_action(std::ostream& out)
					{
						using traits = bit_traits<0>;
						enum : size_t
						{
							value = 0,
							index = combinations<space_mask>::select<traits::population_count>::get_components_index<value>(),

							equiv_space_mask = (1uLL << bit_traits<space_mask>::population_count) - 1,
							equiv_value = combinations<equiv_space_mask>::select<traits::population_count>::get<index>(),
						};
						static_assert(equiv_value == 0, "invalid scalar basis");
						out << "\t\t";
						out << std::left;
						out << std::setw(24) << "1";
						out << std::setw(1) << " ~ ";
						out << std::setw(24) << "1";
						out << std::setw(1) << "\t(" << equiv_value << ")\t";
						out << std::endl;
					}
				};
			};

		public:
			output_rank(std::ostream& out)
			{
				out << "\tRank = " << rank_size << std::endl;
				for_each_combination<select_combinations<space_mask, rank_size>>::iterate<output_combination<space_mask>::do_action>(out);
			};
		};

	public:
		static void do_action(std::ostream& out)
		{
			out << "Dim = " << combinations<space_mask>::dimension_size << ":" << std::endl;
			static_for_each<0, combinations<space_mask>::dimension_size + 1>::iterate<output_rank>(out);
		};
	};

	template<size_t bit_index, size_t loop>
	struct do_action
	{
		do_action(std::ostream& out)
		{
			enum : size_t
			{
				space_mask = get_space<bit_index>(),
			};
			output_combinations<space_mask>::do_action(out);
		}
	};

	test_combination() : RegisteredFunctor(fct) {}
	static void fct()
	{
		for_each_bit_index<SPACE_PRINT_DIMENSION_MASK>::iterate<do_action>(std::cout);
	}
	static test_combination instance;
};
#if (USING_TEST_MASK & 1) != 0
test_combination test_combination::instance;
#endif // #if (USING_TEST_MASK & 1) != 0

//////////////////////////////////////////////////////////////////////////////

class test_multivector : public RegisteredFunctor
{
	enum
	{
		e0  = (1 << 0),  e1  = (1 << 1),
		e2  = (1 << 2),  e3  = (1 << 3),
		e4  = (1 << 4),  e5  = (1 << 5),
		e6  = (1 << 6),  e7  = (1 << 7),
		e8  = (1 << 8),  e9  = (1 << 9),
		e10 = (1 << 10), e11 = (1 << 11),
		e12 = (1 << 12), e13 = (1 << 13),
		e14 = (1 << 14), e15 = (1 << 15),
	};
	using multivector_type1 = multivector_t<float, e0 | e1 | e2, 1>;
	using multivector_type2 = multivector_t<float, e0 | e1 | e2, 2>;
	using multivector_type3 = multivector_t<float, e0 | e1 | e2, 3>;

	test_multivector() : RegisteredFunctor(fct) {}
	static void fct()
	{
		multivector_type1 test1{ cos(0.4f), -sin(0.4f), 0.0f, 0.0f };
		multivector_type1 test2{ sin(0.4f),  cos(0.4f), 0.0f, 0.0f };
		multivector_type1 test3{    0.0f,      0.0f,    1.0f, 0.0f};

		const std::string input_filename = "../../tmp/test_multivector.in";
		const bool onlytest = (std::cin.gcount() == 0);
		if (onlytest)
		{
			std::cout << "Press 'd' to delete file, press anything else to continue..." << std::endl;
		}
		int data = std::cin.get();
		if (data == 'd')
			std::remove(input_filename.c_str());
		else if (!onlytest)
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
				std::cout << "Please enter 9 numbers : ";
			}

			std::istream& in = file.is_open() ? file : std::cin;

			in >> test1;
			in >> test2;
			in >> test3;

			if (!file.is_open())
			{
				file.open(input_filename, std::ios_base::out);
				if (file.is_open())
				{
					file << version_string << " " << version << std::endl;
					file << " " << test1.cget<e0>();
					file << " " << test1.cget<e1>();
					file << " " << test1.cget<e2>();
					file << " " << test2.cget<e0>();
					file << " " << test2.cget<e1>();
					file << " " << test2.cget<e2>();
					file << " " << test3.cget<e0>();
					file << " " << test3.cget<e1>();
					file << " " << test3.cget<e2>();
				}
			}
		}

		//DirectX::XMVECTOR test1v{ test1.components[0], test1.components[1], test1.components[2] };
		//DirectX::XMVECTOR test2v{ test2.components[0], test2.components[1], test2.components[2] };
		DirectX::XMVECTOR test1v{ test1.components.container[0] };
		DirectX::XMVECTOR test2v{ test2.components.container[0] };

		auto test4 = (test1 ^ test2);
		std::cout << test1 << " ^ " << test2 << " = " << test4 << std::endl;

		auto test5 = CrossProduct(test1, test2);
		std::cout << test1 << " x " << test2 << " = " << test5 << std::endl;

		auto test6 = InnerProduct(test1, test2);
		std::cout << test1 << " . " << test2 << " = " << test6 << std::endl;

		DirectX::XMVECTORF32 test6v;
		test6v.v = DirectX::XMVector3Dot(test1v, test2v);
		std::cout << test1 << " . " << test2 << " = " << test6v.f[0] << std::endl;

		auto test7 = (test1 ^ test2 ^ test3);
		std::cout << "Det{" << test1 << test2 << test3 << "} = " << *test7 << std::endl;
	}

	static test_multivector instance;
};
#if (USING_TEST_MASK & 2)  != 0
test_multivector test_multivector::instance;
#endif // #if (USING_TEST_MASK & 2) != 0

//////////////////////////////////////////////////////////////////////////////

class test_vector : public RegisteredFunctor
{
	enum
	{
		e0  = (1 << 0),  e1 = (1 << 1),
		e2  = (1 << 2),  e3 = (1 << 3),
		e4  = (1 << 4),  e5 = (1 << 5),
		e6  = (1 << 6),  e7 = (1 << 7),
		e8  = (1 << 8),  e9 = (1 << 9),
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
		vector_type2 test3{ -1.0f,-1.0f,0.0f,0.0f, 1.0f }; // only sets components for e0 and e2, all other being 0.
		vector_type2 test4;
		vector_type3 test5{ -1.0f,-1.0f }; // only sets components for e0 and e1, all other being 0.
		vector_type3 test6 = test1;
		vector_type1 test7 = test5.project<vector_type1>(); // only e0 component is set to a non-zero value as e1 is not is vector_type1
		vector_type1::scalar_type coeff1, coeff2;

		const std::string input_filename = "../../tmp/test_vector.in";
		const bool onlytest = (std::cin.gcount() == 0);
		if (onlytest)
		{
			std::cout << "Press 'd' to delete file, press anything else to continue..." << std::endl;
		}
		int data = std::cin.get();
		if (data == 'd')
			std::remove(input_filename.c_str());
		else if (!onlytest)
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

		std::cout << "test_result1: " << test_result1 << std::endl;
		auto test_result2 = coeff1 * test3 * coeff2 + test4;
		std::cout << "test_result2: " << test_result2 << std::endl;
		auto test_result3 = coeff1d * test5 * coeff2d + test6;
		std::cout << "test_result3: " << test_result3 << std::endl;
		//auto test_result4 = coeff1 * test1 * coeff2 + test4; // this will fail compilation (vector types are incompatible) ... eventually this should be fixed as it all fits into destination
		auto test_result4 = test_result1 + test_result1;
		std::cout << "test_result4: " << test_result4 << std::endl;
		test_result4 = test_result1;
		test_result4 += test_result4;
		test_result4 *= coeff1;
		test_result4 /= coeff2;
		std::cout << "test_result1 (copied on 4): " << test_result1 << std::endl;
		std::cout << "test_result4 (modified 1): " << test_result4 << std::endl;
		std::cout << test7 << std::endl;

		std::cout << "... run test '" << instance.id << "d' to delete input file..." << std::endl;
	}

	static test_vector instance;
};
#if (USING_TEST_MASK & 4)  != 0
test_vector test_vector::instance;
#endif // #if (USING_TEST_MASK & 4) != 0

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

	template<size_t first_loop_value>
	struct output_bit
	{
		template<size_t value, size_t loop>
		struct do_action
		{
			do_action(std::ostream& out)
			{
				static const char* const delimiter = ((loop == first_loop_value) ? "" : ",");
				out << delimiter << value;
			}
		};
	};

	test_clifford_algebra() : RegisteredFunctor(fct) {}
	static void fct()
	{
		std::cout << traits::bit_mask << " (" << traits::population_count << ")";
		std::cout << ": -> (";
		for_each_bit<traits::bit_mask>::iterate<output_bit<0>::do_action>(std::cout);
		std::cout << ") ~ (";
		for_each_bit_index<traits::bit_mask>::iterate<output_bit<traits::get_bit<0>()>::do_action>(std::cout);
		std::cout << ") ~ (";
		for_each_bit_compoment<traits::bit_mask>::iterate<output_bit<traits::get_bit<0>()>::do_action>(std::cout);
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
#if (USING_TEST_MASK & 8) != 0
test_clifford_algebra test_clifford_algebra::instance;
#endif // #if (USING_TEST_MASK & 8) != 0

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
#if (USING_TEST_MASK & 16) != 0
test_multivector_space test_multivector_space::instance;
#endif // #if (USING_TEST_MASK & 16) != 0

//////////////////////////////////////////////////////////////////////////////

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
