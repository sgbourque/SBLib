#include <test_common.h>
using namespace SBLib::Test;
#ifdef USE_CURRENT_TEST
#undef USE_CURRENT_TEST
#endif
#define USE_CURRENT_TEST 0

template<typename functor_type>
struct invoke_helper
{
private:
	template<size_t first_index, size_t... indices>
	struct invoke_internal
	{
		template<typename... arg_types>
		static auto call(std::tuple<arg_types...>&& arguments)
		{
			using tuple_t = std::tuple<arg_types...>;
			return invoke_internal<first_index - 1, first_index, indices...>::call(std::forward<tuple_t>(arguments));
		}
	};
	template<size_t... indices>
	struct invoke_internal<0, indices...>
	{
		template<typename... arg_types>
		static auto call(std::tuple<arg_types...>&& arguments)
		{
			using tuple_t = std::tuple<arg_types...>;
			return functor_type::call(std::get<0>(std::forward<tuple_t>(arguments)), std::get<indices>(std::forward<tuple_t>(arguments))...);
		}
	};
public:

	template<typename... arg_types>
	static auto call(std::tuple<arg_types...>&& arguments)
	{
		using tuple_t = std::tuple<arg_types...>;
		return invoke_internal<sizeof...(arg_types)-1>::call(std::move(arguments));
	}
};

struct sum
{
	static constexpr auto call() { return 0; }

	template<typename first_type, typename... arg_types>
	static constexpr auto call(first_type&& argument0, arg_types&&... arguments)
	{
		return std::move(argument0) + std::move(call(std::forward<arg_types>(arguments)...));
	}
};
struct product
{
	static constexpr auto call() { return 1; }

	template<typename first_type, typename... arg_types>
	static constexpr auto call(first_type&& argument0, arg_types&&... arguments)
	{
		return std::move(argument0) * std::move(call(std::forward<arg_types>(arguments)...));
	}
};

template<typename functor_type, typename... arg_types>
struct uneval
{
	uneval() = delete;
	uneval(const uneval&) = delete;
	uneval& operator =(const uneval&) = delete;

	uneval(uneval&&) = default;
	uneval& operator =(uneval&&) = default;

	//explicit uneval(const std::remove_reference_t<arg_types>&... args) : container{ std::remove_reference_t<arg_types>(args)... } {}
	//explicit uneval(std::remove_reference_t<arg_types>&&... args) : container{ std::forward<arg_types>(args)... } {}
	explicit uneval(const arg_types&... args) : container{ args... } {}
	explicit uneval(arg_types&&... args) : container{ std::forward<arg_types>(args)... } {}
	explicit uneval(std::tuple<arg_types...>&& args) : container{ std::move(args) } {}

	std::tuple<arg_types...> container;

	auto operator ()()
	{
		auto result = invoke_helper<functor_type>::call(std::move(container));
		return std::move(result);
	}
};
//template<typename... arg_types1, typename functor_type, typename... arg_types2>
//auto operator +(uneval<sum, arg_types1...>&& result0, uneval<functor_type, arg_types2...>&& result1)
//{
//	return result0 + result1();
//}
template<typename... arg_types1, typename... arg_types2>
auto operator +(uneval<sum, arg_types1...>&& result0, uneval<sum, arg_types2...>&& result1)
{
	auto arguments = std::tuple_cat(std::forward<decltype(result0.container)>(result0.container), std::forward<decltype(result1.container)>(result1.container));
	static_assert(std::is_same_v<decltype(arguments), std::tuple<arg_types1..., arg_types2...>>, "?");
	uneval<sum, arg_types1..., arg_types2...> result(std::move(arguments));
	return std::move(result);
}
template<typename... arg_types1, typename type>
auto operator +(uneval<sum, arg_types1...>&& result0, type&& remainder)
{
	auto last_argument = std::make_tuple(std::forward<type>(remainder));
	auto arguments = std::tuple_cat(std::forward<decltype(result0.container)>(result0.container), std::move(last_argument));
	static_assert(std::is_same_v<decltype(arguments), std::tuple<arg_types1..., type>>, "?");
	uneval<sum, arg_types1..., type> result(std::move(arguments));
	return std::move(result);
}

template<bool...>    struct bool_pack;
template<bool... bs> using all_true = std::is_same<bool_pack<bs..., true>, bool_pack<true, bs...>>;
template<bool... bs> using all_false = std::is_same<bool_pack<bs..., false>, bool_pack<false, bs...>>;
template<bool... bs> using any_true = typename std::conditional<all_false<bs...>::value, std::false_type, std::true_type>::type;
template<bool... bs> using any_false = typename std::conditional<all_true<bs...>::value, std::false_type, std::true_type>::type;

