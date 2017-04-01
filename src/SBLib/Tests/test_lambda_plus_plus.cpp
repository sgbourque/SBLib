#include <test_common.h>
using namespace SBLib::Test;
#ifdef USE_CURRENT_TEST
#undef USE_CURRENT_TEST
#endif
#define USE_CURRENT_TEST 1

#define USING_NAIVE_IMPLEMENTATION 0
#define USING_PASS_BY_REF          1

// Test : pass-by-ref or pass-by-value?
#if USING_PASS_BY_REF
using byref_traits = std::true_type;
#else // #if USING_PASS_BY_REF
using byref_traits = std::false_type;
#endif // #else // #if USING_PASS_BY_REF
constexpr bool byref_traits_v = byref_traits::value;


//
// invoke_operator
// It essentially convert a tuple into a parameter pack expension
// (could be made more generic to act on containers others than tuple as an array or the like)
//
// Assumes that template<typename... operand_type> static auto operator_type::evaluate(arg_types...) exists for at least for some argument pack(s) (could be void).
//
template<typename operator_type>
struct invoke_operator
{
private:
	template<size_t first_index, size_t... indices>
	struct invoke_internal
	{
		template<typename... operand_type>
		static auto call(std::tuple<operand_type...>&& arguments)
		{
			using tuple_t = std::tuple<operand_type...>;
			return invoke_internal<first_index - 1, first_index, indices...>::call(std::forward<tuple_t>(arguments));
		}
	};
	template<size_t... indices>
	struct invoke_internal<0, indices...>
	{
		template<typename... operand_type>
		static auto call(std::tuple<operand_type...>&& arguments)
		{
			using tuple_t = std::tuple<operand_type...>;
			return operator_type::evaluate(std::get<0>(std::forward<tuple_t>(arguments)), std::get<indices>(std::forward<tuple_t>(arguments))...);
		}
	};
public:

	template<typename... operand_type>
	static auto call(std::tuple<operand_type...>&& arguments)
	{
		using tuple_t = std::tuple<operand_type...>;
		return invoke_internal<sizeof...(operand_type)-1>::call(std::move(arguments));
	}
};


//
// operator_traits : defines how to operates on operands
//
template<typename operator_type, typename... operand_type>
struct operator_traits;


//
// operator_node for operator tree construction (need some template aliases to prevent having to ever use these tree_depth constant explicitely)
//
#include <type_traits>
template<bool...>    struct bool_pack;
template<bool... bs> using  all_true  = std::is_same<bool_pack<bs..., true>, bool_pack<true, bs...>>;
template<bool... bs> using  all_false = std::is_same<bool_pack<bs..., false>, bool_pack<false, bs...>>;
template<bool... bs> using  any_true  = std::bool_constant<!all_false<bs...>::value>;
template<bool... bs> using  any_false = std::bool_constant<!all_true<bs...>::value>;

template<bool... bs> constexpr bool all_true_v  = all_true<bs...>::value;
template<bool... bs> constexpr bool all_false_v = typename all_false<bs...>::value;
template<bool... bs> constexpr bool any_true_v  = typename any_true<bs...>::value;
template<bool... bs> constexpr bool any_false_v = typename any_false<bs...>::value;

template<template<typename, typename> typename binary_op, typename ref, typename... operand_type>
using compare_all = all_true< binary_op<ref, operand_type>::value... >;
template<template<typename, typename> typename binary_op, typename ref, typename... operand_type>
using compare_any = any_true< binary_op<ref, operand_type>::value... >;

template<template<typename, typename> typename binary_op, typename... operand_type>
constexpr bool compare_all_v = compare_all<binary_op, operand_type...>::value;
template<template<typename, typename> typename binary_op, typename... operand_type>
constexpr bool compare_any_v = compare_any<binary_op, operand_type...>::value;

