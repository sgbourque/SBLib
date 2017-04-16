#include <test_common.h>
//#include <Traits/clifford_traits.h>

constexpr uint8_t  operator "" _8u (unsigned long long int value) { return static_cast<uint8_t>(value); }
constexpr uint16_t operator "" _16u(unsigned long long int value) { return static_cast<uint16_t>(value); }
constexpr uint32_t operator "" _32u(unsigned long long int value) { return static_cast<uint32_t>(value); }
constexpr uint64_t operator "" _64u(unsigned long long int value) { return static_cast<uint64_t>(value); }

constexpr int8_t  operator "" _8i (unsigned long long int value) { return static_cast<int8_t>(value); }
constexpr int16_t operator "" _16i(unsigned long long int value) { return static_cast<int16_t>(value); }
constexpr int32_t operator "" _32i(unsigned long long int value) { return static_cast<int32_t>(value); }
constexpr int64_t operator "" _64i(unsigned long long int value) { return static_cast<int64_t>(value); }

constexpr int64_t test_min     =  0x8000000000000000_64i;
constexpr int64_t test_neg_min = -0x8000000000000000_64i;
static_assert( test_min     == -std::numeric_limits<int64_t>::max() - 1, "" );
static_assert( test_neg_min == +(-std::numeric_limits<int64_t>::max() - 1), "" );
static_assert( test_neg_min == -(-std::numeric_limits<int64_t>::max() - 1), "" );
static_assert( test_min     == -test_neg_min, "" );

