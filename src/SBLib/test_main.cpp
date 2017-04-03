#include <test_common.h>
#include <intrin.h>

#include <Algorithms/counter.h>

template< size_t counter_value = SBLib::get_counter() >
using next_token = SBLib::counter_token< counter_value >;

template< size_t counter_value = SBLib::get_counter() >
constexpr auto get_token() { return SBLib::counter_token< counter_value >(); }


enum
{
	k0_glob = SBLib::get_counter(), g0_glob = __COUNTER__,
	k1_glob = SBLib::get_counter(), g1_glob = __COUNTER__,
	k2_glob = SBLib::get_counter(), g2_glob = __COUNTER__,
	k3_glob = SBLib::get_counter(), g3_glob = __COUNTER__,
	k4_glob = SBLib::get_counter(), g4_glob = __COUNTER__,
	k5_glob = SBLib::get_counter(), g5_glob = __COUNTER__,
	k6_glob = SBLib::get_counter(), g6_glob = __COUNTER__,
	k7_glob = SBLib::get_counter(), g7_glob = __COUNTER__,
};


namespace SBLib::Test
{
class test : RegisteredFunctor
{
	enum
	{
		k0_test = get_counter(), g0_test = __COUNTER__,
		k1_test = get_counter(), g1_test = __COUNTER__,
		k2_test = get_counter(), g2_test = __COUNTER__,
	};

	test() : RegisteredFunctor(__FUNCTION__, fct) {}
	static void fct()
	{
		enum
		{
			k0 = get_counter(), g0 = __COUNTER__,
			k1 = get_counter(), g1 = __COUNTER__,
			k2 = get_counter(), g2 = __COUNTER__,
		};
		size_t v0 = get_counter();
		constexpr auto c0 = get_counter();
		constexpr auto p0 = __COUNTER__;

		size_t v1 = get_counter();
		constexpr auto c1 = get_counter();
		constexpr auto p1 = __COUNTER__;

		size_t v2 = get_counter();
		constexpr auto c2 = get_counter();
		constexpr auto p2 = __COUNTER__;

		size_t v3 = get_counter();
		constexpr auto c3 = get_counter();
		constexpr auto p3 = __COUNTER__;

		size_t v4 = get_counter();
		// ...

		std::cout
			<< c0 << " " << c1 << " " << c2 << " " << c3 << " : "
			<< p0 << " " << p1 << " " << p2 << " " << p3 << " : "
			<< v0 << " " << v1 << " " << v2 << " " << v3 << " " << v4
			<< std::endl;
		static_assert( (c0 >= 0) && (c1 > c0) && (c2 > c1) && (c3 > c2), "counters are not counting correctly...");

		using token0 = next_token<>;
		using token1 = next_token<>;
		auto dummy0 = token0();
		auto dummy1 = token1();
		std::cout
			<< typeid(dummy0).name() << std::endl
			<< typeid(dummy1).name() << std::endl;
		static_assert( !std::is_same_v<token0, token1>, "Distinct tokens should not be of the same type!" );
	}
	static test instance;
};
test test::instance;
}

int main()
{
	using namespace SBLib::Test;

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
