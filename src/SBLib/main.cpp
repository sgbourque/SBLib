#include <Algorithms/static_for_each.h>
#include <Mathematics/binomial_coefficient.h>
#include <Traits/bit_traits.h>
#include <Traits/clifford_traits.h>

#define TEST_PATH 0

//
// vector_t
//
#include <array>
#define DIRECTX_VECTOR

template<template<typename, size_t> class components_type, typename scalar_t, size_t dimension>
struct components_helper
{
public:
	enum
	{
		dimension_size = dimension,
	};
	using scalar_type       = scalar_t;
	using container_type    = std::array<scalar_type, dimension_size>;
	using intermediate_type = scalar_type[dimension_size];

	scalar_type& operator[](const size_t index) { return static_cast<this_type*>(this)->container[index]; }
	const scalar_type& operator[](const size_t index) const { return static_cast<this_type*>(this)->container[index]; }

private:
	using this_type = components_type<scalar_t, dimension>;
};

template<typename scalar_t, size_t dimension>
struct components_t : components_helper<components_t, scalar_t, dimension>
{
	components_t() : container{} {};
	components_t(components_t<scalar_type, dimension_size>&& v): container(std::move(v.container)) {};
	components_t(const components_t<scalar_type, dimension_size>& v) : container(v.container) {};

	explicit components_t(const container_type& v) : container(v) {};
	explicit components_t(const intermediate_type& v) { container.v = v; };
	template<typename... scalars> explicit components_t(scalar_type&& first, scalars&&... coords) : container{ first, std::forward<scalars>(coords)... } {}

	components_t(container_type&& v) : container(v) {}
	components_t(intermediate_type&& v) { container.v = std::move(v); }

	operator container_type&() { return container; }
	operator const container_type&() const { return container; }

	container_type container;
};

template<typename scalar_t, size_t dimension>
auto begin(components_t<scalar_t, dimension>& u)
{
	return u.container.begin();
}
template<typename scalar_t, size_t dimension>
const auto begin(const components_t<scalar_t, dimension>& u)
{
	return u.container.begin();
}
template<typename scalar_t, size_t dimension>
const auto end(const components_t<scalar_t, dimension>& u)
{
	return u.container.end();
}
template<typename scalar_t, size_t dimension>
components_t<scalar_t, dimension> operator *(const scalar_t& scale, const components_t<scalar_t, dimension>& v)
{
	components_t<scalar_t, dimension> value;
	std::transform(begin(v), end(v), begin(value), [&scale](const auto& u) -> auto { return u * scale; });
	return std::move(value);
}
template<typename scalar_t, size_t dimension>
components_t<scalar_t, dimension> operator *(const components_t<scalar_t, dimension>& u, const scalar_t& scale)
{
	return std::move(scale * u);
}
template<typename scalar_t, size_t dimension>
components_t<scalar_t, dimension> operator +(const components_t<scalar_t, dimension>& u, const components_t<scalar_t, dimension>& v)
{
	components_t<scalar_t, dimension> value;
	std::transform(begin(u), end(u), begin(v), begin(value), [](const auto& a, const auto& b) -> auto { return a + b; });
	return std::move(value);
}

#if defined( DIRECTX_VECTOR )
#include <DirectXMath.h>
template<template<typename, size_t> class components_type>
struct components_helper<components_type, float, 4>
{
public:
	enum
	{
		dimension_size = 4,
	};
	typedef float scalar_type;
	typedef DirectX::XMVECTORF32 container_type;
	typedef DirectX::XMVECTOR    intermediate_type;

	scalar_type& operator[](const size_t index) { return static_cast<this_type*>(this)->container.f[index]; }
	const scalar_type& operator[](const size_t index) const { return static_cast<const this_type*>(this)->container.f[index]; }

private:
	typedef components_type<scalar_type, dimension_size> this_type;
};

float* begin(components_t<float, 4>& u)
{
	return &u.container.f[0];
}
const float* begin(const components_t<float, 4>& u)
{
	return &u.container.f[0];
}
const float* end(const components_t<float, 4>& u)
{
	return &u.container.f[4];
}