template<template<typename> typename unary_op, typename... operand_type>
using are_all = all_true< unary_op<operand_type>::value... >;
template<template<typename> typename unary_op, typename... operand_type>
using is_any  = all_true< unary_op<operand_type>::value... >;

template<template<typename> typename unary_op, typename... operand_type>
constexpr bool are_all_v = are_all<unary_op, operand_type...>::value;
template<template<typename> typename unary_op, typename... operand_type>
constexpr bool is_any_v  = is_any<unary_op, operand_type...>::value;


using tree_depth_evaluated = std::integral_constant<long long, -1>;
using tree_depth_root      = std::integral_constant<long long,  0>;
using tree_depth_construct = std::integral_constant<long long,  1>;

template<typename operator_type, typename node_depth, typename... operand_type>
struct operator_node
{
	enum : size_t { operand_count = sizeof...(operand_type), };
	enum : typename node_depth::value_type { tree_depth = node_depth::value, };
	using this_type      = typename operator_node<operator_type, node_depth, operand_type...>;
	using construct_type = typename operator_node<operator_type, std::integral_constant<typename node_depth::value_type, (tree_depth + 1)>, operand_type...>;
	using return_type    = typename operator_node<operator_type, std::integral_constant<typename node_depth::value_type, (tree_depth - 1)>, operand_type...>;
	using traits_type    = typename operator_traits<operator_type, operand_type...>;
	using result_type    = typename traits_type::result_type;
	using partial_type   = typename traits_type::partial_type;

	using container_type     = std::tuple<operand_type...>;
	using first_operand_type = std::tuple_element_t<0, container_type>;

	operator_node(operand_type... operand) : operands{ std::forward<operand_type>(operand)... } {}

	template< typename = std::enable_if_t<(tree_depth > 0)> >
	return_type leave()
	{
		static_assert(tree_depth != 0, "WTF?");
		std::cout << "Evaluating a simple move at no cost..." << std::endl;
		return return_type( std::move(*this) );
	}

	// proceed to partial operator evaluation
	template< typename = std::enable_if_t<(tree_depth == 0)> >
	auto leave()
	{
		//return (partial_type)(*this);
		static_assert(tree_depth == 0, "WTF?");
		// This should normally only evaluates any leftover references
		std::cout << "Evaluating all references of " << operator_type::name() << " operation (faked)..." << std::endl;
		return partial_type( this->operator ()() );
	}
	//operator partial_type()
	//{
	//	static_assert(tree_depth == 0, "WTF?");
	//	// This should normally only evaluates any leftover references
	//	std::cout << "Evaluating all references of " << operator_type::name() << " operation (faked)..." << std::endl;
	//	return partial_type( this->operator ()() );
	//}

	// proceed to full operator evaluation
	auto operator ()()
	{
		return (result_type)(*this);
	}
	operator result_type()
	{
		// unroll parameters first...
		std::cout << "Evaluating full " << operator_type::name() << " operation..." << std::endl;
		auto result = invoke_operator<operator_type>::call(std::move(operands));
		return result;
	}

	// No default, no copy
	operator_node() = delete;
	operator_node(const operator_node&) = delete;
	operator_node& operator =(const operator_node&) = delete;

	operator_node(operator_node&& original) : operands( std::move(original.operands) )
	{
		static_assert((tree_depth != 0), "Should never return a possibly dirty operator. Please use leave().");
	}

	operator_node(construct_type&& original) : operands(std::move(original.operands))
	{
	}

	container_type operands;

private:
	operator_node& operator =(operator_node&&) = default;
};


//
// generic_operator
// Defines basic operators logic, other operators should just simply derive from this one and add whatever they would need into operator_traits.
// You may want to overload name() to have a pretty name debug print...
//
template<typename operator_type>
struct generic_operator
{
	template<typename... operand_type>
	static auto assign(operand_type&&... args)
	{
		return operator_node<operator_type, tree_depth_construct, operand_type...>{std::forward<operand_type>(args)...}.leave();
	}

