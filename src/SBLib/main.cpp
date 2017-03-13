#include <Algorithms/static_for_each.h>
#include <Mathematics/binomial_coefficient.h>
#include <Traits/bit_traits.h>
#include <Traits/clifford_traits.h>


#include <array>
#define DIRECTX_VECTOR

template<typename type_t>
constexpr bool is_power_of_two(const type_t& value)
{
	static_assert(std::is_integral<type_t>::value, "type is not integral.");
	return value && ((value & (value - 1)) == 0);
}




template<typename scalar_t, size_t dimension>
struct coordinates_t
{
private:
	typedef bit_traits<(1 << dimension) - 1> traits;
public:
	enum
	{
		dimension_size = traits::population_count,
	};
	static_assert( dimension_size == dimension, "dimension mismatch" );
	typedef scalar_t scalar_type;
	typedef std::array<scalar_type, dimension_size> container_type;

	coordinates_t() = default;
	coordinates_t(const coordinates_t<scalar_type, dimension_size>& v): container(v.container) {};

	explicit coordinates_t(const container_type& v) : container(v) {};
	template<typename... scalars> explicit coordinates_t(scalar_type&& first, scalars&&... coords) : container{ first, std::forward<scalars>(coords)... } {}

	coordinates_t(container_type&& v) : container(v) {}

	operator container_type&() { return container; }
	operator const container_type&() const { return container; }

	scalar_t& operator[](const size_t index) { return container[index]; }
	const scalar_t& operator[](const size_t index) const { return container[index]; }

	container_type container;
};
template<typename scalar_t, size_t dimension>
coordinates_t<scalar_t, dimension> operator *(const float& scale, const coordinates_t<scalar_t, dimension>& v)
{
	coordinates_t<scalar_t, dimension> value;
	std::transform(v.container.begin(), v.container.end(), value.container.begin(), [&scale](const auto& u) -> auto { return u * scale; });
	return std::move(value);
}
template<typename scalar_t, size_t dimension>
coordinates_t<scalar_t, dimension> operator *(const coordinates_t<scalar_t, dimension>& u, const float& scale)
{
	return std::move(scale * u);
}
template<typename scalar_t, size_t dimension>
coordinates_t<scalar_t, dimension> operator +(const coordinates_t<scalar_t, dimension>& u, const coordinates_t<scalar_t, dimension>& v)
{
	coordinates_t<scalar_t, dimension> value;
	std::transform(u.container.begin(), u.container.end(), v.container.begin(), value.container.begin(), [](const auto& a, const auto& b) -> auto { return a + b; });
	return std::move(value);
}

#if defined( DIRECTX_VECTOR )
#include <DirectXMath.h>
template<>
struct coordinates_t<float, 4>
{
private:
	typedef bit_traits<(1 << 4) - 1> traits;
public:
	enum
	{
		dimension_size = traits::population_count,
	};
	typedef float scalar_type;
	typedef DirectX::XMVECTORF32 container_type;
	typedef DirectX::XMVECTOR    intermediate_type;

	coordinates_t() = default;
	coordinates_t(const coordinates_t<scalar_type, dimension_size>& v) = default;

	explicit coordinates_t(const container_type& v) : container(v) {};
	explicit coordinates_t(const intermediate_type& v) { container.v = v; };
	template<typename... scalars> explicit coordinates_t(scalar_type&& first, scalars&&... coords): container{first, std::forward<scalars>(coords)...} {}

	coordinates_t(container_type&& v) : container(v) {}
	coordinates_t(intermediate_type&& v) { container.v = std::move(v); }

	operator container_type&() { return container; }
	operator const container_type&() const { return container; }

	scalar_type&  operator[](const size_t index) { return container.f[index]; }
	const scalar_type& operator[](const size_t index) const { return container.f[index]; }

	container_type container;
};

