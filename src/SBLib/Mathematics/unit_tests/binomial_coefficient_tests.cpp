#include <Mathematics/binomial_coefficient.h>

#if !defined( NO_STATIC_UNIT_TESTS ) || defined( UNIT_TESTS )
#define TEST_BINOMIAL_COEFFICIENTS
#endif

#if defined( GENERATE_UNIT_TESTS )
#define GENERATE_TABLE
#endif

#define TEST_TABLE_FILE "Mathematics/unit_tests/binomial_coefficient_ref_table.h"

namespace
{
	// If you change these values, don't forget to rebuild ref_results from a trusted source
	enum
	{
		min_dimension = -10,
		max_dimension =  10,

		min_rank = -10,
		max_rank =  10,
	};

	static_assert(min_dimension <= max_dimension, "minimum value is greater than maximum!");
	static_assert(min_rank <= max_rank, "minimum value is greater than maximum!");
}

#if defined( GENERATE_TABLE )
namespace
{
	// Genrated from Maple 7
	int ref_results[max_dimension - min_dimension + 1][max_rank - min_rank + 1] = {
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, -10, 55, -220, 715, -2002, 5005, -11440, 24310, -48620, 92378},
		{-9, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, -9, 45, -165, 495, -1287, 3003, -6435, 12870, -24310, 43758},
		{36, -8, 1, 0, 0, 0, 0, 0, 0, 0, 1, -8, 36, -120, 330, -792, 1716, -3432, 6435, -11440, 19448},
		{-84, 28, -7, 1, 0, 0, 0, 0, 0, 0, 1, -7, 28, -84, 210, -462, 924, -1716, 3003, -5005, 8008},
		{126, -56, 21, -6, 1, 0, 0, 0, 0, 0, 1, -6, 21, -56, 126, -252, 462, -792, 1287, -2002, 3003},
		{-126, 70, -35, 15, -5, 1, 0, 0, 0, 0, 1, -5, 15, -35, 70, -126, 210, -330, 495, -715, 1001},
		{84, -56, 35, -20, 10, -4, 1, 0, 0, 0, 1, -4, 10, -20, 35, -56, 84, -120, 165, -220, 286},
		{-36, 28, -21, 15, -10, 6, -3, 1, 0, 0, 1, -3, 6, -10, 15, -21, 28, -36, 45, -55, 66},
		{9, -8, 7, -6, 5, -4, 3, -2, 1, 0, 1, -2, 3, -4, 5, -6, 7, -8, 9, -10, 11},
		{-1, 1, -1, 1, -1, 1, -1, 1, -1, 1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 3, 1, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 4, 6, 4, 1, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 5, 10, 10, 5, 1, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 6, 15, 20, 15, 6, 1, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 7, 21, 35, 35, 21, 7, 1, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 8, 28, 56, 70, 56, 28, 8, 1, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 9, 36, 84, 126, 126, 84, 36, 9, 1, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 10, 45, 120, 210, 252, 210, 120, 45, 10, 1},
	};
}

#include <fstream>
#include <sstream>
#include <string>

class binomial_table_generator
{
	binomial_table_generator()
	{
		std::fstream table_file(TEST_TABLE_FILE, std::ios_base::out);
		if (table_file)
		{
			write_header(table_file);

			for (int dimension_size = min_dimension; dimension_size <= max_dimension; ++dimension_size)
			{
				for (int rank_size = min_rank; rank_size <= max_rank; ++rank_size)
				{
					write_coefficient(table_file, dimension_size, rank_size);
				}
			}
		}
	}
	void write_header(std::ostream& out)
	{
		out << "//////////////////////////////////////////////////////////////////////\n"\
		       "// This file is auto-generated from\n"\
			   "// " __BASE_FILE__ "\n"\
			   "//////////////////////////////////////////////////////////////////////\n"\
		       "template<int dimension_size, int rank_size> struct binomial_coefficient_ref_table;\n"
		    << std::endl;
	}
	void write_coefficient(std::ostream& out, int dimension_size, int rank_size)
	{
		out << "template<> struct binomial_coefficient_ref_table<"
			<< dimension_size << ", " << rank_size << ">"\
		       "{ enum { value = " << ref_results[dimension_size - min_dimension][rank_size - min_rank] << " }; };"
		    << std::endl;
	}

	static binomial_table_generator instance;
};

binomial_table_generator binomial_table_generator::instance;

#elif defined( TEST_BINOMIAL_COEFFICIENTS )

#include TEST_TABLE_FILE

template<int dimension_size, int rank_size>
struct binomial_coefficient_check
{
	enum
	{
		value     = binomial_coefficient<dimension_size, rank_size>::value,
		ref_value = binomial_coefficient_ref_table<dimension_size, rank_size>::value,
	};

	binomial_coefficient_check()
	{
		static_assert(value == ref_value, "Incorrect binomial coefficient value in " __FUNCTION__);
	}
};

template<int min_dimension, int max_dimension, int min_rank, int max_rank, int dimension_size = min_dimension, int rank_size = min_rank>
struct binomial_coefficient_test_helper
{
	enum
	{
		next_dimension = (rank_size < max_rank) ? dimension_size : dimension_size + 1,
		next_rank      = (rank_size < max_rank) ? rank_size + 1  : min_rank,
	};

	binomial_coefficient_test_helper<min_dimension, max_dimension, min_rank, max_rank, next_dimension, next_rank> next;
	binomial_coefficient_check<dimension_size, rank_size> check;
};

template<int min_dimension, int max_dimension, int min_rank, int max_rank>
struct binomial_coefficient_test_helper<min_dimension, max_dimension, min_rank, max_rank, max_dimension, max_rank>
{
	enum
	{
		dimension_size = max_dimension,
		rank_size      = max_dimension,
	};
	binomial_coefficient_check<dimension_size, rank_size> check;
};

static binomial_coefficient_test_helper<min_dimension, max_dimension, min_rank, max_rank> unit_test;

#endif