	template<typename... operand_type>
	static auto evaluate(operand_type&&... operands)
	{
		using traits = operator_traits<operator_type, operand_type...>;
		return traits::evaluate(std::forward<operand_type>(operands)...);
	}

	static constexpr auto name() { return typeid(operator_type).name(); }
};


//
// identity_operator : defers evaluation of operands
//
struct identity_operator : generic_operator<identity_operator>
{
	static constexpr auto name() { return "identity"; }
};
//
// Identity is essentially only well defined on a single operand in general.
//
// However, it could be possible to define it on multiple operand if it makes sense.
// For instance, as a matrix operator, two-sided identity operation would naturally correspond to inner product on its operand.
// This does not mean we should define it as such... but we could if needed...
//
template<typename operand_type>
struct operator_traits<identity_operator, operand_type>
{
	using result_type  = operand_type;
	using partial_type = operator_node<identity_operator, tree_depth_evaluated, operand_type>;
	static auto evaluate(operand_type&& operand)
	{
		return operand;
	}
};


//
// sum_operator :
// - need to implement user-called interface (usually operator +) tbat will call assign on supported types
// - need to implement operator_traits<operator, types...> for all supported combinations on the type.
// c.f., demo with finite field below for a simple use.
//
// TODO : find a "nice" (generic) way to forward flat variadic evaluation (see manually expanded demo with finite field below)
// HINT : The generic way to do it will use operator tree flattening on nested operations on same types and operator_traits's result_type.
// NOTE : general operators are neither commutative nor associative (think of function composition that need not be associative) but
//        associative and/or commutative properties could be useful for generic operator simplification having any of these properties.
//
struct sum_operator : generic_operator<sum_operator>
{
	static constexpr auto name() { return "sum"; }
};


////////////////////////////////////////////////////////////////////
//
// Lets do some testing with some demo case...
// Demo : sum on finite fields of integers mod p
//
template<size_t prime_number>
struct my_finite_field_p
{
	enum :size_t { order = prime_number, };
	//static_assert( is_prime<order>, "Finite field is only defined for prime element count" )

	my_finite_field_p() = default;
	my_finite_field_p(const my_finite_field_p&) = default;
	my_finite_field_p& operator=(const my_finite_field_p&) = default;
	my_finite_field_p(my_finite_field_p&&) = default;
	my_finite_field_p& operator =(my_finite_field_p&&) = default;

	explicit my_finite_field_p( int n ) : value{ (n % prime_number) }
	{
		std::cout << "<--- doing a division --->" << std::endl;
	}

	int get() const { return value; }

#if USING_NAIVE_IMPLEMENTATION
	auto leave() { return std::move(*this); }
	auto operator()() { return *this; }
#endif // #if USING_NAIVE_IMPLEMENTATION

private:
	int value;
};
template<size_t prime_number>
std::istream& operator >>(std::istream& in, my_finite_field_p<prime_number>& n)
{
	int tmpN;
	in >> tmpN;
	n = my_finite_field_p<prime_number>( tmpN );
	return in;
}
template<size_t prime_number>
std::ostream& operator <<(std::ostream& out, my_finite_field_p<prime_number>& n)
{
	return out << n.get();
}


#if USING_NAIVE_IMPLEMENTATION

// Naive addition : let constructor do the hard work
//template<size_t prime_number>
template<size_t prime_number>
auto operator+(my_finite_field_p<prime_number> a, my_finite_field_p<prime_number> b)
{
	return my_finite_field_p<prime_number>(a.get() + b.get());
}

#else // #if USING_NAIVE_IMPLEMENTATION

