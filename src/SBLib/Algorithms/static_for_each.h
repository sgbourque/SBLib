#pragma once

template<size_t begin, size_t end, class get_helper, class increment_helper>
struct static_for_each
{
	template<class fct_type>
	static void iterate(fct_type fct)
	{
		fct(get_helper::get_helper<begin>::value, begin);
		static_for_each<increment_helper::increment_helper<begin>::value, end, get_helper, increment_helper>::iterate(fct);
	}
};
template<size_t end, class get_helper, class increment_helper>
struct static_for_each<end, end, get_helper, increment_helper>
{
	template<class fct_type>
	static void iterate(fct_type)
	{
		// TODO / C++17: use [[unused]] fct here instead so we can still use it for debugging if needed later...
	}
};
