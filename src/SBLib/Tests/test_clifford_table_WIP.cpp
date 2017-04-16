#include <test_common.h>
//#include <DirectXMath.h>

#include <Mathematics/multivector.h>
#include <stdint.h>

#if _MSC_VER != 1910
#error "Need MS Visual Studio 2017"
#endif // #if _MSC_VER != 1910
#if _MSVC_LANG < 201403L
#error "Need to set c++ standard to c++-latest."
#endif // #if _MSVC_LANG != 201403L

#define USING(X)  	( (2 - X) / X == X )
#define IN_USE    	(+1)
#define NOT_IN_USE	(-1)

#define ARCH_AVX2 ( __AVX2__ ? IN_USE : NOT_IN_USE )
#define ARCH_AVX  ( __AVX__  ? IN_USE : NOT_IN_USE )
#define ARCH_SSE  ( (_M_IX86_FP > 0) || (_M_X64 > 0) ? IN_USE : NOT_IN_USE )

#if USING( ARCH_SSE )
#include <intrin.h>
#endif

namespace SBLib::Mathematics
{
// any packed_scalars_traits::packed_type should map to some low-level register
// e.g.,
// -	For half precision, packed_type could be a single 16-bits short or 32-bits int packing 2 halfs.
// -	For single precision, we will usually use faster SSE operations so we pack floats into 128-bits.
// -	If we need more than one packed scalar at a times, it may still maps to a register so use it (e.g., 512-bits AVX2 registers which may hold up to 16 floats).
template<typename scalar_t, size_t count = 0>
struct packed_scalars_traits
{
	using scalar_type = scalar_t;
	using packed_type = typename packed_scalars_traits<scalar_type, count - 1>::packed_type;
	enum : size_t
	{
		scalar_count = count,
		packed_size  = sizeof(packed_type) / sizeof(scalar_type),
		packed_count = (scalar_count + (packed_size - 1)) / packed_size,
	};
	using container_type = std::array<packed_type, packed_count>;
	using init_container = container_type;
};
template<typename scalar_t>
struct packed_scalars_traits<scalar_t, 0>
{
	using scalar_type = scalar_t;
	using packed_type = scalar_type;
	enum : size_t
	{
		scalar_count = 1,
		packed_size  = sizeof(packed_type) / sizeof(scalar_type),
		packed_count = (scalar_count + (packed_size - 1)) / packed_size,
	};
	using container_type = std::array<packed_type, packed_count>;
	using init_container = container_type;
};

// packed_scalars are essentially containers to be seen as a representation state over some module.
template<typename scalar_t, size_t count>
struct packed_scalars
{
	using scalar_type    = scalar_t;
	using traits         = packed_scalars_traits<scalar_type, count>;
	using packed_type    = typename traits::packed_type;
	using container_type = typename traits::container_type;

	enum eUNINITIALIZED : bool { UNINITIALIZED = true, };
	packed_scalars(eUNINITIALIZED) {};

	packed_scalars() : value{} {}
	packed_scalars( const packed_scalars& ) = default;
	packed_scalars& operator =( const packed_scalars& ) = default;
	packed_scalars( packed_scalars&& ) = default;
	packed_scalars& operator =( packed_scalars&& ) = default;

	explicit packed_scalars( const container_type& _value ) : value(_value) {};
	explicit packed_scalars( container_type&& _value ) : value(_value) {};

	template< typename first_type, typename... init_type, typename = std::enable_if_t< !std::conjunction_v< std::is_same<first_type, init_type>... > > >
	explicit packed_scalars(first_type, init_type...)
	{
		static_assert( std::conjunction_v< std::is_same<first_type, init_type>... >, "Invalid initializers." );
	}
	template< typename... init_type, typename = std::enable_if_t< std::conjunction_v< std::is_same<scalar_type, init_type>... > > >
	explicit packed_scalars( scalar_type first_value, init_type... _value ) : value{ std::move( typename traits::init_container{ first_value, std::forward<init_type>(_value)... } ) }
	{
		static_assert( sizeof...(init_type) < count, "Too many initializers" );
	}
	template< typename... init_type, typename = std::enable_if_t< std::conjunction_v< std::is_same<packed_type, init_type>... > > >
	explicit packed_scalars( std::enable_if_t< !std::is_same_v<scalar_type, packed_type>, packed_type > first_pack, init_type... _packed_value ) : value{ std::move( typename traits::init_container{ first_pack, std::forward<init_type>(_packed_value)... } ) }
	{
		static_assert( sizeof...(init_type) < traits::packed_count, "Too many initializers");
	}

