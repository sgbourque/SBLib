#include <Mathematics/binomial_coefficient.h>
#include <Mathematics/canonical_components.h>
#include <Traits/clifford_traits.h>
#include <Traits/bit_traits.h>

#define USING_STATIC_FOR_EACH_OUTPUT 0 // setting this to 1 will use very heavy static code (test)

//
// vector_t
//
template<typename scalar_t, size_t space_mask>
struct vector_t
{
private:
	using traits = bit_traits<space_mask>;

	template<size_t subspace_mask>
	struct get_traits_helper
	{
		using reference_type       = scalar_t&;
		using const_reference_type = const scalar_t&;
	};
	template<>
	struct get_traits_helper<0>
	{
		using reference_type       = scalar_t;
		using const_reference_type = scalar_t;
	};
	template<size_t subspace_mask>
	struct get_traits
	{
		static const size_t mask   = ( bit_traits<subspace_mask>::population_count == 1 ) ? (subspace_mask & space_mask) : 0;
		using reference_type       = typename get_traits_helper<mask>::reference_type;
		using const_reference_type = typename get_traits_helper<mask>::const_reference_type;
	};

	template<size_t subspace_mask>
	constexpr auto get_helper() -> typename get_traits<subspace_mask>::reference_type
	{
		return components[traits::get_bit_index<subspace_mask>()];
	}
	template<>
	constexpr auto get_helper<0>() -> typename get_traits<0>::reference_type
	{
		return scalar_t(0);
	}
	template<size_t subspace_mask>
	constexpr auto get_helper() const -> typename get_traits<subspace_mask>::const_reference_type
	{
		return components[traits::get_bit_index<subspace_mask>()];
	}
	template<>
	constexpr auto get_helper<0>() const -> typename get_traits<0>::const_reference_type
	{
		return scalar_t(0);
	}

	template<size_t value, size_t loop>
	struct component_assign_helper
	{
		template<typename scalar_dest, size_t space_mask_dest, typename scalar_src, size_t space_mask_src>
		component_assign_helper(vector_t<scalar_dest, space_mask_dest>& u, const vector_t<scalar_src, space_mask_src>& v)
		{
			static_assert( (value & space_mask) != 0, "Cannot assign value to orthogonal components of a vector." );
			u.get<value>() = static_cast<scalar_dest>( v.get<value>() );
		}
	};

public:
	enum
	{
		space_mask     = space_mask,
		dimension_size = traits::population_count,
	};
	using components_type = typename canonical_components_t<scalar_t, dimension_size>              ;
	using scalar_type     = typename canonical_components_t<scalar_t, dimension_size>::scalar_type;

	vector_t() : components() {};
	vector_t(const vector_t& v) : components(v.components) {};
	explicit vector_t(const components_type& v) : components(v) {};
	explicit vector_t(components_type&& v) : components(v) {};

	template<typename... scalars>
	explicit vector_t(scalars&&... coords) : components{std::forward<scalars>(coords)...} {}

	template<typename alt_scalar_t, size_t alt_space_mask>
	vector_t(const vector_t<alt_scalar_t, alt_space_mask>& v) : components()
	{
		using alt_traits = bit_traits<alt_space_mask>;
		for_each_bit<alt_traits>::iterate<component_assign_helper>(*this, v);
	};

	template<typename alt_scalar_t, size_t alt_space_mask>
	vector_t<scalar_t, alt_space_mask> project() const
	{
		vector_t<alt_scalar_t, alt_space_mask> v;
		using alt_traits = bit_traits<(alt_space_mask & space_mask)>;
		for_each_bit<alt_traits>::iterate<component_assign_helper>(v, *this);
		return std::move(v);
	};
	template<typename alt_vector_t>
	alt_vector_t project() const
	{
		return project<alt_vector_t::scalar_type, alt_vector_t::space_mask>();
	};


	template<size_t subspace_mask>
	constexpr auto get() -> typename get_traits<subspace_mask>::reference_type
	{
		return get_helper<get_traits<subspace_mask>::mask>();
	}
	template<size_t subspace_mask>
	constexpr auto get() const
	{
		return get_helper<get_traits<subspace_mask>::mask>();
	}
	template<size_t subspace_mask>
	constexpr auto cget() const
	{
		return get<subspace_mask>();
	}

	components_type components;
};


template<typename scalar_t, size_t space_mask>
const auto& operator *=(vector_t<scalar_t, space_mask>& u, const scalar_t& scale)
{
	return u.components *= scale;
}
template<typename scalar_t, size_t space_mask>
auto operator *(const vector_t<scalar_t, space_mask>& u, const scalar_t& scale)
{
	return vector_t<scalar_t, space_mask>(std::move(u.components * scale));
}
template<typename scalar_t, size_t space_mask>
auto operator *(const scalar_t& scale, const vector_t<scalar_t, space_mask>& v)
{
	return std::move(v * scale);
}
template<typename scalar_t, size_t space_mask>
const auto& operator /=(vector_t<scalar_t, space_mask>& u, const scalar_t& scale)
{
	return u.components /= scale;
}
template<typename scalar_t, size_t space_mask>
auto operator /(const vector_t<scalar_t, space_mask>& u, const scalar_t& scale)
{
	return vector_t<scalar_t, space_mask>(std::move(v.components / scale));
}
template<typename scalar_t, size_t space_mask>
const auto& operator +=(vector_t<scalar_t, space_mask>& u, vector_t<scalar_t, space_mask>& v)
{
	return u.components += v.components;
}
template<typename scalar_t, size_t space_mask>
auto operator +(const vector_t<scalar_t, space_mask>& u, const vector_t<scalar_t, space_mask>& v)
{
	return vector_t<scalar_t, space_mask>(std::move(u.components + v.components));
}


//template<typename scalar_t, size_t subspace_mask0, ...>
//struct multivector_t
//{
//
//};



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
			out << delimiter << vec.get<bit_mask>();
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
test_vector test_vector::instance;

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
			static const char* const delimiter = ((loop == traits::get_bit<0>::value) ? "" : ",");
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
test_clifford_algebra test_clifford_algebra::instance;

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
test_multivector_space test_multivector_space::instance;

int main()
{
	bool quit = false;
	do
	{
		size_t testCase = ~0u;
		std::cout << "Enter test case [0 - " << RegisteredFunctor::size() - 1 << "] or 'q' to quit : ";
		std::cin >> testCase;
		while (!std::cin.good())
		{
			std::cin.clear();
			int data = std::cin.get();
			quit = (data == 'q');
		}

		if (quit)
			std::cout << "bye";
		else if (!RegisteredFunctor::eval(testCase))
			std::cout << "... test not found, please try again!" << std::endl;

		std::cout << std::endl;
	}
	while (std::cin.good() && !quit);
}
