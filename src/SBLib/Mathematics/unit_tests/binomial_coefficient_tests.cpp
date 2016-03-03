#include <Mathematics/binomial_coefficient.h>

#define DEFINE_DEFAULT_VARIABLE( X ) decltype(X) X
#define DEFINE_VARIABLE( X ) decltype(X) X = decltype(X)

#include <functional>
template<long long value, long long ref_value, typename caller_type>
struct are_equal
{
	are_equal()
	{
		static_assert(value == ref_value, __FUNCTION__);
	}
};

#if !defined( NO_STATIC_UNIT_TESTS ) || defined( UNIT_TESTS )
#define TEST_BINOMIAL_COEFFICIENTS
#endif

#if defined( GENERATE_UNIT_TESTS )
#if defined( GENERATE_ERROR_UNIT_TESTS )
#define GENERATE_RANDOM_ERROR
#endif
#define GENERATE_TABLE
#endif

#define TEST_TABLE_FILE   "Mathematics/unit_tests/binomial_coefficient_ref_table.h"
#define MAPLE_OUTPUT_FILE "Mathematics/unit_tests/binomial_coefficient_maple_output.inc"

namespace
{
	// If you change these values, don't forget to rebuild ref_results from a trusted source
	enum
	{
		min_ref_dimension = -20,
		max_ref_dimension = +20,

		min_ref_rank = -20,
		max_ref_rank = +20,
	};
}

#if defined( GENERATE_TABLE )

namespace
{
	// Generated from Maple 7  with source code :
	/*
		min_dimension := -20:  max_dimension := +20:
		min_rank      := -20:  max_rank      := +20:
		for dimension from min_dimension to max_dimension do
		  print(
			op(map(rank->cat(binomial(dimension, rank), 'LL'), [$(min_rank..max_rank)])), ``
		  );
		od:
	*/
	long long ref_results[max_ref_dimension - min_ref_dimension + 1][max_ref_rank - min_ref_rank + 1] = {
		#include MAPLE_OUTPUT_FILE
	};
	are_equal<sizeof(ref_results), (max_ref_dimension - min_ref_dimension + 1) * (max_ref_rank - min_ref_rank + 1) * sizeof(decltype(ref_results[0][0])), decltype(ref_results)> size_check;
}

#include <fstream>
#include <sstream>
#include <string>
#include <assert.h>

#if defined( GENERATE_RANDOM_ERROR )
#include <random>
#endif

class binomial_table_generator
{
	binomial_table_generator()
#if defined( GENERATE_RANDOM_ERROR )
		:
			dim_error( dim_dist(generator) ),
			rank_error( rank_dist(generator) )
#endif
	{
		std::fstream table_file(TEST_TABLE_FILE, std::ios_base::out);
		if (table_file)
		{
			write_header(table_file);

			for (int dimension_size = min_ref_dimension; dimension_size <= max_ref_dimension; ++dimension_size)
				for (int rank_size = min_ref_rank; rank_size <= max_ref_rank; ++rank_size)
					write_coefficient(table_file, dimension_size, rank_size);
		}
	}
	void write_header(std::ostream& out)
	{
		out << "#pragma once\n"\
		       "//////////////////////////////////////////////////////////////////////\n"\
		       "// This file is auto-generated from\n"\
		       "// " __BASE_FILE__ "\n"\
		       "//////////////////////////////////////////////////////////////////////\n"\
		       "template<int dimension_size, int rank_size> struct binomial_coefficient_ref_table;\n"
		    << std::endl;
	}
	void write_coefficient(std::ostream& out, int dimension_size, int rank_size)
	{
#if defined( GENERATE_RANDOM_ERROR )
		const bool is_error  = ( dimension_size == dim_error && rank_size == rank_error );
		auto ref_value = ref_results[dimension_size - min_ref_dimension][rank_size - min_ref_rank];
		if (is_error)
		{
			std::uniform_int_distribution<decltype(ref_value)> value_dist = std::uniform_int_distribution<decltype(ref_value)>(-abs(ref_value) - 1, abs(ref_value) + 1);
			auto value = decltype(ref_value)(0);
			while( !(value = value_dist(generator)) );
			ref_value += value;
		}
#else
		const auto ref_value = ref_results[dimension_size - min_ref_dimension][rank_size - min_ref_rank];
#endif
		assert(dimension_size - min_ref_dimension >= 0 && rank_size - min_ref_rank >= 0);
		out << "template<> struct binomial_coefficient_ref_table<"
		    << dimension_size << ", " << rank_size << "> "\
		       "{ enum : long long { value = " << ref_value << "LL }; };"
#if defined( GENERATE_RANDOM_ERROR )
			<< ( is_error ? " // this line should trigger an error" : "" )
#endif
			<< std::endl;
	}

