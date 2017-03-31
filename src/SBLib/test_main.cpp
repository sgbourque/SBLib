#include <test_common.h>
#include <intrin.h>
using namespace SBLib::Test;

namespace SBLib::Test
{
RegisteredFunctor::data_t RegisteredFunctor::common_data;
}

#define USING_NAIVE_IMPLEMENTATION 0

template<typename operator_type>
struct invoke_operator
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
			return operator_type::evaluate(std::get<0>(std::forward<tuple_t>(arguments)), std::get<indices>(std::forward<tuple_t>(arguments))...);
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


template<typename operator_type, typename... operand_type>
struct operator_traits;

//
// identity_operator : defers evaluation of operands
//
struct identity_operator
{
	template<typename... T>
	static auto assign(T... args)
	{
		return operator_node<identity_operator, tree_depth_construct, T...>{std::forward<T>(args)...}.leave();
	}

	template<typename... T>
	static auto evaluate(T... operands)
	{
		using traits = operator_traits<identity_operator, T...>;
		return traits::evaluate(std::forward<T>(operands)...);
	}

	static constexpr char* name() { return "identity"; }
};
template<typename T>
struct operator_traits<identity_operator, T>
{
	using result_type = T;
	static auto evaluate(T t)
	{
		return t;
	}
};



using tree_depth_evaluated = std::integral_constant<long long, -1>;
using tree_depth_root      = std::integral_constant<long long, 0>;
using tree_depth_construct = std::integral_constant<long long, 1>;

template<typename operator_type, typename node_depth, typename... T>
struct operator_node
{
	enum : typename node_depth::value_type { tree_depth = node_depth::value, };
	using this_type      = typename operator_node<operator_type, node_depth, T...>;
	using construct_type = typename operator_node<operator_type, std::integral_constant<typename node_depth::value_type, tree_depth + 1>, T...>;
	using result_type    = typename operator_traits<operator_type, T...>::result_type;
	using return_type    = typename operator_node<operator_type, std::integral_constant<typename node_depth::value_type, (tree_depth - 1)>, T...>;

	operator_node(T... op) : operands{ std::forward<T>(op)... } {}

	template< typename T = std::enable_if_t<(tree_depth > 0)> >
	auto leave()
	{
		static_assert(tree_depth != 0, "WTF?");
		std::cout << "Evaluating a simple move at no cost..." << std::endl;
		return return_type( std::move(*this) );
	}

	// proceed to partial operator evaluation
	template< typename = std::enable_if_t<(tree_depth == 0)> >
	result_type leave()
	{
		static_assert(tree_depth == 0, "WTF?");
		// This should normally only evaluates any leftover references
		std::cout << "Evaluating all references of " << operator_type::name() << " operation (faked)..." << std::endl;
		return this->operator ()();
	}

	// proceed to full operator evaluation
	auto operator ()()
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

	std::tuple<T...> operands;

private:
	operator_node& operator =(operator_node&&) = default;
};

//
// sum_operator :
// - need to implement user-called interface (usually operator +) tbat will call assign on supported types
// - need to implement operator_traits<operator, types...> for all supported combinations on the type.
// c.f., demo with finite field below for a simple use.
//
// TODO : find a "nice" way to forward flat variadic evaluation (see manually expanded demo with finite field below)
//
struct sum_operator
{
	template<typename... T>
	static auto assign(T... args)
	{
		return operator_node<sum_operator, tree_depth_construct, T...>{std::forward<T>(args)...}.leave();
	}

	template<typename... T>
	static auto evaluate(T... operands)
	{
		using traits = operator_traits<sum_operator, T...>;
		return traits::evaluate(std::forward<T>(operands)...);
	}

	static constexpr char* name() { return "sum"; }
};