template<typename... arg_types> using any_reference = any_true<std::is_reference<arg_types>::value...>;

static_assert(!any_reference<int, double>::value, "yeah");
static_assert(any_reference<int&, int>::value, "yeah");

// This is "safer" but create a copy of the arguments
//template<typename functor_type, typename... arg_types>
//typename std::enable_if<any_reference<arg_types...>::value, uneval<functor_type, arg_types...>>::type invoke(arg_types&&... arguments)
//{
//	//static_assert(any_reference<arg_types...>::value, "yeah");
//	uneval<functor_type, arg_types...> delayed(std::forward<arg_types>(arguments)...);
//	return std::move(delayed);
//}
//// This is "unsafe" but keep track of the references
//template<typename functor_type, typename... arg_types>
//typename std::enable_if<!any_reference<arg_types...>::value, uneval<functor_type, arg_types...>>::type invoke(const arg_types&... arguments)
//{
//	static_assert(!any_reference<arg_types...>::value, "yeah");
//	uneval<functor_type, arg_types...> delayed(arguments...);
//	return std::move(delayed);
//}

//
// Tests
//
template<typename functor_type, typename... arg_types>
typename std::enable_if<any_reference<arg_types...>::value, uneval<functor_type, std::decay<arg_types>...>>::type invoke(const arg_types&... arguments)
{
	static_assert(any_reference<arg_types...>::value, "yeah");
	uneval<functor_type, arg_types...> delayed(arguments...);
	return std::move(delayed);
}
template<typename functor_type, typename... arg_types>
typename std::enable_if<!any_reference<arg_types...>::value, uneval<functor_type, arg_types...>>::type invoke(const arg_types&... arguments)
{
	static_assert(!any_reference<arg_types...>::value, "yeah");
	uneval<functor_type, arg_types...> delayed(arguments...);
	return std::move(delayed);
}

template<typename functor_type, typename... arg_types1, typename... arg_types2, typename... arg_types3>
auto invoke(uneval<functor_type, arg_types1...>&& first, uneval<functor_type, arg_types2...>&& second, arg_types3&&... arguments)
{
	auto first_arguments = std::tuple_cat(std::forward<decltype(first.container)>(first.container), std::forward<decltype(second.container)>(second.container));
	auto firstEval = uneval<functor_type, arg_types1..., arg_types2...>(std::move(first_arguments));
	return invoke<functor_type>(std::move(firstEval), std::forward<arg_types3>(arguments)...);
}
template<typename functor_type, typename... arg_types1>
auto invoke(uneval<functor_type, arg_types1...>&& fct)
{
	return std::forward<uneval<functor_type, arg_types1...>>(fct);
}

template<typename functor_type, typename... arg_types>
auto invoke(std::tuple<arg_types...>&& arguments)
{
	return invoke_helper<functor_type>::call(std::forward<decltype(arguments)>(arguments));
}

class test_dangerous_lambda : RegisteredFunctor
{
	template<template<typename...> typename base_traits, typename... arg_types>
	struct operator_traits
	{
		static auto operation(arg_types... args) { return base_traits<arg_types>::operation(std::forward_as_tuple(args...)); }
	};


	template<typename unary_arg>
	struct twice_traits
	{
		using return_type = unary_arg;
		static auto operation(std::tuple<unary_arg> a)
		{
			auto value = std::get<0>(a) * 2;
			std::cout << "twice " << std::get<0>(a) << " -> " << value << std::endl;
			return 2 * std::get<0>(a);
		};
	};
	template<typename unary_arg>
	struct half_traits
	{
		using return_type = unary_arg;
		static auto operation(std::tuple<unary_arg> a)
		{
			auto value = std::get<0>(a) / 2;
			std::cout << "half " << std::get<0>(a) << " -> " << value << std::endl;
			return value;
		};
	};


	template<template<typename...> typename traits, typename... arg_types>
	struct uneval
	{
		using operation_traits = traits<arg_types...>;

		uneval(arg_types&&... args) : container{ std::forward<arg_types>(args)... } {}
		operator typename operation_traits::return_type() { return operation_traits::operation(container); }

		std::tuple<arg_types...> container;
	};

	template<template<typename...> typename traits, typename... arg_types>
	struct twice_traits<uneval<traits, arg_types...>> : twice_traits<typename traits<arg_types...>::return_type> {};

