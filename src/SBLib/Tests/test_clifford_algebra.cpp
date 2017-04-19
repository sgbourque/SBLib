#include <test_common.h>
#include <Traits/clifford_traits.h>

namespace SBLib::Test
{
class test_clifford_algebra : public RegisteredFunctor
{
	enum
	{
		e0 = (1 << 0),
		e1 = (1 << 1),
		e2 = (1 << 2),
		e3 = (1 << 3),
		e4 = (1 << 4),
		e5 = (1 << 5),
	};
	typedef SBLib::bit_traits<0xAB> traits;

	template<size_t first_loop_value>
	struct output_bit
	{
		template<size_t value, size_t loop>
		struct do_action
		{
			do_action(std::ostream& out)
			{
				static const char* const delimiter = ((loop == first_loop_value) ? "" : ",");
				out << delimiter << value;
			}
		};
	};

	test_clifford_algebra() : RegisteredFunctor("test_clifford_algebra", fct) {}
	static void fct()
	{
		std::cout << traits::value << " (" << bit_count(traits::value) << ")";
		std::cout << ": -> (";
		SBLib::for_each_bit<traits::value>::iterate<output_bit<0>::do_action>(std::cout);
		std::cout << ") ~ (";
		SBLib::for_each_bit_index<traits::value>::iterate<output_bit<get_bit<0>(traits::value)>::do_action>(std::cout);
		std::cout << ") ~ (";
		SBLib::for_each_bit_compoment<traits::value>::iterate<output_bit<get_bit<0>(traits::value)>::do_action>(std::cout);
		std::cout << ")" << std::endl;

		std::cout << "((e0 ^ e2) ^ e1) \n\t= "
			<< SBLib::alternating_traits<(e0 ^ e2), e1>::sign
			<< " * (e0 ^ e1 ^ e2) \n\t= "
			<< SBLib::reversion_conjugacy_traits<(e0 ^ e2)>::sign * SBLib::alternating_traits<(e2 ^ e0), e1, false>::sign
			<< " * (e2 ^ e1 ^ e0)"
			<< std::endl;

		std::cout << "((e5 ^ e3 ^ e2) ^ (e4 ^ e0)) \n\t= "
			<< SBLib::reversion_conjugacy_traits<(e5 ^ e3 ^ e2)>::sign * SBLib::reversion_conjugacy_traits<(e4 ^ e0)>::sign * SBLib::alternating_traits<(e2 ^ e3 ^ e5), (e0 ^ e4)>::sign
			* SBLib::reversion_conjugacy_traits<(e5 ^ e4 ^ e3 ^ e2 ^ e0)>::sign
			<< " * (e5 ^ e4 ^ e3 ^ e2 ^ e0) \n\t= "
			<< SBLib::alternating_traits<(e5 ^ e3 ^ e2), (e4 ^ e0), false>::sign * SBLib::reversion_conjugacy_traits<(e5 ^ e4 ^ e3 ^ e2 ^ e0)>::sign
			<< " * (e0 ^ e2 ^ e3 ^ e4 ^ e5)"
			<< std::endl;

		std::cout << "((e5 ^ e3 ^ e2) ^ (e5 ^ e1)) \n\t= "
			<< SBLib::reversion_conjugacy_traits<(e5 ^ e3 ^ e2)>::sign * SBLib::reversion_conjugacy_traits<(e5 ^ e1)>::sign * SBLib::alternating_traits<(e2 ^ e3 ^ e5), (e1 ^ e5)>::sign
			* SBLib::reversion_conjugacy_traits<(e5 ^ e5 ^ e3 ^ e2 ^ e1)>::sign
			<< " * (e5 ^ e5 ^ e3 ^ e2 ^ e1) \n\t= "
			<< SBLib::alternating_traits<(e5 ^ e3 ^ e2), (e5 ^ e1), false>::sign * SBLib::reversion_conjugacy_traits<(e5 ^ e5 ^ e3 ^ e2 ^ e1)>::sign
			<< " * (e1 ^ e2 ^ e3 ^ e5 ^ e5)"
			<< std::endl;
	}

	static test_clifford_algebra instance;
};
test_clifford_algebra test_clifford_algebra::instance;
} // namespace SBLib::Test