components_t<float, 4> operator *(const float& scale, const components_t<float, 4>& v)
{
	return std::move(v.container * scale);
}
components_t<float, 4> operator *(const components_t<float, 4>& u, const float& scale)
{
	return std::move(scale * u);
}
components_t<float, 4> operator +(const components_t<float, 4>& u, const components_t<float, 4>& v)
{
	return std::move(u.container + v.container);
}
#endif // #if defined( DIRECTX_VECTOR )


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

public:
	enum
	{
		dimension_size = traits::population_count,
	};
	using components_type = typename components_t<scalar_t, dimension_size>              ;
	using scalar_type     = typename components_t<scalar_t, dimension_size>::scalar_type;

	vector_t() : components() {};
	vector_t(const vector_t<scalar_t, space_mask>& v) : components(v.components) {};
	explicit vector_t(const components_type& v) : components(v) {};
	explicit vector_t(components_type&& v) : components(v) {};

	template<typename... scalars>
	explicit vector_t(scalars&&... coords) : components{std::forward<scalars>(coords)...} {}

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
auto operator *(const scalar_t& scale, const vector_t<scalar_t, space_mask>& v)
{
	return vector_t<scalar_t, space_mask>(std::move(v.components * scale));
}
template<typename scalar_t, size_t space_mask>
auto operator *(const vector_t<scalar_t, space_mask>& u, const scalar_t& scale)
{
	return std::move(scale * u);
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



#include <sstream>
#include <string>

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

template<template<typename> class traits, typename field_type, size_t dimension_size>
std::string to_string(const vector_t<field_type, dimension_size>& vec)
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


#if (TEST_PATH == 0)
#include <iostream>
#include <iomanip>
#include <fstream>
template<typename type_t> using out_t = raw_t<type_t>;
int main()
{
	enum
	{
		e0  = (1 << 0), 	e1  = (1 << 1),
		e2  = (1 << 2), 	e3  = (1 << 3),
		e4  = (1 << 4), 	e5  = (1 << 5),
		e6  = (1 << 6), 	e7  = (1 << 7),
		e8  = (1 << 8), 	e9  = (1 << 9),
		e10 = (1 << 10),	e11 = (1 << 11),
		e12 = (1 << 12),	e13 = (1 << 13),
		e14 = (1 << 14),	e15 = (1 << 15),
	};

	typedef vector_t<float, e0 | e2 | e7 | e15>                  vector_type1;
	typedef vector_t<long double, e0 | e1 | e2 | e7 | e13 | e15> vector_type2;
	vector_type1 test1{-1.0f,-1.0f,-1.0f,-1.0f}; // sets all 4 components
	vector_type1 test2;
	vector_type2 test3{-1.0f,-1.0f}; // only sets components for e0 and e1, all other being 0.
	vector_type2 test4;
	static_assert(sizeof(vector_type1) == vector_type1::dimension_size * sizeof(vector_type1::scalar_type), "vector size is incorrect...");
	static_assert(sizeof(vector_type2) == vector_type2::dimension_size * sizeof(vector_type2::scalar_type), "vector size is incorrect...");

	vector_type1::scalar_type coeff1, coeff2;

	const std::string input_filename = "../../tmp/test_input.in";
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

		test3.get<e0>()  = test1.cget<e0>();
		//test3.get<e1>()  = test1.cget<e1>(); // this would be fine and would not have any effect! not doing it just to check explicitly data set in constructor
		test3.get<e2>()  = test1.cget<e2>();
		test3.get<e7>()  = test1.cget<e7>();
		//test3.get<e13>() = test1.get<e13>(); // this would be fine and would not have any effect! not doing it just to check for unitialized data...
		test3.get<e15>() = test1.cget<e15>();
		test4.get<e0>()  = test2.cget<e0>();
		//test4.get<e1>() = test2.cget<e1>(); // this would be fine and would not have any effect! not doing it just to check for unitialized data...
		test4.get<e2>()  = test2.cget<e2>();
		test4.get<e7>()  = test2.cget<e7>();
		test4.get<e13>() = test2.cget<e13>(); // this is fine even if test1 does not have any e1 component!
		test4.get<e15>() = test2.cget<e15>();
	}

	vector_type2::scalar_type coeff1d = coeff1, coeff2d = coeff2;
	auto test_result1 = coeff1 * test1 * coeff2 + test2;
	auto test_result2 = coeff1d * test3 * coeff2d + test4;
	//auto test_result3 = coeff1 * test1 * coeff2 + test4; // this will fail compilation (vector types are incompatible) ... eventually this should be fixed as it all fits into destination
	std::cout << "testing... (press 'd' to delete input file)" << std::endl;

	// checking both const and non-const accessors
	std::cout << "("
		<< test_result1.cget<e0>() << ", "
	    << test_result1.get<e2>() << ", "
		<< test_result1.get<e7>() << ", "
		<< test_result1.get<e15>()
		<< ")" << std::endl;

	std::cout << "0 = "
		<< test_result1.cget<0>()       << " = "
		<< test_result1.get<e1 | e2>() << " = "
		<< test_result1.get<e3>()
		<< std::endl;

	std::cout << to_string<out_t>(test_result1) << std::endl;
	std::cout << to_string<out_t>(test_result2) << std::endl;

	int data = std::cin.get();
	if (data == 'd')
		std::remove(input_filename.c_str());
}
#endif // // #if (TEST_PATH == 0)
#if (TEST_PATH == 1)
#include <iostream>
#include <iomanip>
int main()
{
	typedef bit_traits<0xAB> traits;
	std::cout << traits::value << " (" << traits::population_count << ")";
	std::cout << " -> (";
	static_for_each<0, traits::population_count, get_bit_helper<traits>, increment_bit_index_helper<traits>>::iterate(
		[](size_t value, size_t cur_loop) -> void
		{
			char* delimiter = ((cur_loop == 0) ? "" : ",");
			std::cout << delimiter << value;
		}
	);
	std::cout << ")";
	std::cout << " ~ (";
	static_for_each<traits::get_bit<0>::value, traits::get_bit<traits::population_count>::value, get_bit_index_helper<traits>, next_bit_helper<traits>>::iterate(
		[](size_t value, size_t cur_loop) -> void
		{
			char* delimiter = ((cur_loop == traits::get_bit<0>::value) ? "" : ",");
			std::cout << delimiter << value;
		}
	);
	std::cout << ")" << std::endl;
	
	enum
	{
		e0 = (1 << 0),
		e1 = (1 << 1),
		e2 = (1 << 2),
		e3 = (1 << 3),
		e4 = (1 << 4),
		e5 = (1 << 5),
	};
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

	std::cin.get();
}
#endif // #if (TEST_PATH == 1)
#if (TEST_PATH == 2)
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