//
// Optimized approach : automatically sum underlying representation and only do modulo when needed :
// 	(((a % p) + (b % p)) % p) == (a + b) % p;
//
template<size_t prime_number, typename = std::enable_if_t<byref_traits_v>>
auto operator+(const my_finite_field_p<prime_number>& a, const my_finite_field_p<prime_number>& b)
{
	return sum_operator::assign(a, b);
}
template<size_t prime_number, typename = std::enable_if_t<!byref_traits_v>>
auto operator+(my_finite_field_p<prime_number> a, my_finite_field_p<prime_number> b)
{
	return sum_operator::assign(std::move(a), std::move(b));
}

template<size_t prime_number>
struct operator_traits<sum_operator, const my_finite_field_p<prime_number>&, const my_finite_field_p<prime_number>&>
{
	using value_type   = const my_finite_field_p<prime_number>&;
	using result_type  = std::decay_t<value_type>;//operator_node<identity_operator, tree_depth_evaluated, std::decay_t<value_type>>;
	using partial_type = operator_node<identity_operator, tree_depth_evaluated, value_type>;
	static auto evaluate(value_type a, value_type b)
	{
		return result_type( std::decay_t<value_type>(a.get() + b.get()) );
	}
};
template<size_t prime_number>
struct operator_traits<sum_operator, my_finite_field_p<prime_number>, my_finite_field_p<prime_number>>
{
	using value_type   = my_finite_field_p<prime_number>;
	using result_type  = std::decay_t<value_type>;//operator_node<identity_operator, tree_depth_evaluated, std::decay_t<value_type>>;
	using partial_type = operator_node<identity_operator, tree_depth_evaluated, value_type>;
	static auto evaluate(value_type a, value_type b)
	{
		return result_type(std::decay_t<value_type>(a.get() + b.get()));
	}
};


//
// Normally we could build up a generic system to do recursion but lets just do a simple proof of concept of the optimization system...
// ... simply imagine we already have the way to build up variadic number of my_finite_field_p<prime_number>'s
//
//template<size_t prime_number, typename node_depth>
//auto operator+( const operator_node< sum_operator, node_depth, const my_finite_field_p<prime_number>&, const my_finite_field_p<prime_number>& >& a, const my_finite_field_p<prime_number>& b )
//{
//	return sum_operator::assign(std::get<0>(a.operands), std::get<1>(a.operands), b);
//}

namespace std
{
	// Note : I don't like that much to put things in std but anyway...
	template<size_t index, typename... T> constexpr auto&&      get(      operator_node<T...>&& op) { return std::get<index>(op.operands); }
	template<size_t index, typename... T> constexpr auto&       get(      operator_node<T...>&  op) { return std::get<index>(op.operands); }
	template<size_t index, typename... T> constexpr const auto& get(const operator_node<T...>&  op) { return std::get<index>(op.operands); }
}

template<size_t prime_number, typename node_depth>
auto operator+(const operator_node< sum_operator, node_depth, const my_finite_field_p<prime_number>&, const my_finite_field_p<prime_number>& >& a, const my_finite_field_p<prime_number>& b)
{
	return sum_operator::assign(std::get<0>(a), std::get<1>(a), b);
}
template<size_t prime_number, typename node_depth>
auto operator+(operator_node< sum_operator, node_depth, my_finite_field_p<prime_number>, my_finite_field_p<prime_number> >& a, my_finite_field_p<prime_number> b)
{
	return sum_operator::assign(std::move(std::get<0>(a)), std::move(std::get<1>(a)), std::move(b));
}
template<size_t prime_number, typename node_depth>
auto operator+(operator_node< sum_operator, node_depth, my_finite_field_p<prime_number>, my_finite_field_p<prime_number> >&& a, my_finite_field_p<prime_number> b)
{
	return sum_operator::assign(std::move(std::get<0>(a)), std::move(std::get<1>(a)), std::move(b));
}
template<size_t prime_number>
struct operator_traits<sum_operator, const my_finite_field_p<prime_number>&, const my_finite_field_p<prime_number>&, const my_finite_field_p<prime_number>&>
{
	using value_type        = const my_finite_field_p<prime_number>&;
	using sub_operator_type = operator_node<sum_operator, tree_depth_evaluated, value_type, value_type>;
	using result_type       = std::decay_t<value_type>;//operator_node<identity_operator, tree_depth_evaluated, std::decay_t<value_type>>;
	using partial_type      = operator_node<identity_operator, tree_depth_evaluated, std::decay_t<value_type>>;
	static auto evaluate(value_type a, value_type b, value_type c)
	{
		return result_type(std::decay_t<value_type>(a.get() + b.get() +c.get()));
	}
};
template<size_t prime_number>
struct operator_traits<sum_operator, my_finite_field_p<prime_number>, my_finite_field_p<prime_number>, my_finite_field_p<prime_number>>
{
	using value_type        = my_finite_field_p<prime_number>;
	using sub_operator_type = operator_node<sum_operator, tree_depth_evaluated, value_type, value_type>;
	using result_type       = std::decay_t<value_type>;//operator_node<identity_operator, tree_depth_evaluated, value_type>;
	using partial_type      = operator_node<identity_operator, tree_depth_evaluated, std::decay_t<value_type>>;
	static auto evaluate(value_type a, value_type b, value_type c)
	{
		return result_type(std::decay_t<value_type>(a.get() + b.get() + c.get()));
	}
};

