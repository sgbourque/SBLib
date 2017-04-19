#include <test_common.h>
#include <Mathematics/combinations.h>
#ifdef USE_CURRENT_TEST
#undef USE_CURRENT_TEST
#endif
#define USE_CURRENT_TEST 1

namespace SBLib::Test
{
	enum
	{
		e0 = (1 << 0), e1 = (1 << 1),
		e2 = (1 << 2), e3 = (1 << 3),
		e4 = (1 << 4), e5 = (1 << 5),
		e6 = (1 << 6), e7 = (1 << 7),
		e8 = (1 << 8), e9 = (1 << 9),
		e10 = (1 << 10), e11 = (1 << 11),
		e12 = (1 << 12), e13 = (1 << 13),
		e14 = (1 << 14), e15 = (1 << 15),
		e16 = (1 << 16), e17 = (1 << 17),

#if USING_STANDARD_BASIS
		E0 = 0,
		E1 = (E1 | e0),
		E2 = (E1 | e1),
		E3 = (E2 | e2),
		E4 = (E3 | e3),
		E5 = (E4 | e4),
		E6 = (E5 | e5),
		E7 = (E6 | e6),
#else // #if USING_STANDARD_BASIS
		E0 = 0,
		E1 = (E0 | e2),
		E2 = (E1 | e3),
		E3 = (E2 | e5),
		E4 = (E3 | e7),
		E5 = (E4 | e11),
		E6 = (E5 | e13),
		E7 = (E6 | e17),
#endif // #if USING_STANDARD_BASIS

		SPACE_PRINT_DIMENSION_MASK = (e0 | e1 | e2 | e3 | e4 | e7),
	};

	template<size_t dimension> constexpr size_t get_space();
	template<> constexpr size_t get_space<0>() { return E0; }
	template<> constexpr size_t get_space<1>() { return E1; }
	template<> constexpr size_t get_space<2>() { return E2; }
	template<> constexpr size_t get_space<3>() { return E3; }
	template<> constexpr size_t get_space<4>() { return E4; }
	template<> constexpr size_t get_space<5>() { return E5; }
	template<> constexpr size_t get_space<6>() { return E6; }
	template<> constexpr size_t get_space<7>() { return E7; }

class test_combination : public RegisteredFunctor
{
	template<size_t space_mask>
	class output_combinations
	{
	private:
		template<size_t rank_size, size_t loop_index>
		class output_rank
		{
		private:
			template<size_t space_mask>
			struct output_combination
			{
			private:
				template<size_t bit_mask, size_t loop>
				struct do_action_internal
				{
					do_action_internal(std::ostream& out)
					{
						const std::string delimiter = (loop == 0) ? "" : "^";
						out << delimiter << "e" << SBLib::get_bit_index(bit_mask);
					}
				};

			public:
				template<size_t value, size_t loop>
				struct do_action
				{
					do_action(std::ostream& out)
					{
						using traits = SBLib::bit_traits<value>;
						enum : size_t
						{
							index            = SBLib::select_combinations<space_mask, bit_count(value)>::template get_components_index<value>(),

							equiv_space_mask = (1uLL << SBLib::bit_count(space_mask)) - 1,
							equiv_value      = SBLib::select_combinations<equiv_space_mask, bit_count(value)>::template get<index>(),
						};
						using equiv_traits = SBLib::bit_traits<equiv_value>;
						out << "\t\t";
						std::stringstream ss;
						SBLib::for_each_bit<traits::value>::template iterate<do_action_internal>(ss);
						out << std::left;
						out << std::setw(24) << ss.str();
						ss = std::stringstream();
						out << std::setw(1) << " ~ ";
						SBLib::for_each_bit<equiv_traits::value>::template iterate<do_action_internal>(ss);
						out << std::setw(24) << ss.str();
						ss.clear();
						out << std::setw(1) << "\t(" << equiv_value << ")\t";
						out << std::endl;
					}
				};
				template<size_t loop>
				struct do_action<0, loop>
				{
					static_assert(loop == 0, "Invalid loop");
					do_action(std::ostream& out)
					{
						enum : size_t
						{
							value = 0,
							index = SBLib::select_combinations<space_mask, bit_count(0)>::template get_components_index<value>(),

							equiv_space_mask = (1uLL << SBLib::bit_count(space_mask)) - 1,
							equiv_value = SBLib::select_combinations<equiv_space_mask, bit_count(0)>::template get<index>(),
						};
						static_assert(equiv_value == 0, "invalid scalar basis");
						out << "\t\t";
						out << std::left;
						out << std::setw(24) << "1";
						out << std::setw(1) << " ~ ";
						out << std::setw(24) << "1";
						out << std::setw(1) << "\t(" << equiv_value << ")\t";
						out << std::endl;
					}
				};
			};

		public:
			output_rank(std::ostream& out)
			{
				out << "\tRank = " << rank_size << std::endl;
				using output = output_combination<space_mask>;
				SBLib::for_each_combination< SBLib::select_combinations<space_mask, rank_size> >::template iterate< output::template do_action >(out);
			};
		};

	public:
		static void do_action(std::ostream& out)
		{
			out << "Dim = " << SBLib::combinations<space_mask>::dimension_size << ":" << std::endl;
			SBLib::static_for_each<0, SBLib::combinations<space_mask>::dimension_size + 1>::template iterate<output_rank>(out);
		};
	};

	template<size_t bit_index, size_t loop>
	struct do_action
	{
		do_action(std::ostream& out)
		{
			enum : size_t
			{
				space_mask = get_space<bit_index>(),
			};
			output_combinations<space_mask>::do_action(out);
		}
	};

	test_combination() : RegisteredFunctor("test_combination", fct) {}
	static void fct()
	{
		SBLib::for_each_bit_index<SPACE_PRINT_DIMENSION_MASK>::iterate<do_action>(std::cout);
	}
	static test_combination instance;
};
#if USE_CURRENT_TEST
test_combination test_combination::instance;
#endif // #if USE_CURRENT_TEST
} // namespace SBLib::Test