namespace SBLib::Traits
{
template<size_t mask>
struct bit_mask : std::integral_constant<size_t, mask> {};

template<size_t mask, size_t current_mask, size_t current_bit>
struct bit_iterator
{
	template< typename = std::enable_if_t< (current_mask | current_bit) != 0 > >
	constexpr auto operator ++() const { return bit_iterator<mask, (current_mask & ~current_bit), current_mask & ~((current_mask & ~current_bit) - 1)>(); }
	template< typename = std::enable_if_t< (current_mask | current_bit) != 0 > >
	constexpr auto operator ->() const { return reinterpret_cast< bit_mask<current_bit>* >(nullptr); }
	template< typename = std::enable_if_t< (current_mask | current_bit) != 0 > >
	constexpr auto operator *() const  { return *(this->operator->()); }
};
template<size_t mask>
constexpr auto begin( bit_mask<mask> )
{
	return ++bit_iterator<mask, mask, 0>();
}
template<size_t mask>
constexpr auto end(bit_mask<mask>)
{
	return bit_iterator<mask, 0, 0>();
}

static_assert( (*begin( bit_mask<5>{} )).value == 1, "invalid bit iterator" );
static_assert( begin( bit_mask<5>{} )->value == 1, "invalid bit iterator" );
static_assert( (++begin( bit_mask<5>{} ))->value == 4, "invalid bit iterator" );
static_assert( std::is_same_v< decltype( ++++begin( bit_mask<5>{} ) ), decltype( end( bit_mask<5>() ) ) >, "invalid bit iterator" );

template< typename begin_t, typename end_t, typename operation_t >
constexpr void for_each( begin_t begin, end_t end, operation_t op )
{
	op( *begin );
	for_each( ++begin, end, op );
}
template< typename end_t, typename operation_t > constexpr void for_each( end_t, end_t, operation_t ) {}

//#define disable_warning(X) __pragma( warning (disable: X) )
//#define reset_warnings() __pragma( warning (default) )
//#define push_disable_warning(X) __pragma( warning (push; disable: X) )
//#define push_warnings() __pragma( warning (push) )
//#define pop_warnings() __pragma( warning (pop) )

//template<class T> struct bit_count_traits;
//template<> struct bit_count_traits< uint8_t  > { constexpr size_t max_iteration = 2; };
//template<> struct bit_count_traits< uint16_t > { constexpr size_t max_iteration = 3; };
//template<> struct bit_count_traits< uint32_t > { constexpr size_t max_iteration = 4; };
//template<> struct bit_count_traits< uint64_t > { constexpr size_t max_iteration = 5; };

//template<class T>
//constexpr left_shift( T a, T b )

//template<size_t iteration, class T>
//constexpr auto bit_count_internal(T /*mask*/)
//{
//	static_assert( std::is_unsigned_v<T>, "bit mask must be unsigned" );
//	auto constexpr max_value     = std::numeric_limits<T>::max();
//	auto constexpr half_mask     = ((1 << (iteration + 1)) << 1) - 1;
//	static_assert(max_value / half_mask == 0x55, "test");
//	auto parallel_mask = ( max_value / half_mask );
//	return parallel_mask;
//}

template<class T> constexpr auto half_bits_mask(T iteration) { return (T(1) << (T(1) << iteration)) - T(1); }
template<class T> constexpr auto parallel_bits_mask(T iteration)
{
	static_assert(std::is_unsigned_v<T>, "bit mask must be unsigned");
	constexpr T all_bits = T(~T(0));
	return (half_bits_mask(iteration) * (all_bits / half_bits_mask(iteration + 1)));
}
template<class T>
constexpr auto parallel_bit_count(T mask, T iteration)
{
	const T parallel_mask = parallel_bits_mask(iteration);
	return ((mask >> (T(0) << iteration)) & parallel_mask) + ((mask >> (T(1) << iteration)) & parallel_mask);
}

template<class T>
constexpr auto bit_count(T mask)
{
	using type = size_t;//std::make_unsigned_t<T>;
	type bit_mask = mask;
	bit_mask = parallel_bit_count(bit_mask, type(0));
	bit_mask = parallel_bit_count(bit_mask, type(1));
	bit_mask = parallel_bit_count(bit_mask, type(2));
	bit_mask = parallel_bit_count(bit_mask, type(3));
	bit_mask = parallel_bit_count(bit_mask, type(4));
	bit_mask = parallel_bit_count(bit_mask, type(5));
	return bit_mask;
}

static_assert( bit_count(-0x1_8i) == 0x40, "invalid bit count" );
static_assert( bit_count(0x3_8i) == 2, "invalid bit count" );
static_assert( bit_count(0xF_8i) == 4, "invalid bit count" );
static_assert( bit_count(0xFF_16i) == 8, "invalid bit count" );
static_assert( bit_count(0xFFFF_16u) == 16, "invalid bit count" );
static_assert( bit_count(0xFFFF_32i) == 16, "invalid bit count" );
static_assert( bit_count(0xFFFFFFFF_32u) == 32, "invalid bit count" );
static_assert( bit_count(0xFFFFFFFFFFFFFFFF_64u) == 64, "invalid bit count" );


//template<size_t remaining_bit_mask, bool is_big_endian, size_t first_bit = 0, int cur_index = -1>
//struct bit_iterator : bit_mask<first_bit, is_big_endian>
//{
//private:
//	enum : size_t
//	{
//		next_remaining_bit_mask = ( (remaining_bit_mask - 1) & remaining_bit_mask ),
//		next_bit                = remaining_bit_mask & ~( (remaining_bit_mask - 1) & remaining_bit_mask ),
//	};
//	enum : int
//	{
//		current_index           = std::conditional_t<is_big_endian,
//		                                             std::integral_constant<int, cur_index>,
//		                                             std::integral_constant<int, bit_iterator<next_remaining_bit_mask, false, next_bit, 0>::index_value + 1>
//		                          >::value,
//		next_index              = std::conditional_t<is_big_endian,
//		                                             std::integral_constant<int, cur_index + 1>,
//		                                             std::integral_constant<int, current_index - 1>
//		                          >::value,
//	};
//public:
//	enum : int
//	{
//		index_value = current_index,
//	};
//
//	using next_type  = bit_iterator<next_remaining_bit_mask, is_big_endian, next_bit, next_index>;
//	constexpr auto operator ++() const { return next_type(); }
//
//	constexpr auto operator *() const { return bit_mask<first_bit, is_big_endian>; }
//};
//template<bool is_big_endian, size_t first_bit, int cur_index>
//struct bit_iterator<0, is_big_endian, first_bit, cur_index> : std::integral_constant<size_t, first_bit>
//{
//	enum : int
//	{
//		index_value = cur_index,
//	};
//	using next_type = end_iterator;
//	constexpr auto operator ++() { return next_type(); }
//
//	constexpr auto operator *() const { return value; }
//};
//template<bool is_big_endian, int cur_index>
//struct bit_iterator<0, is_big_endian, 0, cur_index>
//{
//};
//
//template<size_t mask, bool is_big_endian>
//constexpr auto begin( bit_mask<mask, is_big_endian> )
//{
//	return bit_iterator<mask, is_big_endian>::next_type();
//}
//template<size_t mask, bool is_big_endian>
//constexpr auto end( bit_mask<mask, is_big_endian> )
//{
//	return end_iterator();
//}
//
//static_assert( *begin( bit_mask<5>{} ) == 1, "invalid bit iterator" );
//static_assert( *++begin( bit_mask<5>{} ) == 4, "invalid bit iterator" );
//static_assert( std::is_same_v< decltype( ++++begin( bit_mask<5>{} ) ), end_iterator >, "invalid bit iterator" );
//
//
//static_assert( *begin( bit_mask<5, false>{} ) == 1, "invalid bit iterator" );
//static_assert( *++begin( bit_mask<5, false>{} ) == 4, "invalid bit iterator" );
//static_assert( std::is_same_v< decltype( ++++begin( bit_mask<5, false>{} ) ), end_iterator >, "invalid bit iterator" );
//
//static_assert( decltype( begin( bit_mask<5>{} ) )::index_value == 0, "invalid bit iterator" );
//static_assert( decltype( begin( bit_mask<5>{} ) )::next_type::index_value == 1, "invalid bit iterator" );
//
//static_assert( decltype( begin( bit_mask<5, false>{} ) )::value == 1, "invalid bit iterator" );
//static_assert( decltype( begin( bit_mask<5, false>{} ) )::next_type::value == 4, "invalid bit iterator" );
//static_assert( decltype( begin( bit_mask<5, false>{} ) )::index_value == 1, "invalid bit iterator" );
//static_assert( decltype( begin( bit_mask<5, false>{} ) )::next_type::index_value == 0, "invalid bit iterator" );
//
//static_assert( decltype( begin( bit_mask<21, false>{} ) )::value == 1, "invalid bit iterator" );
//static_assert( decltype( begin( bit_mask<21, false>{} ) )::next_type::value == 4, "invalid bit iterator" );
//static_assert( decltype( begin( bit_mask<21, false>{} ) )::next_type::next_type::value == 16, "invalid bit iterator" );
//static_assert( decltype( begin( bit_mask<21, false>{} ) )::index_value == 2, "invalid bit iterator" );
//static_assert( decltype( begin( bit_mask<21, false>{} ) )::next_type::index_value == 1, "invalid bit iterator" );
//static_assert( decltype( begin( bit_mask<21, false>{} ) )::next_type::next_type::index_value == 0, "invalid bit iterator" );
//
//
//template< typename begin, typename end, typename operand_type, typename = std::enable_if_t< !std::is_same_v<begin, end> > >
//constexpr auto static_product( begin, end /*end_value*/, operand_type operand )
//{
//	return get< begin::index_value >( operand );// * static_product( begin::next_type(), end_value, operand );
//}
//template< typename end, typename operand_type >
//constexpr auto static_product( end, end, operand_type operand)
//{
//	return static_cast< decltype( get<0>( operand ) ) >(1);
//}
//
//template<int index, size_t mask, bool is_big_endian>
//auto constexpr get( bit_mask<mask, is_big_endian> )
//{
//	return get<index - 1>( bit_mask<mask, is_big_endian> );
//}
//
//static_assert( static_product( begin( bit_mask<9>() ), end( bit_mask<9>() ), integers() ), "test" );
//
//template<size_t first_mask, size_t second_mask, template<bool> typename signature, bool is_big_endian = default_basis_big_endian>
//struct clifford_product_traits
//{
//private:
//	enum : size_t
//	{
//		inner_product_mask = (first_mask & second_mask),
//		first_outer_mask   = (first_mask & inner_product_mask),
//		second_outer_mask  = (second_mask & inner_product_mask),
//	};
//public:
//	//enum : int_t
//	//{
//	//	sign_value = static_product(begin(bit_mask<inner_product_mask>), end(bit_mask<inner_product_mask>), signature<is_big_endian>(bit_mask)),
//	//}
//	enum : size_t
//	{
//		result_mask = (first_mask ^ second_mask),
//	};
//};
//
} // namespace SBLib::Traits

namespace SBLib::Test
{
class test : RegisteredFunctor
{
	test() : RegisteredFunctor(__FUNCTION__, fct) {}
	static void fct()
	{
	}
	static test instance;
};
test test::instance;
}

int main()
{
	using namespace SBLib::Test;

	bool quit = false, help = false;
	size_t testCase = ~0u;
	do
	{
		if (RegisteredFunctor::size() > 1)
		{
			quit = help = false;
			testCase = ~0u;
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