	constexpr packed_type& get(size_t pack_index) { return value[pack_index]; }
	constexpr scalar_type& operator [](size_t scalar_index)
	{
		scalar_type* scalar_ptr = reinterpret_cast<scalar_type*>(&value[scalar_index]);
		return scalar_ptr[scalar_index];
	}
	constexpr scalar_type get(size_t pack_index) const { return value[pack_index]; }
	constexpr scalar_type operator [](size_t scalar_index) const
	{
		const scalar_type* scalar_ptr = reinterpret_cast<const scalar_type*>(&value[scalar_index]);
		return *scalar_ptr;
	}

	container_type value;
};
template<typename scalar_t, size_t count>
auto begin( const packed_scalars<scalar_t, count>& _packed )
{
	return _packed.value.begin();
}
template<typename scalar_t, size_t count>
auto begin( packed_scalars<scalar_t, count>&& _packed )
{
	return _packed.value.begin();
}
template<typename scalar_t, size_t count>
auto begin( packed_scalars<scalar_t, count>& _packed)
{
	return _packed.value.begin();
}
template<typename scalar_t, size_t count>
auto end( const packed_scalars<scalar_t, count>& _packed )
{
	return _packed.value.end();
}

template< typename type >
struct print_traits;

template< typename scalar_t, size_t count >
struct print_traits< packed_scalars<scalar_t, count> >
{
	constexpr static const char* separator( intptr_t distance ) { return distance != 0 ? " " : ""; }
};

template<typename scalar_t, size_t count>
auto& operator >>(std::istream& in, packed_scalars<scalar_t, count>& _packed)
{
	using packed_traits = typename packed_scalars<scalar_t, count>::traits;
	for (	scalar_t* s = reinterpret_cast<scalar_t*>( &*begin(_packed) );
			s != reinterpret_cast<scalar_t*>( &*begin(_packed) ) + count;
			++s )
		in >> *s;
	for (	scalar_t* s = reinterpret_cast<scalar_t*>( &*begin(_packed) ) + count;
			s != reinterpret_cast<scalar_t*>(&*begin(_packed)) + (packed_traits::packed_size * packed_traits::packed_count);
			++s )
		*s = scalar_t(0);
	return in;
}
template<typename scalar_t, size_t count>
auto& operator <<(std::ostream& out, packed_scalars<scalar_t, count>& _packed)
{
	using print_traits = print_traits< packed_scalars<scalar_t, count> >;
	auto const first = reinterpret_cast<scalar_t* const>( &*begin(_packed) );
	for ( scalar_t* s = first; s != first + count; ++s )
		out << print_traits::separator( s - first ) << *s;
	return out;
}

template<typename scalar_t, size_t count0, size_t count1>
auto direct_sum(const packed_scalars<scalar_t, count0>& _packed0, const packed_scalars<scalar_t, count1>& _packed1)
{
	using packed0_type       = typename packed_scalars<scalar_t, count0>::packed_type;
	using packed0_traits     = typename packed_scalars<scalar_t, count0>::traits;
	using packed1_type       = typename packed_scalars<scalar_t, count1>::packed_type;
	using packed1_traits     = typename packed_scalars<scalar_t, count1>::traits;
	using packed_iterator_type = std::conditional_t< sizeof(packed0_type) < sizeof(packed1_type), packed0_type, packed1_type >;

	using result_type        = packed_scalars<scalar_t, count0 + count1>;
	using result_packed_type = typename result_type::packed_type;
	using result_traits      = typename result_type::traits;

	result_type result(result_type::UNINITIALIZED);
	scalar_t* result_iterator = reinterpret_cast<scalar_t*>( &*begin(result) );
	for (	const scalar_t*       unpacked_iterator     = reinterpret_cast<const scalar_t*>( &*begin(_packed0) ),
			              * const unpacked_end_iterator = reinterpret_cast<const scalar_t*>( &*begin(_packed0) ) + count0;
			unpacked_iterator != unpacked_end_iterator;
			++result_iterator, ++unpacked_iterator )
		*result_iterator = *unpacked_iterator;
	for (	const scalar_t*       unpacked_iterator     = reinterpret_cast<const scalar_t*>( &*begin(_packed1) ),
			              * const unpacked_end_iterator = reinterpret_cast<const scalar_t*>( &*begin(_packed1) ) + count1;
			unpacked_iterator != unpacked_end_iterator;
			++result_iterator, ++unpacked_iterator )
		*result_iterator = *unpacked_iterator;
	for (	const scalar_t* const unpacked_end_iterator = reinterpret_cast<const scalar_t*>( &*begin(result) ) + (result_traits::packed_size * result_traits::packed_count);
			result_iterator != unpacked_end_iterator;
			++result_iterator )
		*result_iterator = scalar_t(0);
	return std::move( result );
}
template< size_t extract_count, size_t offset, typename scalar_t, size_t count >
auto extract( const packed_scalars<scalar_t, count>& _packed )
{
	static_assert( offset + extract_count <= count, "Invalid scalars extraction" );
	using result_type        = packed_scalars<scalar_t, extract_count>;
	using result_packed_type = typename result_type::packed_type;
	using result_traits      = typename result_type::traits;

	result_type result(result_type::UNINITIALIZED);
	scalar_t* result_iterator = reinterpret_cast<scalar_t*>( &*begin(result) );
	for (	const scalar_t*      unpacked_iterator     = reinterpret_cast<const scalar_t*>( &*begin(_packed) ),
			              *const unpacked_end_iterator = reinterpret_cast<const scalar_t*>( &*begin(_packed) ) + extract_count;
			unpacked_iterator != unpacked_end_iterator;
			++result_iterator, ++unpacked_iterator )
		*result_iterator = *unpacked_iterator;
	for (	const scalar_t* const unpacked_end_iterator = reinterpret_cast<const scalar_t*>( &*begin(result) ) + (result_traits::packed_size * result_traits::packed_count);
			result_iterator != unpacked_end_iterator;
			++result_iterator )
		*result_iterator = scalar_t(0);
	return std::move( result );
}


#if USING( ARCH_SSE )
template<>
struct packed_scalars_traits<float, 1>
{
	using scalar_type = float;
	using packed_type = __m128;
	enum : size_t
	{
		scalar_count = 1,
		packed_size  = sizeof(packed_type) / sizeof(scalar_type),
		packed_count = (scalar_count + (packed_size - 1)) / packed_size,
	};
	using container_type = std::array<packed_type, packed_count>;
	using init_container = container_type;
};
#endif
#if USING( ARCH_AVX )
template<>
struct packed_scalars_traits<float, 8>
{
	using scalar_type = float;
	using packed_type = __m256;
	enum : size_t
	{
		scalar_count = 8,
		packed_size  = sizeof(packed_type) / sizeof(scalar_type),
		packed_count = (scalar_count + (packed_size - 1)) / packed_size,
	};
	using container_type = std::array<packed_type, packed_count>;
	using init_container = container_type;
};
#endif
#if USING( ARCH_AVX2 )
#error "Untested..."
template<>
struct packed_scalars_traits<float, 16>
{
	using scalar_type = float;
	using packed_type = __m512;
	enum : size_t
	{
		scalar_count = 16,
		packed_size  = sizeof(packed_type) / sizeof(scalar_type),
		packed_count = (scalar_count + (packed_size - 1)) / packed_size,
	};
	using container_type = std::array<packed_type, packed_count>;
	using init_container = container_type;
};
#endif // #if USING( ARCH_AVX2 )




#if 0
template<uint8_t  mask> struct packed_clifford_subspace_mask : clifford_space_mask<mask> {};
template<uint64_t mask, uint8_t submask>
struct clifford_packed_subspace_map
{
	using space_mask_type    = clifford_space_mask<mask>;
	using subspace_mask_type = packed_clifford_subspace_mask<submask>;
};


// float specialization
//
// - Choose 4x32-bits representation : optimize for projective 3-space w/ SSE.
// - Most generic case (in more than 4 dimensions) clifford product will return fill a 512-bits AVX2 register.
template<>

template< typename scalar_t, size_t size_v >
struct multivector_traits
{
	using scalar_pack_type = typename multivector_traits<scalar_t, size_v - 1>::scalar_pack_type;
};
template<>
struct multivector_traits<float, 16>
{
	using scalar_pack_type = __m512;
};
template<>
struct multivector_traits<float, 8>
{
	using scalar_pack_type = __m256;
};
template<>
struct multivector_traits<float, 1>
{
	using scalar_pack_type = __m128;
};
template<>
struct multivector_traits<float, 0>
{
	// scalar_pack_type : SFINAE
};

//
// multivector
//
// - scalar_t typically is a representation of a real or complex field with associative abelian operations +/* (generally float).
// - subspace is a list of std::integral_constant
template<size_t spacemask_v> struct spacemask_constant : std::integral_constant<size_t, spacemask_v> {};
template<size_t spacemask_v> struct grade_constant     : std::integral_constant<long long, spacemask_v> {};
template< typename scalar_t, typename... subspace > class multivector;

template< typename scalar_t, size_t spacemask_v, size_t... grade_v>
class multivector<scalar_t, spacemask_constant<spacemask_v>, grade_constant<grade_v>...>
{
public:
	using scalar_type = scalar_t;

