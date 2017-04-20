#pragma once
#define USE_DIRECTX_VECTOR 0

#if USE_MULTIVECTOR
#include <Mathematics/multivector.h>
#endif

#include <algorithm>
#include <functional>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <unordered_map>

namespace SBLib::Test
{

#if USE_MULTIVECTOR
template<typename scalar_t, size_t space_mask, size_t rank>
using graded_multivector_t = SBLib::graded_multivector_t<scalar_t, space_mask, rank>;
template<typename scalar_t, size_t space_mask>
using vector_t = SBLib::vector_t<scalar_t, space_mask>;
#endif

#define USING_STATIC_FOR_EACH_OUTPUT	USE_MULTIVECTOR // setting this to 1 will use very heavy static code (test)
#define USING_STANDARD_BASIS        	0

#if USE_DIRECTX_VECTOR
#define USE_DIRECTX_VECTOR_HACK 1 // only use it to test wedge/hodge in 3D. It will fail to compile in other dimensions (static_assert is there for a good reason)
#endif // #if USE_DIRECTX_VECTOR

//
// RegisteredFunctor : common test registration
//
class RegisteredFunctor
{
public:
	static bool eval(size_t id)
	{
		data_t& common_data = GetCommonData();
		const bool found = (id < common_data.functionList.size());
		if (found)
		{
			auto& fct = common_data.functionMap[common_data.functionList[id]];
			fct();
		}
		return found;
	}
	static size_t size()
	{
		data_t& common_data = GetCommonData();
		return common_data.functionMap.size();
	}
	static void help(std::ostream& out)
	{
		data_t& common_data = GetCommonData();
		for (size_t id = 0; id < common_data.functionMap.size(); ++id)
		{
			const auto& fctName = common_data.functionList[id];
			out << id << ": " << fctName << std::endl;
		}
	}
	static std::string get_name(size_t id)
	{
		data_t& common_data = GetCommonData();
		return id < size() ? common_data.functionList[id] : "";
	}


protected:
	using fct_type = void();
	RegisteredFunctor(std::string name, fct_type fct)
	{
		data_t& common_data = GetCommonData();
		auto class_name_contructor_name_begin = name.find_last_of(':');
		if (class_name_contructor_name_begin != std::string::npos)
		{
			functionName = name.substr(class_name_contructor_name_begin + 1);
		}
		else if ( name.size() > 0 )
		{
			functionName = name;
		}
		if ( functionName.size() > 0 )
		{
			common_data.functionMap[functionName] = fct;
			common_data.functionList.emplace_back(functionName);
			sort();
		}
	}
	const std::string& get_name()
	{
		return functionName;
	}
	const size_t get_id()
	{
		data_t& common_data = GetCommonData();
		return std::distance(common_data.functionList.begin(), std::find(common_data.functionList.begin(), common_data.functionList.end(), get_name()));
	}

private:
	void sort()
	{
		data_t& common_data = GetCommonData();
		std::sort(common_data.functionList.begin(), common_data.functionList.end());
	}
	std::string functionName;

	struct data_t
	{
		using  functionMap_t = std::unordered_map<std::string, std::function<fct_type>>;
		functionMap_t functionMap;