#endif // #if USING_NAIVE_IMPLEMENTATION


namespace SBLib::Test
{
class test_lambda_plus_plus : RegisteredFunctor
{
	using field_type = my_finite_field_p<7>;

	static auto internal()
	{
		std::array<field_type, 3> test_array;
		auto& a = std::get<0>(test_array);
		auto& b = std::get<1>(test_array);
		auto& c = std::get<2>(test_array);
		std::cout << std::endl << "Enter 3 integers: ";
		std::cin >> a >> b >> c;
		std::cout << std::endl
			<< "Evaluating "
#if USING_NAIVE_IMPLEMENTATION
			<< " (naive) : "
#else // #if USING_NAIVE_IMPLEMENTATION
			<< " (optimized) : "
#endif // #if USING_NAIVE_IMPLEMENTATION

			<< a << " + " << b << " + " << c << " mod " << field_type::order << std::endl;

		auto sum = a + b;
		//auto sum2 = sum; // illegal copy operation (error C2280)
		//auto sum3 = std::move(sum); // illegal move operation (error C2338) : static_assert "Should never return a possibly dirty operator. Please use leave()."
		auto sum4 = sum + c;
		auto sum5 = a + b + c; // equivalent to all operations up there (hacked to compile as compiler will evaluate it as (a+b)+c and I didnt implement a+(b+c) here...)
		//return sum4; // illegal move operation : This may generate weird compiler error if trying to use return type.
		               // However, it really triggers error C2338 as above even if compiler may go mad so it won't ever compile...

		field_type result = sum5; // equivalent to : auto result = sum5();
		std::cout << std::endl << "Give new value for these 3 int: ";
		if (byref_traits_v)
			std::cout << " (result of " << result << " should be changed accordingly)" << std::endl;
		else
			std::cout << " (result of " << result << " should be left unchanged)" << std::endl;

		std::cin >> a >> b >> c;
		return sum5.leave(); // yay! "partially" evaluates expression (currently fully evaluates it for now)...
	}

	test_lambda_plus_plus() : RegisteredFunctor(__FUNCTION__, fct) {}
	static void fct()
	{
		auto sum = internal();
		auto eval_sum = sum();
		static_assert(std::is_same_v<decltype(eval_sum), field_type>, "Wrong return type from operation..." ); // This helps trapping the possible illegal move operation
		std::cout << "Result : " << eval_sum;
		(void)sum;
	}
	static test_lambda_plus_plus instance;
};
#if USE_CURRENT_TEST
test_lambda_plus_plus test_lambda_plus_plus::instance;
#endif // #if USE_CURRENT_TEST
} // namespace SBLib::Test
