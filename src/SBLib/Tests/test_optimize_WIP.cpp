#include <test_common.h>

#include <intrin.h>
//#include <xmmintrin.h>

#include <Algorithms/counter.h>

template< size_t counter_value = SBLib::get_counter() >
using next_token = SBLib::counter_token< counter_value >;

template< size_t counter_value = SBLib::get_counter() >
constexpr auto get_token() { return SBLib::counter_token< counter_value >(); }

//enum
//{
//	k0_glob = SBLib::get_counter(), g0_glob = __COUNTER__,
//	k1_glob = SBLib::get_counter(), g1_glob = __COUNTER__,
//	k2_glob = SBLib::get_counter(), g2_glob = __COUNTER__,
//	k3_glob = SBLib::get_counter(), g3_glob = __COUNTER__,
//	k4_glob = SBLib::get_counter(), g4_glob = __COUNTER__,
//	k5_glob = SBLib::get_counter(), g5_glob = __COUNTER__,
//	k6_glob = SBLib::get_counter(), g6_glob = __COUNTER__,
//	k7_glob = SBLib::get_counter(), g7_glob = __COUNTER__,
//};

#ifndef __has_builtin
#define __has_builtin(X) 0
#endif

template<unsigned int a, unsigned int b, unsigned int c, unsigned int d>
constexpr unsigned int shuffle_order = (a << 0) | (b << 2) | (c << 4) | (d << 6);

struct Scalar
{
	Scalar(float a) : components{ _mm_set1_ps(a) } {}
	Scalar(__m128 a) : components{ a } {}
	operator float() const { return _mm_cvtss_f32(components); }
	operator __m128() { return components; }
	__m128 components;
};
Scalar operator +(Scalar a, Scalar b) { return _mm_add_ps(a, b); }
struct Vector3
{
	Vector3(float x, float y, float z) : components{ _mm_set_ps(0.0f, z, y, x) } {}
	Vector3( __m128 v ) : components{v} {}
	operator __m128() { return components; }
	float operator []( unsigned int i ) const
	{
		union
		{
			float  m128_f32[4];
			__m128 m128;
		} cast;
		cast.m128 = components;
		return cast.m128_f32[i];
	}
	__m128 components;
};
Vector3 operator -(Vector3 a) { return _mm_xor_ps(a, Scalar(-0.0f)); }
Vector3 operator +(Vector3 a, Vector3 b) { return _mm_add_ps(a, b); }

struct Rotor3
{
	Rotor3(Vector3 _from, Vector3 _to) : from(_from), to(_to) {}
	Vector3 from;
	Vector3 to;
};

Scalar dot(Vector3 a, Vector3 b)
{
	auto xx_yy_zz_00       = _mm_mul_ps(a.components, b.components);
#if __has_builtin(__builtin_shufflevector)
	auto zz_00_xx_yy       = __builtin_shufflevector(xx_yy_zz_00, xx_yy_zz_00, 2,3,0,1);
#else
	auto zz_00_xx_yy       = _mm_shuffle_ps(xx_yy_zz_00, xx_yy_zz_00, shuffle_order<2,3,0,1>);
#endif
	auto xxPzz_yy_xxPzz_yy = _mm_add_ps(xx_yy_zz_00, zz_00_xx_yy);
#if __has_builtin(__builtin_shufflevector)
	auto yy_xxPzz_yy_xxPzz = __builtin_shufflevector(xxPzz_yy_xxPzz_yy, xx_yy_zz_00, 1, 0, 1, 0);
#else
	auto yy_xxPzz_yy_xxPzz = _mm_shuffle_ps(xxPzz_yy_xxPzz_yy, xx_yy_zz_00, shuffle_order<1, 0, 1, 0>);
#endif
	return _mm_add_ps(xxPzz_yy_xxPzz_yy, yy_xxPzz_yy_xxPzz);
}

Vector3 Reflect(Vector3 versor, Vector3 vector)
{
	Scalar TWO{ 2.0f };
	Scalar versor_norm = dot(versor, versor);
	Scalar  proj    = _mm_div_ps( dot(vector, versor), versor_norm );
	Vector3 reflect =_mm_mul_ps( _mm_mul_ps(proj, versor), TWO );
	return _mm_sub_ps( vector, reflect );
}
__declspec(noinline) Vector3 Rotate(Rotor3 r, Vector3 vector)
{
	Vector3 half = r.from + r.to;
	// what if half == 0?
	return Reflect(half, Reflect(r.from, vector) );
}


namespace SBLib::Test
{
class test_optimize_WIP : RegisteredFunctor
{
	//enum
	//{
	//	k0_test = get_counter(), g0_test = __COUNTER__,
	//	k1_test = get_counter(), g1_test = __COUNTER__,
	//	k2_test = get_counter(), g2_test = __COUNTER__,
	//};

	test_optimize_WIP() : RegisteredFunctor("test_optimize_WIP", fct) {}
	static void fct()
	{
		static const float pi = 3.141592654f;
		Vector3 vec { 1.0f, 2.0f, 3.0f };

		std::cout << "Enter angle : " << std::endl;
		float angle;
		std::cin >> angle;

		Rotor3  r{ Vector3{ 1.0f, 0.0f, 0.0f },
		           Vector3{ cos(angle * pi / 180.0f), sin(angle * pi / 180.0f), 0.0f } };

		auto res = Rotate(r, vec);
		std::cout << res[0] << ", " << res[1] << ", " << res[2];

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
		//static_assert( (c0 >= 0) && (c1 > c0) && (c2 > c1) && (c3 > c2), "counters are not counting correctly...");

		//using token0 = next_token<>;
		//using token1 = next_token<>;
		//auto dummy0 = token0();
		//auto dummy1 = token1();
		//std::cout
		//	<< typeid(dummy0).name() << std::endl
		//	<< typeid(dummy1).name() << std::endl;
		//static_assert( !std::is_same_v<token0, token1>, "Distinct tokens should not be of the same type!" );
	}
	static test_optimize_WIP instance;
};
test_optimize_WIP test_optimize_WIP::instance;
}