	scalar_type& operator[](size_t index) { return reinterpret_cast<scalar_t*>(&components[index / pack_size()])[index % pack_size()]; }
	constexpr static size_t size() { return add_binomials<grade_v...>::value; }
private:
	enum : size_t { space_dimension = bit_count(spacemask_v), };

	template<size_t... grade> struct add_binomials;
	template<size_t first_grade, size_t... grade> struct add_binomials<first_grade, grade...>
		: std::integral_constant<size_t, binomial_coefficient<space_dimension, first_grade>::value + add_binomials<grade...>::value > {};
	template<> struct add_binomials<> : std::integral_constant<size_t, 0> {};

	using pack_type = typename multivector_traits<scalar_t, multivector::size()>::scalar_pack_type;
	constexpr static size_t pack_size()  { return sizeof(pack_type) / sizeof(scalar_t); }
	constexpr static size_t pack_count() { return (multivector::size() + multivector::pack_size() - 1) / multivector::pack_size(); }
	std::array<pack_type, multivector::pack_count()> components;
};
#endif // 0
} // namespace SBLib::Mathematics

// Pin(2):
// (e, x,y, J) * (f, u,v, K) = (e*e + x*u + y*v - J*K, e*u + x*f - y*K + J*v, e*v + x*K + y*f - J*u, e*K + x*v - y*u +J*f)