////////////////////////////////////////////////////////////////////
//
// Lets do some testing with some demo case...
// Demo : sum on finite fields of integer mod p
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
template<size_t prime_number>
auto operator+(my_finite_field_p<prime_number> a, my_finite_field_p<prime_number> b)
{
	return sum_operator::assign(a, b);
}
template<size_t prime_number>
struct operator_traits<sum_operator, my_finite_field_p<prime_number>, my_finite_field_p<prime_number>>
{
	using value_type  = my_finite_field_p<prime_number>;
	using result_type = operator_node<identity_operator, tree_depth_evaluated, value_type>;
	static auto evaluate(value_type a, value_type b)
	{
		return result_type( value_type(a.get() + b.get()) );
	}
};

//
// Normally we could build up a generic system to do recursion but lets just do a simple proof of concept of the optimization system...
// ... simply imagine we already have the way to build up variadic number of my_finite_field_p<prime_number>'s
//
template<size_t prime_number, typename node_depth>
auto operator+( const operator_node< sum_operator, node_depth, my_finite_field_p<prime_number>, my_finite_field_p<prime_number> >& a, my_finite_field_p<prime_number> b )
{
	return sum_operator::assign(std::get<0>(a.operands), std::get<1>(a.operands), b);
}

template<size_t prime_number>
struct operator_traits<sum_operator, my_finite_field_p<prime_number>, my_finite_field_p<prime_number>, my_finite_field_p<prime_number>>
{
	using value_type        = my_finite_field_p<prime_number>;
	using sub_operator_type = operator_node<sum_operator, tree_depth_evaluated, value_type, value_type>;
	using result_type       = operator_node<identity_operator, tree_depth_evaluated, value_type>;
	static auto evaluate(value_type a, value_type b, value_type c)
	{
		return result_type(value_type(a.get() + b.get() +c.get()));
	}
};

#endif // #if USING_NAIVE_IMPLEMENTATION


class test : RegisteredFunctor
{
	using field_type = my_finite_field_p<7>;

	static auto internal()
	{
		field_type a;
		field_type b;
		field_type c;
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

		return sum4.leave(); // yay! "partially" evaluates expression (currently fully evaluates it for now)...
	}

	test() : RegisteredFunctor(__FUNCTION__, fct) {}
	static void fct()
	{
		auto sum = internal();
		auto eval_sum = sum();
		static_assert(std::is_same_v<decltype(eval_sum), field_type>, "Wrong return type from operation..." ); // This helps trapping the possible illegal move operation
		std::cout << "Result : " << eval_sum;
	}
	static test instance;
};
test test::instance;

int main()
{
	bool quit = false, help = false;
	size_t testCase = ~0uLL;
	do
	{
		if (RegisteredFunctor::size() > 1)
		{
			quit = help = false;
			testCase = ~0uLL;
			std::cout << "Enter test case [0 - " << RegisteredFunctor::size() - 1 << "] or 'q' to quit, 'h' for help : ";
			std::cin >> testCase;
			while (!std::cin.good())
			{
				std::cin.clear();
				int data = std::cin.get();
				quit = (data == 'q');
				help = (data == 'h');
			}
		}
		else
		{
			testCase = ~testCase;
			quit = (testCase != 0);
		}

		if (quit)
		{
			std::cout << "bye";
		}
		else if (!help)
		{
			std::string testName = RegisteredFunctor::get_name(testCase);
			std::cout << std::setw(testName.length()) << std::setfill('-') << "" << std::endl << std::setw(0) << std::setfill(' ');
			std::cout << testName << std::endl;
			std::cout << std::setw(testName.length()) << std::setfill('-') << "" << std::endl << std::setw(0) << std::setfill(' ') << std::endl;

			if (!RegisteredFunctor::eval(testCase))
			{
				std::cout << "... test not found, please try again!" << std::endl;
				help = true;
			}
		}

		if (help)
			RegisteredFunctor::help(std::cout);

		std::cout << std::endl;
	}
	while (std::cin.good() && !quit);

	if (RegisteredFunctor::size() < 2)
		std::cin.get();
}