	template<typename A>
	static auto twice(A a)
	{
		std::cout << "Preparing twice" << std::endl;
		return std::move(uneval<twice_traits, A>(std::move(a)));
	};
	template<typename A>
	static auto half(A a)
	{
		std::cout << "Preparing half" << std::endl;
		return std::move(uneval<half_traits, A>(std::move(a)));
	};
	template<template<typename...> typename T, typename A>
	static auto twice(uneval<T, A>&& a)
	{
		std::cout << "Force evaluation" << std::endl;
		return uneval<twice_traits, int>((typename T<A>::return_type)a);
	};
	template<template<typename...> typename T, typename A>
	static auto half(uneval<T, A>&& a)
	{
		std::cout << "Force evaluation" << std::endl;
		return uneval<half_traits, int>((typename T<A>::return_type)a);
	};

	template<typename A>
	static auto twice(const uneval<half_traits, A>& a)
	{
		std::cout << "Shorcut twice(half) == identity" << std::endl;
		return std::move(std::get<0>(a.container));
	};
	template<typename A>
	static auto twice(uneval<half_traits, A>&& a)
	{
		std::cout << "Shorcut twice(half) == identity (rvalue)" << std::endl;
		return std::move(std::get<0>(a.container));
	};
	template<typename A>
	static auto half(const uneval<twice_traits, A>& a)
	{
		std::cout << "Shorcut half(twice) == identity" << std::endl;
		return std::move(std::get<0>(a.container));
	};
	template<typename A>
	static auto half(uneval<twice_traits, A>&& a)
	{
		std::cout << "Shorcut half(twice) == identity (rvalue)" << std::endl;
		return std::move(std::get<0>(a.container));
	};

	template<typename unary_arg>
	struct force_evaluation_traits
	{
		using return_type = unary_arg;
		static auto operation(std::tuple<unary_arg> a)
		{
			auto value = std::get<0>(a);
			std::cout << "forcing evaluation of " << value << std::endl;
			return value;
		};
	};
	template<typename A>
	static auto force_evaluation(A a)
	{
		std::cout << "Breaking optimizations..." << std::endl;
		return std::move(uneval<force_evaluation_traits, A>(std::move(a)));
	}

	static auto dangerous()
	{
		using type = int;
		type a;
		std::cin >> a;
		//std::cout << "--------------" << std::endl;
		type a1_2 = half(a);
		//std::cout << "..." << std::endl;
		type a1 = twice(a1_2);
		//std::cout << "..." << std::endl;
		//std::cout << "half(" << a << ") = " << a1_2 << std::endl;
		//std::cout << "twice(" << a1_2 << ") = " << a1 << std::endl;

		auto do_sum = invoke<sum>(invoke<sum>(a, a1_2, a1), invoke<sum>(a, a1), invoke<sum>(3, std::move(a)));
		std::cout << do_sum() << " = 3 * " << a << " + 2 * " << a1 << " + " << a1_2 << " + 3 = " << (3 * a + 2 * a1 + a1_2 + 3) << std::endl;
		std::cin >> a;
		a1_2 = half(a);
		a1 = twice(a1_2);
		std::cout << do_sum() << " = 3 * " << a << " + 2 * " << a1 << " + " << a1_2 << " + 3 = " << (3 * a + 2 * a1 + a1_2 + 3) << std::endl;

		//std::cout << "--------------" << std::endl;
		//auto a1_2_opt    = half(a);
		//std::cout << "..." << std::endl;
		//auto a1_opt      = twice(a1_2_opt);
		//std::cout << "..." << std::endl;
		//auto a1_full_opt = twice(half(a));
		//std::cout << "..." << std::endl;
		//std::cout << "twice(half(" << a << ")) = " << a1_full_opt << " = " << a1_opt << " ~ twice(" << a1_2_opt << ")" << std::endl;

		//std::cout << "--------------" << std::endl;
		//auto a1_forced_eval = twice(force_evaluation(half(a)));
		//std::cout << "..." << std::endl;
		//std::cout << "twice(half(" << a << ")) = " << a1_forced_eval << std::endl;

		return do_sum;//;std::move(do_sum);
	}

	test_dangerous_lambda() : RegisteredFunctor("test_dangerous_lambda", fct) {}
	static void fct()
	{
		auto do_sum = dangerous();
		std::cout << do_sum() << std::endl;
	}
	static test_dangerous_lambda instance;
};
#if USE_CURRENT_TEST
test_dangerous_lambda test_dangerous_lambda::instance;
#endif // #if USE_CURRENT_TEST