coordinates_t<float, 4> operator *(const float& scale, const coordinates_t<float, 4>& v)
{
	return std::move(v.container * scale);
}
coordinates_t<float, 4> operator *(const coordinates_t<float, 4>& u, const float& scale)
{
	return std::move(scale * u);
}
coordinates_t<float, 4> operator +(const coordinates_t<float, 4>& u, const coordinates_t<float, 4>& v)
{
	return std::move(u.container + v.container);
}
#endif

template<typename scalar_t, size_t space_mask>
struct vector_t
{
private:
	template<size_t subspace_mask>
	struct get_traits_helper
	{
		typedef scalar_t& reference_type;
		typedef const scalar_t& const_reference_type;
	};
	template<>
	struct get_traits_helper<0>
	{
		typedef scalar_t reference_type;
		typedef scalar_t const_reference_type;
	};
	template<size_t subspace_mask>
	struct get_traits
	{
		static const size_t mask = is_power_of_two(subspace_mask) ? (subspace_mask & space_mask) : 0;
		typedef typename get_traits_helper<mask>::reference_type reference_type;
		typedef typename get_traits_helper<mask>::const_reference_type const_reference_type;
	};

	template<size_t subspace_mask>
	constexpr typename get_traits<subspace_mask>::reference_type get_helper()
	{
		return coordinates[traits::get_bit_index<subspace_mask>()];
	}
	template<>
	constexpr scalar_t get_helper<0>()
	{
		return scalar_t(0);
	}
	template<size_t subspace_mask>
	constexpr typename get_traits<subspace_mask>::const_reference_type get_helper() const
	{
		return coordinates[traits::get_bit_index<subspace_mask>()];
	}
	template<>
	constexpr scalar_t get_helper<0>() const
	{
		return scalar_t(0);
	}

public:
	typedef bit_traits<space_mask> traits;
	enum
	{
		dimension_size = traits::population_count,
	};
	typedef typename coordinates_t<scalar_t, dimension_size>              coordinates_type;
	typedef typename coordinates_t<scalar_t, dimension_size>::scalar_type scalar_type;

	vector_t() = default;
	vector_t(const vector_t<scalar_t, space_mask>& v) = default;
	explicit vector_t(const coordinates_type& v) : coordinates(v) {};
	explicit vector_t(coordinates_type&& v) : coordinates(v) {};

	template<typename... scalars>
	explicit vector_t(scalar_t&& first, scalars&&... coords) : coordinates{first, std::forward<scalars>(coords)...} {}


	template<size_t subspace_mask>
	constexpr typename get_traits<subspace_mask>::reference_type get()
	{
		return get_helper<get_traits<subspace_mask>::mask>();
	}
	template<size_t subspace_mask>
	constexpr typename get_traits<subspace_mask>::const_reference_type get() const
	{
		return get_helper<get_traits<subspace_mask>::mask>();
	}
	template<size_t subspace_mask>
	constexpr typename get_traits<subspace_mask>::const_reference_type cget() const
	{
		return get<subspace_mask>();
	}

	coordinates_type coordinates;
};


template<typename scalar_t, size_t space_mask>
vector_t<scalar_t, space_mask> operator *(const scalar_t& scale, const vector_t<scalar_t, space_mask>& v)
{
	return vector_t<scalar_t, space_mask>(std::move(v.coordinates * scale));
}
template<typename scalar_t, size_t space_mask>
vector_t<scalar_t, space_mask> operator *(const vector_t<scalar_t, space_mask>& u, const scalar_t& scale)
{
	return std::move(scale * u);
}
template<typename scalar_t, size_t space_mask>
vector_t<scalar_t, space_mask> operator +(const vector_t<scalar_t, space_mask>& u, const vector_t<scalar_t, space_mask>& v)
{
	return vector_t<scalar_t, space_mask>(std::move(u.coordinates + v.coordinates));
}


//template<typename scalar_t, size_t subspace_mask0, ...>
//struct multivector_t
//{
//
//};

#if 1