template<typename scalar_t, size_t count>
auto clifford_sum(const SBLib::packed_scalars<scalar_t, count>& a, const SBLib::packed_scalars<scalar_t, count>& b)
{
	using namespace SBLib;
	using packed_scalars_type = packed_scalars<scalar_t, count>;
	using half_packed_scalars_type = packed_scalars<scalar_t, count / 2>;
	const half_packed_scalars_type* a0 = reinterpret_cast<const half_packed_scalars_type*>(&*begin(a));
	const half_packed_scalars_type* a1 = reinterpret_cast<const half_packed_scalars_type*>( reinterpret_cast<const scalar_t*>(&*begin(a)) + count / 2 );
	const half_packed_scalars_type* b0 = reinterpret_cast<const half_packed_scalars_type*>(&*begin(b));
	const half_packed_scalars_type* b1 = reinterpret_cast<const half_packed_scalars_type*>(reinterpret_cast<const scalar_t*>(&*begin(b)) + count / 2);
	return direct_sum(clifford_sum(*a0, *b0), clifford_product(*a1, *b1));
}
auto clifford_sum(const SBLib::packed_scalars<float, 1>& a, const SBLib::packed_scalars<float, 1>& b)
{
	return SBLib::packed_scalars<float, 1>{ a[0] + b[0] };
}

