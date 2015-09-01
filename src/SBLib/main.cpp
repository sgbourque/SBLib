#include <array>

template<typename field_type, size_t dimension_size>
struct vector_t
{
	typedef field_type field_t;
	enum { dimension = dimension_size };

	vector_t()
		: coordinates()
	{}
	vector_t(std::array<field_t, dimension>&& coords)
		: coordinates(coords)
	{}

	std::array<field_t, dimension> coordinates;
};


template<size_t dimension_size, size_t rank_size>
struct binomial_coefficient
{
	// using Pascal triangle construction
	enum
	{
		value = binomial_coefficient<dimension_size - 1, rank_size - 1>::value
		      + binomial_coefficient<dimension_size - 1, rank_size    >::value
	};
};

template<>
struct binomial_coefficient<0, 0>
{
	enum { value = 1 };
};

template<size_t dimension_size>
struct binomial_coefficient<dimension_size, 0>
{
	enum { value = 1 };
};

template<size_t dimension_size>
struct binomial_coefficient<dimension_size, dimension_size>
{
	enum { value = 1 };
};

template<typename vector_type, size_t rank_size>
struct multivector_t
	: vector_t<typename vector_type::field_t,
	           binomial_coefficient<vector_type::dimension, rank_size>::value>
{
	multivector_t()
		: vector_t()
	{}
	multivector_t(std::array<field_t, dimension>&& coords)
		: vector_t(std::move(coords))
	{}
};

template<typename vector_type>
struct outervector_t
	: vector_t<typename vector_type::field_t, (1 << vector_type::dimension)>
{
	outervector_t()
		: vector_t()
	{}
	outervector_t(std::array<field_t, dimension>&& coords)
		: vector_t(std::move(coords))
	{}
};

#include <sstream>
#include <string>

template<typename type_t>
struct latex_t;
template<typename type_t>
struct raw_t;

template<typename field_t, size_t dimension>
struct latex_t<vector_t<field_t, dimension>>
{
	static std::string prefix()    { return "\\left(";  }
	static std::string delimiter() { return ",\\,"; }
	static std::string postfix()   { return "\\right)"; }
};
template<typename field_t, size_t dimension>
struct raw_t<vector_t<field_t, dimension>>
{
	static std::string prefix()    { return "{"; }
	static std::string delimiter() { return ","; }
	static std::string postfix()   { return "}"; }
};

template<template<typename> class traits, typename field_type, size_t dimension_size>
std::string to_string(const vector_t<field_type, dimension_size>& vec)
{
	typedef vector_t<field_type, dimension_size> type_t;
	typedef traits<type_t> traits_t;

	std::stringstream ss;
	ss << traits_t::prefix();
	std::string delimiter;
	for (const auto& value : vec.coordinates)
		(ss << delimiter << value), (delimiter = traits_t::delimiter());
	ss << traits_t::postfix();

	std::string value;
	ss >> value;
	return value;
}
/*
template<template<typename> class traits, typename vector_type, size_t rank_size>
std::string to_string(const multivector_t<vector_type, rank_size>& multivec)
{
	typedef multivector_t<vector_type, rank_size> multivector_type;

	enum { dimension = multivector_type::dimension };
	typedef typename multivector_type::field_t    field_type;

	typedef vector_t<field_type, dimension>       vector_type;
	return to_string<traits>(static_cast<const vector_type&>(multivec));
}
*/
#include <iostream>

template<typename type_t> using out_t = raw_t<type_t>;
int main()
{
	vector_t<float, 1> bin0({
		binomial_coefficient<0,0>::value,
	});
	vector_t<float, 2> bin1({
		binomial_coefficient<1, 0>::value,
		binomial_coefficient<1, 1>::value,
	});
	vector_t<float, 3> bin2({
		binomial_coefficient<2, 0>::value,
		binomial_coefficient<2, 1>::value,
		binomial_coefficient<2, 2>::value,
	});
	vector_t<float, 4> bin3({
		binomial_coefficient<3, 0>::value,
		binomial_coefficient<3, 1>::value,
		binomial_coefficient<3, 2>::value,
		binomial_coefficient<3, 3>::value,
	});
	vector_t<float, 5> bin4({
		binomial_coefficient<4, 0>::value,
		binomial_coefficient<4, 1>::value,
		binomial_coefficient<4, 2>::value,
		binomial_coefficient<4, 3>::value,
		binomial_coefficient<4, 4>::value,
	});
	vector_t<float, 6> bin5({
		binomial_coefficient<5, 0>::value,
		binomial_coefficient<5, 1>::value,
		binomial_coefficient<5, 2>::value,
		binomial_coefficient<5, 3>::value,
		binomial_coefficient<5, 4>::value,
		binomial_coefficient<5, 5>::value,
	});
	vector_t<float, 7> bin6({
		binomial_coefficient<6, 0>::value,
		binomial_coefficient<6, 1>::value,
		binomial_coefficient<6, 2>::value,
		binomial_coefficient<6, 3>::value,
		binomial_coefficient<6, 4>::value,
		binomial_coefficient<6, 5>::value,
		binomial_coefficient<6, 6>::value,
	});
	vector_t<float, 8> bin7({
		binomial_coefficient<7, 0>::value,
		binomial_coefficient<7, 1>::value,
		binomial_coefficient<7, 2>::value,
		binomial_coefficient<7, 3>::value,
		binomial_coefficient<7, 4>::value,
		binomial_coefficient<7, 5>::value,
		binomial_coefficient<7, 6>::value,
		binomial_coefficient<7, 7>::value,
	});
	vector_t<float, 9> bin8({
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
	vector_t<float, 10> bin9({
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

	typedef multivector_t<decltype(bin5), 4> multivector_type;
	multivector_type multivec5_4({
		multivector_type::dimension,
	});
	std::cout << decltype(multivec5_4)::dimension << std::endl;
	std::cout << to_string<out_t>(multivec5_4) << std::endl;

	typedef outervector_t<decltype(bin5)> outervector_type;
	outervector_type outer5({
		outervector_type::dimension,
	});
	std::cout << decltype(outer5)::dimension << std::endl;
	std::cout << to_string<out_t>(outer5) << std::endl;

	std::cin.get();
}