#include <iostream>
#include <iomanip>
#include <fstream>
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

	vector_t<float, e0 | e2 | e7 | e15> test;
	vector_t<float, e0 | e2 | e7 | e15> test2;

	float coeff;
	{
		const std::string filename = "../../tmp/test_input.in";
		auto file = std::fstream(filename, std::ios_base::in | std::ios_base::_Nocreate);
		std::istream& in = file.is_open() ? file : std::cin;

		in >> coeff;
		in >> test.get<e0>();
		in >> test.get<e2>();
		in >> test.get<e7>();
		in >> test.get<e15>();
		in >> test2.get<e0>();
		in >> test2.get<e2>();
		in >> test2.get<e7>();
		in >> test2.get<e15>();

		if (!file.is_open())
		{
			file.open(filename, std::ios_base::out);
			if (file.is_open())
			{
				file << coeff;
				file << " " << test.get<e0>();
				file << " " << test.get<e2>();
				file << " " << test.get<e7>();
				file << " " << test.get<e15>();
				file << " " << test2.get<e0>();
				file << " " << test2.get<e2>();
				file << " " << test2.get<e7>();
				file << " " << test2.get<e15>();
			}
		}
	}

	auto test3 = coeff * test * 2.0f + test2;
	static_assert( sizeof(test) == 4 * sizeof(float), "vector size is incorrect..." );
	std::cout << "testing..." << std::endl;

	std::cout << "("
		<< test3.cget<e0>() << ", "
	    << test3.get<e2>() << ", "
		<< test3.get<e7>() << ", "
		<< test3.get<e15>()
		<< ")" << std::endl;

	std::cout << "0 = "
		<< test3.cget<0>()       << " = "
		<< test3.get<e1 | e2>() << " = "
		<< test3.get<e3>()
		<< std::endl;

	std::cin.get();
}

#elif 1

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
		<< alternating_traits<(e2 ^ e0), e1>::sign
		<< " * (e0 ^ e1 ^ e2) \n\t= "
		<< -alternating_traits<(e0 ^ e2), e1, false>::sign
		<< " * (e2 ^ e1 ^ e0)"
		<< std::endl;

	std::cout << "((e5 ^ e3 ^ e2) ^ (e4 ^ e0)) \n\t= "
		<< alternating_traits<(e5 ^ e3 ^ e2), (e4 ^ e0)>::sign
		<< " * (e5 ^ e4 ^ e3 ^ e2 ^ e0) \n\t= "
		<< +alternating_traits<(e2 ^ e3 ^ e5), (e0 ^ e4), false>::sign
		<< " * (e0 ^ e2 ^ e3 ^ e4 ^ e5)"
		<< std::endl;

	std::cout << "((e5 ^ e3 ^ e2) ^ (e5 ^ e1)) \n\t= "
		<< alternating_traits<(e5 ^ e3 ^ e2), (e5 ^ e1)>::sign
		<< " * (e5 ^ e5 ^ e3 ^ e2 ^ e1) \n\t= "
		<< +alternating_traits<(e2 ^ e3 ^ e5), (e1 ^ e5), false>::sign
		<< " * (e1 ^ e2 ^ e3 ^ e5 ^ e5)"
		<< std::endl;

	std::cin.get();
}

#elif 1


//
// module_t
//
template<typename field_type, size_t dimension_size>
struct module_t
{
	typedef field_type field_t;
	enum { dimension = dimension_size };

	module_t()
		: coordinates()
	{}
	module_t(std::array<field_t, dimension>&& coords)
		: coordinates(coords)
	{}

	std::array<field_t, dimension> coordinates;
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
using scalar_t = uniform_multimodule_t<module_type, 0>;

template<typename module_type>
using vector_t = uniform_multimodule_t<module_type, 1>;

template<typename module_type>
using pseudovector_t = uniform_multimodule_t<module_type, module_type::dimension - 1>;

template<typename module_type>
using pseudoscalar_t = uniform_multimodule_t<module_type, module_type::dimension>;


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
	for (const auto& value : vec.coordinates)
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

#endif