template<typename scalar_t, size_t count>
auto clifford_product(const SBLib::packed_scalars<scalar_t, count>& a, const SBLib::packed_scalars<scalar_t, count>& b)
{
	using namespace SBLib;
	using packed_scalars_type = packed_scalars<scalar_t, count>;
	using half_packed_scalars_type = packed_scalars<scalar_t, count / 2>;
	const half_packed_scalars_type* a0 = reinterpret_cast<const half_packed_scalars_type*>( &*begin(a) );
	const half_packed_scalars_type* a1 = reinterpret_cast<const half_packed_scalars_type*>( reinterpret_cast<const scalar_t*>(&*begin(a)) + count / 2 );
	const half_packed_scalars_type* b0 = reinterpret_cast<const half_packed_scalars_type*>( &*begin(b) );
	const half_packed_scalars_type* b1 = reinterpret_cast<const half_packed_scalars_type*>( reinterpret_cast<const scalar_t*>(&*begin(b)) + count / 2 );
	return direct_sum( clifford_sum(clifford_product(*a0, *b0), clifford_product(*a1, *b1)), clifford_sum(clifford_product(*a0, *b1), clifford_product(*a1, *b0)) );
}
auto clifford_product(const SBLib::packed_scalars<float, 1>& a, const SBLib::packed_scalars<float, 1>& b)
{
	return SBLib::packed_scalars<float, 1>{ a[0]*b[0] };
}

#include <Traits/clifford_traits.h>
namespace SBLib::Traits
{
template<size_t first, size_t second, bool big_endian = default_basis_big_endian>
struct clifford_product_traits
{
	enum : size_t
	{
		inner_bit_set = (first & second),
		first_outer_bit_set  = (first  & ~inner_bit_set),
		second_outer_bit_set = (second & ~inner_bit_set),
	};
	enum : int
	{
		first_sign  = alternating_traits<first_outer_bit_set, inner_bit_set, big_endian>::sign
		              * reversion_conjugacy_traits<inner_bit_set, big_endian>::sign,
		second_sign = alternating_traits<inner_bit_set, second_outer_bit_set, big_endian>::sign,
	};
public:
	enum : int
	{
		sign = first_sign * second_sign * alternating_traits<first_outer_bit_set, second_outer_bit_set, big_endian>::sign,
	};
	enum : size_t
	{
		bit_set = (first ^ second),
	};
	static_assert( sign * sign == 1, "Invalid clifford product sign." );
};
}

#include <algorithm>
namespace SBLib::Test
{
class test_clifford_table_WIP : RegisteredFunctor
{
	template<size_t dim, size_t offset0 = 0, size_t offset1 = offset0>
	static void print_table( std::ostream& out, int setw, std::function< std::string(size_t, size_t) > bin_op )
	{
		out << std::setw(setw) << "^" << " | ";
		for (size_t j = offset1; j < offset1 + dim; ++j)
			out << std::setw(setw) << j << " ";
		out << std::endl;

		out << std::setw(setw) << std::setfill('-') << "-" << "--";
		for (size_t j = offset1; j < offset1 + dim; ++j)
			out << std::setw(setw) << std::setfill('-') << "-" << "-";
		out << std::setw(0) << std::setfill(' ') << std::endl;

		for (size_t i = 0; i < dim; ++i)
		{
			out << std::setw(setw) << offset0 + i << " | ";
			for (size_t j = 0; j < dim; ++j)
				out << std::setw(setw) << bin_op(i, j) << " ";
			out << std::endl;
		}
		out << std::endl;
	}

