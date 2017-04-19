#define USE_MULTIVECTOR 1
#include <test_common.h>
#include <Mathematics/binomial_coefficient.h>
#include <array>

constexpr float operator "" _f ( unsigned long long v ) { return static_cast<float>( v ); }

namespace SBLib::Test
{
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
		using container_t = std::array<field_t, dimension>;

		module_t()
			: components()
		{}

		template<typename... coords_t>
		module_t( coords_t&&... coords )
			: components{{ std::forward<coords_t>(coords)... }}
		{}

		container_t components;
	};

	//
	// uniform_multimodule_t
	//
	template<typename module_type, size_t rank_size>
	struct uniform_multimodule_t
		: module_t<typename module_type::field_t, SBLib::binomial_coefficient<module_type::dimension, rank_size>::value>
	{
		using super = module_t<typename module_type::field_t, SBLib::binomial_coefficient<module_type::dimension, rank_size>::value>;
		enum { rank = rank_size };

		uniform_multimodule_t()
			: super()
		{}
		template<typename... coords_t>
		uniform_multimodule_t( coords_t&&... coords )
			: super{ std::forward<coords_t>(coords)... }
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

	test_multivector_space() : RegisteredFunctor("test_multivector_space", fct) {}
	static void fct()
	{
		module_t<float, 1> bin0({
			float(SBLib::binomial_coefficient<0,0>::value),
		});
		module_t<float, 2> bin1({
			float(SBLib::binomial_coefficient<1, 0>::value),
			float(SBLib::binomial_coefficient<1, 1>::value),
		});
		module_t<float, 3> bin2({
			float(SBLib::binomial_coefficient<2, 0>::value),
			float(SBLib::binomial_coefficient<2, 1>::value),
			float(SBLib::binomial_coefficient<2, 2>::value),
		});
		module_t<float, 4> bin3({
			float(SBLib::binomial_coefficient<3, 0>::value),
			float(SBLib::binomial_coefficient<3, 1>::value),
			float(SBLib::binomial_coefficient<3, 2>::value),
			float(SBLib::binomial_coefficient<3, 3>::value),
		});
		module_t<float, 5> bin4({
			float(SBLib::binomial_coefficient<4, 0>::value),
			float(SBLib::binomial_coefficient<4, 1>::value),
			float(SBLib::binomial_coefficient<4, 2>::value),
			float(SBLib::binomial_coefficient<4, 3>::value),
			float(SBLib::binomial_coefficient<4, 4>::value),
		});
		module_t<float, 6> bin5({
			float(SBLib::binomial_coefficient<5, 0>::value),
			float(SBLib::binomial_coefficient<5, 1>::value),
			float(SBLib::binomial_coefficient<5, 2>::value),
			float(SBLib::binomial_coefficient<5, 3>::value),
			float(SBLib::binomial_coefficient<5, 4>::value),
			float(SBLib::binomial_coefficient<5, 5>::value),
		});
		module_t<float, 7> bin6({
			float(SBLib::binomial_coefficient<6, 0>::value),
			float(SBLib::binomial_coefficient<6, 1>::value),
			float(SBLib::binomial_coefficient<6, 2>::value),
			float(SBLib::binomial_coefficient<6, 3>::value),
			float(SBLib::binomial_coefficient<6, 4>::value),
			float(SBLib::binomial_coefficient<6, 5>::value),
			float(SBLib::binomial_coefficient<6, 6>::value),
		});
		module_t<float, 8> bin7({
			float(SBLib::binomial_coefficient<7, 0>::value),
			float(SBLib::binomial_coefficient<7, 1>::value),
			float(SBLib::binomial_coefficient<7, 2>::value),
			float(SBLib::binomial_coefficient<7, 3>::value),
			float(SBLib::binomial_coefficient<7, 4>::value),
			float(SBLib::binomial_coefficient<7, 5>::value),
			float(SBLib::binomial_coefficient<7, 6>::value),
			float(SBLib::binomial_coefficient<7, 7>::value),
		});
		module_t<float, 9> bin8({
			float(SBLib::binomial_coefficient<8, 0>::value),
			float(SBLib::binomial_coefficient<8, 1>::value),
			float(SBLib::binomial_coefficient<8, 2>::value),
			float(SBLib::binomial_coefficient<8, 3>::value),
			float(SBLib::binomial_coefficient<8, 4>::value),
			float(SBLib::binomial_coefficient<8, 5>::value),
			float(SBLib::binomial_coefficient<8, 6>::value),
			float(SBLib::binomial_coefficient<8, 7>::value),
			float(SBLib::binomial_coefficient<8, 8>::value),
		});
		module_t<float, 10> bin9({
			float(SBLib::binomial_coefficient<9, 0>::value),
			float(SBLib::binomial_coefficient<9, 1>::value),
			float(SBLib::binomial_coefficient<9, 2>::value),
			float(SBLib::binomial_coefficient<9, 3>::value),
			float(SBLib::binomial_coefficient<9, 4>::value),
			float(SBLib::binomial_coefficient<9, 5>::value),
			float(SBLib::binomial_coefficient<9, 6>::value),
			float(SBLib::binomial_coefficient<9, 7>::value),
			float(SBLib::binomial_coefficient<9, 8>::value),
			float(SBLib::binomial_coefficient<9, 9>::value),
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
			float(uniform_multimodule_type::dimension), float(decltype(bin5)::dimension), 4.0f
		});
		std::cout << decltype(multivec6_4)::dimension << std::endl;
		std::cout << to_string<out_t>(multivec6_4) << std::endl;
	}
	static test_multivector_space instance;
};
test_multivector_space test_multivector_space::instance;
} // namespace SBLib::Test