	static binomial_table_generator instance;
#if defined( GENERATE_RANDOM_ERROR )
	static std::random_device generator;
	static std::uniform_int_distribution<int> dim_dist;
	static std::uniform_int_distribution<int> rank_dist;

	const int dim_error;
	const int rank_error;
#endif
};
#if defined( GENERATE_RANDOM_ERROR )
DEFINE_DEFAULT_VARIABLE(binomial_table_generator::generator);
DEFINE_VARIABLE(binomial_table_generator::dim_dist)(min_ref_dimension, max_ref_dimension);
DEFINE_VARIABLE(binomial_table_generator::rank_dist)(min_ref_rank, max_ref_rank);
#endif

DEFINE_DEFAULT_VARIABLE(binomial_table_generator::instance);

#elif defined( TEST_BINOMIAL_COEFFICIENTS )

#include TEST_TABLE_FILE

template<int dimension_size, int rank_size>
struct binomial_coefficient_check
{
	enum : long long
	{
		value     = binomial_coefficient<dimension_size, rank_size>::value,
		ref_value = binomial_coefficient_ref_table<dimension_size, rank_size>::value,
	};

	binomial_coefficient_check()
	{
		are_equal<value, ref_value, binomial_coefficient_check> validate;
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

namespace
{
	// Range is split so that the compiler will accept template instanciation
	enum
	{
		min_dimension0 = -20,
		max_dimension0 = -1,
		min_dimension1 =  0,
		max_dimension1 = +20,

		min_rank0 = -20,
		max_rank0 = -1,
		min_rank1 =  0,
		max_rank1 = +20,
	};
	static_assert(min_ref_dimension <= min_dimension0 && max_ref_dimension >= max_dimension0 && min_ref_rank <= min_rank0 && max_ref_rank >= max_rank0, "Value has not been generated");
	static_assert(min_ref_dimension <= min_dimension1 && max_ref_dimension >= max_dimension1 && min_ref_rank <= min_rank1 && max_ref_rank >= max_rank1, "Value has not been generated");
	static_assert(min_dimension0 <= max_dimension0 && min_rank0 <= max_rank0, "minimum value is greater than maximum!");
	static_assert(min_dimension1 <= max_dimension1 && min_rank1 <= max_rank1, "minimum value is greater than maximum!");
}

static binomial_coefficient_test_helper<min_dimension0, max_dimension0, min_rank0, max_rank0> unit_test00;
static binomial_coefficient_test_helper<min_dimension0, max_dimension0, min_rank1, max_rank1> unit_test01;
static binomial_coefficient_test_helper<min_dimension1, max_dimension1, min_rank0, max_rank0> unit_test10;
static binomial_coefficient_test_helper<min_dimension1, max_dimension1, min_rank1, max_rank1> unit_test11;
are_equal<sizeof(unit_test00), (max_dimension0 - min_dimension0 + 1) * (max_rank0 - min_rank0 + 1) * sizeof(char), decltype(unit_test00)> size_check00;

#endif