	template<size_t dim_count, size_t offset0, size_t offset1>
	struct build_clifford_table
	{
		template<size_t value, size_t loop>
		struct build
		{
		private:
			template<size_t value_internal, size_t loop_internal>
			struct build_internal
			{
				build_internal(int sign_table[dim_count])
				{
					sign_table[value_internal] = clifford_product_traits<value + offset0, value_internal + offset1>::sign;
				}
			};
		public:
			build(int sign_table[dim_count][dim_count])
			{
				static_for_each<0, dim_count>::iterate<build_internal>(sign_table[value]);
			}
		};
	};
	template<size_t clifford_dim, size_t bloc_size, size_t dim_offset = 0>
	struct print_clifford_table
	{
		template<size_t offset, size_t loop>
		struct print
		{
		private:
			template<size_t offset_internal, size_t loop>
			struct print_internal
			{
				print_internal( std::ostream& out )
				{
					static int table[bloc_size][bloc_size];
					static_for_each<0, bloc_size>::iterate< build_clifford_table<bloc_size, dim_offset + offset, dim_offset + offset_internal>::build >(table);
					print_table<bloc_size, dim_offset + offset, dim_offset + offset_internal>( out, 2, [](size_t i, size_t j) -> auto
						{
							std::stringstream ss;
							//ss << (table[i][j] < 0 ? "-" : "") << ((i + dim_offset + offset)^(j + dim_offset + offset_internal));
							ss << (table[i][j] < 0 ? "-" : "+");
							return ss.str();
						}
					);
				}
			};
		public:
			print(std::ostream& out)
			{
				static_for_each<0, offset + bloc_size, increment_index_helper<bloc_size>>::iterate< print_internal >(std::cout);
				out << std::endl;
			}
		};
	};

	test_clifford_table_WIP() : RegisteredFunctor(__FUNCTION__, fct) {}
	static void fct()
	{
		enum : size_t { dim = 7, clifford_dim = (1 << dim), bloc_size = (1 << std::min<size_t>(5, dim)), };
		static_assert( bloc_size <= clifford_dim, "invalid bloc_size" );
		static_for_each<0, clifford_dim, increment_index_helper<bloc_size>>::iterate< print_clifford_table<clifford_dim, bloc_size>::print >( std::cout );


		//using packed_scalars = packed_scalars<float, 3>;
		//packed_scalars a( packed_scalars::UNINITIALIZED );
		//std::cin >> a;
		//auto a2 = direct_sum(a, a);
		//auto a3 = direct_sum(a, a2);
		//auto a4 = direct_sum(a, a3);
		//auto a2_3 = extract<6, 3>(a4);
		//std::cout << a2_3;
		//test_mask test;
		//std::cout << std::endl << test.values[127] << std::endl;

		//using clifford_scalars = packed_scalars<float, 4>;
		//clifford_scalars a{ 1.0f, 0.0f, 0.0f, 1.0f }, b{ 1.0f, 0.0f, 0.0f, 1.0f };
		//auto c = clifford_product(a, b);
		//std::cout << c;

		//std::cout << std::endl;
		//multivector<float, spacemask_constant<7>, grade_constant<0>> scalar{};
		//for (size_t index = 0; index < scalar.size(); ++index)
		//	std::cout << scalar[index] << " ";

		//std::cout << std::endl;
		//multivector<float, spacemask_constant<7>, grade_constant<1>> vector{};
		//for (size_t index = 0; index < vector.size(); ++index)
		//	std::cout << vector[index] << " ";

		//std::cout << std::endl;
		//multivector<float, spacemask_constant<7>, grade_constant<0>, grade_constant<2>> quaternion{};
		//for (size_t index = 0; index < quaternion.size(); ++index)
		//	std::cout << quaternion[index] << " ";

		//std::cout << std::endl;
		//multivector<float, spacemask_constant<7>, grade_constant<0>, grade_constant<1>, grade_constant<2>, grade_constant<3>> multivec{};
		//for (size_t index = 0; index < multivec.size(); ++index)
		//	std::cout << multivec[index] << " ";

		//std::cout << std::endl;
		//multivector<float, spacemask_constant<7>, grade_constant<1>, grade_constant<1>> dual_quaternion{};
		//for (size_t index = 0; index < quaternion.size(); ++index)
		//	std::cout << dual_quaternion[index] << " ";

		//std::cout << std::endl;
		//multivector<float, spacemask_constant<7>, grade_constant<2>> pseudo_vector{};
		//for (size_t index = 0; index < pseudo_vector.size(); ++index)
		//	std::cout << pseudo_vector[index] << " ";

		//std::cout << std::endl;
		//multivector<float, spacemask_constant<7>, grade_constant<3>> pseudo_scalar{};
		//for (size_t index = 0; index < pseudo_scalar.size(); ++index)
		//	std::cout << pseudo_scalar[index] << " ";
	}
	static test_clifford_table_WIP instance;
};
test_clifford_table_WIP test_clifford_table_WIP::instance;
}
