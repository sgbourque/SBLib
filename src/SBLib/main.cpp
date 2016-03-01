#include <Algorithms/static_for_each.h>
#include <Mathematics/binomial_coefficient.h>

#include <array>

// Temp while constexpr isnt supported, remove this when it becomes available
//#define constexpr

#include <Traits/bit_traits.h>
#include <Traits/clifford_traits.h>

template<size_t embedding_space_bits, typename scalar_type>
struct module
{
	typedef bit_traits<embedding_space_bits> bit_traits_t;
	enum
	{
		dimension_size = bit_traits_t::population_count,
	};

	typedef scalar_type scalar_t;
	typedef std::array<scalar_t, dimension_size> container_t;

	container_t coordinates;
};

// TODO : use SSE & other optimizations as needed

#if 0

#include <iostream>
#include <iomanip>
int main()
{
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
	
	std::cout << "((e2 ^ e0) ^ e1) = "
		<< alternating_traits<(1<<2)|(1<<0), (1<<1)>::sign
		<< " * (e2 ^ e1 ^ e0)"
		<< std::endl;

	std::cout << "((e5 ^ e3 ^ e2) ^ (e4 ^ e0)) = "
		<< alternating_traits<(1<<5)|(1<<3)|(1<<2), (1<<4)|(1<<0)>::sign
		<< " * (e5 ^ e4 ^ e3 ^ e2 ^ e0)"
		<< std::endl;

	std::cout << "((e5 ^ e3 ^ e2) ^ (e5 ^ e1)) = "
		<< alternating_traits<(1<<5)|(1<<3)|(1<<2), (1<<5)|(1<<1)>::sign
		<< " * (e5 ^ e5 ^ e3 ^ e2 ^ e1)"
		<< std::endl;

	std::cin.get();
}

#endif


#if 0


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