		using  functionList_t = std::vector<std::string>;
		functionList_t functionList;
	};
	static data_t& GetCommonData()
	{
		static data_t common_data;
		return common_data;
	}
};

//
// common multivector i/o
//
template<typename type_t>
struct latex_t;
template<typename type_t>
struct raw_t;
template<typename type_t>
struct algebraic_t;

#if USE_MULTIVECTOR
template<typename field_t, size_t mask, size_t rank>
struct latex_t<graded_multivector_t<field_t, mask, rank>>
{
	static constexpr const char* prefix() { return "\\left("; }
	static constexpr const char* delimiter() { return ",\\,"; }
	static constexpr const char* postfix() { return "\\right)"; }
};
template<typename field_t, size_t mask, size_t rank>
struct raw_t<graded_multivector_t<field_t, mask, rank>>
{
	static constexpr const char* prefix() { return "{"; }
	static constexpr const char* delimiter() { return ", "; }
	static constexpr const char* postfix() { return "}"; }
};
template<typename field_t, size_t mask, size_t rank>
struct algebraic_t<graded_multivector_t<field_t, mask, rank>>
{
	static constexpr const char* prefix() { return "("; }
	static constexpr const char* delimiter() { return " + "; }
	static constexpr const char* postfix() { return ")"; }
};
#endif

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
					enum { first_bit = SBLib::get_bit<0>(bit_mask), };
					const std::string delimiter = (loop == first_bit) ? "" : outer_basis_traits::delimiter();
					out << delimiter << outer_basis_traits::symbol() << outer_basis_traits::accessor_prefix() << bit_index << outer_basis_traits::accessor_postfix();
				}
			};
		};

	public:
		template<typename scalar_t, size_t space_mask, size_t rank_size>
		do_action(std::ostream& out, const graded_multivector_t<scalar_t, space_mask, rank_size>& V)
		{
			using type_t = graded_multivector_t<scalar_t, space_mask, rank_size>;
			using output_traits_t = output_traits<type_t>;
			const std::string delimiter = (loop == 0) ? "" : output_traits_t::delimiter();
			const std::string prefix    = (SBLib::bit_count(bit_mask) <= 1) ? " " : output_traits_t::prefix();
			const std::string postfix   = (SBLib::bit_count(bit_mask) <= 1) ? "" : output_traits_t::postfix();
			out << delimiter << get<bit_mask>(V) << prefix;
			using output = output_basis<outer_basis_traits<output_traits_t>>;
			SBLib::for_each_bit_index<bit_mask>::template iterate< output::template do_action >(out);
			out << postfix;
		}

		template<typename scalar_t, size_t space_mask>
		do_action(std::ostream& out, const graded_multivector_t<scalar_t, space_mask, 0>& V)
		{
			out << get<bit_mask>(V);
		}
	};
};
#endif // #if USING_STATIC_FOR_EACH_OUTPUT

#if USE_MULTIVECTOR
template<template<typename> class output_traits, typename field_type, size_t space_mask, size_t rank_size>
inline std::string to_string(const graded_multivector_t<field_type, space_mask, rank_size>& V)
{
	using type_t = graded_multivector_t<field_type, space_mask, rank_size>;
	using output_traits_t = output_traits<type_t>;

	std::stringstream ss;
	ss << output_traits_t::prefix();

#if USING_STATIC_FOR_EACH_OUTPUT
	using output = output_multivector_components<output_traits>;
	SBLib::for_each_combination< SBLib::select_combinations<space_mask, rank_size> >::template iterate< output::template do_action >(ss, V);
	//SBLib::for_each_combination<SBLib::select_combinations<space_mask, rank_size>>::template iterate< output_multivector_components<output_traits>::do_action >(ss, V);
#else // #if USING_STATIC_FOR_EACH_OUTPUT
	std::string delimiter;
	for (size_t index = 0; index < V.dimension_size; ++index)
	{
		ss << delimiter << V.components[index];
		delimiter = output_traits_t::delimiter();
	}
#endif // #if USING_STATIC_FOR_EACH_OUTPUT

	ss << output_traits_t::postfix();
	return ss.str();
}

#if USING_STATIC_FOR_EACH_OUTPUT
template<typename type_t> using out_t = algebraic_t<type_t>;
#else // #if USING_STATIC_FOR_EACH_OUTPUT
template<typename type_t> using out_t = raw_t<type_t>;
#endif // #if USING_STATIC_FOR_EACH_OUTPUT

template<typename field_type, size_t space_mask, size_t rank_size>
inline std::ostream& operator <<(std::ostream& out, const graded_multivector_t<field_type, space_mask, rank_size>& V)
{
	return out << to_string<out_t>(V);
}

template<typename field_type, size_t space_mask, size_t rank_size>
inline std::istream& operator >>(std::istream& in, graded_multivector_t<field_type, space_mask, rank_size>& V)
{
	for (size_t index = 0; index < V.dimension_size; ++index)
		in >> V.components[index];
	return in;
}
#endif
} // namespace SBLib::Test