#include <sstream>
#include <string>

template<typename type_t>
struct latex_t;
template<typename type_t>
struct raw_t;

template<typename field_t, size_t dimension>
struct latex_t<module_t<field_t, dimension>>
{
	static constexpr std::string prefix()    { return "\\left("; }
	static constexpr std::string delimiter() { return ",\\,"; }
	static constexpr std::string postfix()   { return "\\right)"; }
};
template<typename field_t, size_t dimension>
struct raw_t<module_t<field_t, dimension>>
{
	static constexpr std::string prefix()    { return "{"; }
	static constexpr std::string delimiter() { return ","; }
	static constexpr std::string postfix()   { return "}"; }
};

template<template<typename> class traits, typename field_type, size_t dimension_size>
std::string to_string(const module_t<field_type, dimension_size>& vec)
{
	typedef module_t<field_type, dimension_size> type_t;
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
std::string to_string(const uniform_multimodule_t<module_type, rank_size>& multivec)
{
	typedef uniform_multimodule_t<module_type, rank_size> uniform_multimodule_type;

	enum { dimension = uniform_multimodule_type::dimension };
	typedef typename uniform_multimodule_type::field_t    field_type;

	typedef module_t<field_type, dimension>       module_type;
	return to_string<traits>(static_cast<const module_type&>(multivec));
}
*/

#include <iostream>

template<typename type_t> using out_t = raw_t<type_t>;
int main()
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
	uniform_multimodule_type multivec5_4({
		uniform_multimodule_type::dimension,
	});
	std::cout << decltype(multivec5_4)::dimension << std::endl;
	std::cout << to_string<out_t>(multivec5_4) << std::endl;

	std::cin.get();
}
#endif // #if (TEST_PATH == 2)
