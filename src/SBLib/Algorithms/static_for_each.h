#pragma once
#include <functional>

template<size_t begin, size_t end, class get_helper, class increment_helper>
struct static_for_each
{
	template<template<size_t, size_t> class fct_type, typename... type_t>
	static void iterate(type_t&&... types)
	{
		fct_type<get_helper::get_helper<begin>::value, begin>(types...);
		static_for_each<increment_helper::increment_helper<begin>::value, end, get_helper, increment_helper>::iterate<fct_type>(types...);
	}
};
template<size_t end, class get_helper, class increment_helper>
struct static_for_each<end, end, get_helper, increment_helper>
{
	template<template<size_t, size_t> class fct_type, typename... type_t>
	static void iterate(type_t&&...)
	{
	}
};
