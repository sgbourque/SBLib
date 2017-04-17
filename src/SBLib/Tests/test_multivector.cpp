#define USE_MULTIVECTOR 1
#include <test_common.h>
#include <Traits/clifford_traits.h>
#include <Mathematics/multivector.h>

#include <fstream>

#include <intrin.h> // for SSE / AVX

#define USE_DIRECTX_MATH _MSC_EXTENSIONS
#if USE_DIRECTX_MATH
#include <DirectXMath.h>
#endif

#ifdef USE_CURRENT_TEST
#undef USE_CURRENT_TEST
#endif
#define USE_CURRENT_TEST 1

namespace SBLib::Mathematics
{
//
// Wedge product
//
template<size_t subspace_mask, size_t loop>
struct wedge_product_helper
{
private:
	template<size_t subspace_mask2, size_t loop2>
	struct wedge_product_internal
	{
		template<typename scalar_t>
		struct assign
		{
			template<int sign> static constexpr void alternate_multiply(scalar_t, const scalar_t&, const scalar_t&) {}; // nothing to do
		};
		template<typename scalar_t>
		struct assign<scalar_t&>
		{
			template<int sign> static constexpr void alternate_multiply    (scalar_t& result, const scalar_t& u, const scalar_t& v); // should not be called
			template<>         static constexpr void alternate_multiply<+1>(scalar_t& result, const scalar_t& u, const scalar_t& v) { result += u * v; }
			template<>         static constexpr void alternate_multiply<-1>(scalar_t& result, const scalar_t& u, const scalar_t& v) { result -= u * v; }
		};

	public:
		template<typename scalar_t, size_t space_mask0, size_t space_mask2, size_t rank_size0, size_t rank_size2>
		wedge_product_internal(graded_multivector_t<scalar_t, space_mask0, rank_size0>& result, const scalar_t& u, const graded_multivector_t<scalar_t, space_mask2, rank_size2>& v)
		{
			using traits = SBLib::alternating_traits<subspace_mask, subspace_mask2>;
			using ref_type = decltype( result.get<(subspace_mask ^ subspace_mask2)>() );
			assign<ref_type>::alternate_multiply<traits::sign>(result.get<(subspace_mask ^ subspace_mask2)>(), u, v.get<subspace_mask2>());
		}
	};

public:
	template<typename scalar_t, size_t space_mask0, size_t space_mask1, size_t space_mask2, size_t rank_size0, size_t rank_size1, size_t rank_size2>
	wedge_product_helper(graded_multivector_t<scalar_t, space_mask0, rank_size0>& result, const graded_multivector_t<scalar_t, space_mask1, rank_size1>& u, const graded_multivector_t<scalar_t, space_mask2, rank_size2>& v)
	{
		SBLib::for_each_combination< SBLib::select_combinations<space_mask2, rank_size2> >::iterate<wedge_product_internal>(result, u.get<subspace_mask>(), v);
	}
};
//
// Generic version
//
template<typename scalar_t, size_t space_mask1, size_t space_mask2, size_t rank_size1, size_t rank_size2>
auto wedge_product(const graded_multivector_t<scalar_t, space_mask1, rank_size1>& u, const graded_multivector_t<scalar_t, space_mask2, rank_size2>& v)
{
	using multivec_t = graded_multivector_t<scalar_t, (space_mask1 | space_mask2), (rank_size1 + rank_size2)>;
	multivec_t result;
	SBLib::for_each_combination< SBLib::select_combinations<space_mask1, rank_size1> >::iterate<wedge_product_helper>(result, u, v);
	return std::move(result);
}

#if USE_DIRECTX_VECTOR_HACK
//
// HACK ALERT : temporary test code for optimization check.
// Once optimized, triple hodge on dot product cause about 44% performance loss (13 instructions instead of 9), which is bad.
// This needs to be fixed.
//
// Also, hodge isn't even defined yet so using it here is ackward...
//
template<size_t space_mask>
auto wedge_product(const vector_t<float, space_mask>& u, const graded_multivector_t<float, space_mask, vector_t<float, space_mask>::dimension_size - 1>& v)
{
	static_assert(vector_t<float, space_mask>::dimension_size == 3, "These hacks are only working in 3 dimensions.");
	using multivec_t = graded_multivector_t<float, space_mask, vector_t<float, space_mask>::dimension_size>;
	multivec_t result(multivec_t::UNINITIALIZED);
	result.components.container[0] = DirectX::XMVector3Dot(u.components.container[0], (*v).components.container[0]);
	for (size_t batch = 1; batch != u.components.container.size(); ++batch)
		result.components.container[0] = DirectX::XMVectorAdd(result.components.container[0], DirectX::XMVector3Dot(u.components.container[batch], (*v).components.container[batch]));
	return std::move(result);
}
//
// HACK ALERT : Only valid on 3 dimension (test only)
//
template<size_t space_mask>
auto wedge_product(const vector_t<float, space_mask>& u, const vector_t<float, space_mask>& v)
{
	static_assert(vector_t<float, space_mask>::dimension_size == 3, "These hacks are only working in 3 dimensions.");
	using multivec_t = graded_multivector_t<float, space_mask, 1>;
	multivec_t result(multivec_t::UNINITIALIZED);
	result.components.container[0] = DirectX::XMVector3Cross(u.components.container[0], v.components.container[0]);
	return std::move(*result);
}
#endif // #if USE_DIRECTX_VECTOR
template<typename scalar_t, size_t space_mask1, size_t space_mask2, size_t rank_size1, size_t rank_size2>
auto operator ^(const graded_multivector_t<scalar_t, space_mask1, rank_size1>& u, const graded_multivector_t<scalar_t, space_mask2, rank_size2>& v)
{
	return std::move(wedge_product(u, v));
}


//
// Hodge dual
//
template<size_t subspace_mask, size_t loop>
struct hodge_conjugate_helper
{
private:
	using scalar_t = float;
	template<typename scalar_t>
	struct assign
	{
		template<int sign> static constexpr void conjugate(scalar_t, const scalar_t&) {}; // result is not in destination space : nothing to do
	};
	template<typename scalar_t>
	struct assign<scalar_t&>
	{
		template<int sign> static constexpr void conjugate(scalar_t& result, const scalar_t& u); // should not be called ever
		template<>         static constexpr void conjugate<+1>(scalar_t& result, const scalar_t& u) { result = +u; }
		template<>         static constexpr void conjugate<-1>(scalar_t& result, const scalar_t& u) { result = -u; }
	};

public:
	template<typename scalar_t, size_t space_mask0, size_t space_mask1, size_t rank_size0, size_t rank_size1>
	hodge_conjugate_helper(graded_multivector_t<scalar_t, space_mask0, rank_size0>& result, const graded_multivector_t<scalar_t, space_mask1, rank_size1>& u)
	{
		using traits = SBLib::hodge_conjugacy_traits<subspace_mask, space_mask0>;
		using ref_type = decltype(result.get<traits::bit_set>());
		static_assert((subspace_mask & space_mask0) == subspace_mask, "Cannot calculate Hodge dual over non-embedding space. Please project onto target space first.");
		static_assert((subspace_mask ^ traits::bit_set) == space_mask0, "Incorrect dual space.");
		assign<ref_type>::conjugate<traits::sign>(result.get<traits::bit_set>(), u.get<subspace_mask>());
	}
};
//
// Generic version
//
template<typename scalar_t, size_t space_mask, size_t rank_size>
auto hodge_conjugate(const graded_multivector_t<scalar_t, space_mask, rank_size>& u)
{
	using multivec_t = graded_multivector_t<scalar_t, space_mask, vector_t<scalar_t, space_mask>::dimension_size - rank_size>;
	multivec_t result(multivec_t::UNINITIALIZED);
	SBLib::for_each_combination< SBLib::select_combinations<space_mask, rank_size> >::iterate<hodge_conjugate_helper>(result, u);
	return std::move(result);
}

#if USE_DIRECTX_VECTOR_HACK
//
// HACK ALERT : temporary test code for optimization check. This won't work at all in generic cases (it works in dim 3)
// Once optimized, double hodge on dot product cause about 44% performance loss (13 instructions instead of 9), which is bad.
// This needs to be fixed.
//
static const __m128i maskNeg2 = DirectX::XMVECTORU32{ 0x00000000u, 0x80000000u, 0x00000000u, 0x00000000u };
template<size_t space_mask, size_t rank_size>
auto hodge_conjugate(const graded_multivector_t<float, space_mask, rank_size>& u)
{
	static_assert(vector_t<float, space_mask>::dimension_size == 3, "These hacks are only working in 3 dimensions.");
	using multivec_t = graded_multivector_t<float, space_mask, SBLib::bit_count(space_mask) - rank_size>;
	multivec_t result(multivec_t::UNINITIALIZED);
	result.components.container[0] = _mm_castsi128_ps(_mm_xor_si128(_mm_castps_si128(u.components.container[0]), maskNeg2));
	return std::move(result);
}
#endif // USE_DIRECTX_VECTOR
template<typename scalar_t, size_t space_mask, size_t rank_size>
auto operator *(const graded_multivector_t<scalar_t, space_mask, rank_size>& u)
{
	return std::move(hodge_conjugate(u));
}


template<typename scalar_t, size_t space_mask1, size_t space_mask2, size_t rank_size1, size_t rank_size2>
auto CrossProduct(const graded_multivector_t<scalar_t, space_mask1, rank_size1>& u, const graded_multivector_t<scalar_t, space_mask2, rank_size2>& v)
{
	return std::move( *(u ^ v) );
}
template<typename scalar_t, size_t space_mask1, size_t space_mask2, size_t rank_size1, size_t rank_size2>
auto InnerProduct(const graded_multivector_t<scalar_t, space_mask1, rank_size1>& u, const graded_multivector_t<scalar_t, space_mask2, rank_size2>& v)
{
	return std::move( *(u ^ *v) );
}
} // namespace SBLib::Mathematics

//////////////////////////////////////////////////////////////////////////////

namespace SBLib::Test
{
class test_multivector : public RegisteredFunctor
{
	enum
	{
		e0  = (1 << 0),  e1  = (1 << 1),
		e2  = (1 << 2),  e3  = (1 << 3),
		e4  = (1 << 4),  e5  = (1 << 5),
		e6  = (1 << 6),  e7  = (1 << 7),
		e8  = (1 << 8),  e9  = (1 << 9),
		e10 = (1 << 10), e11 = (1 << 11),
		e12 = (1 << 12), e13 = (1 << 13),
		e14 = (1 << 14), e15 = (1 << 15),
	};
	using multivector_type1 = graded_multivector_t<float, e0 | e1 | e2, 1>;
	using multivector_type2 = graded_multivector_t<float, e0 | e1 | e2, 2>;
	using multivector_type3 = graded_multivector_t<float, e0 | e1 | e2, 3>;

	test_multivector() : RegisteredFunctor("test_multivector", fct) {}
	static void fct()
	{
		multivector_type1 test1{ cos(0.4f), -sin(0.4f), 0.0f };
		multivector_type1 test2{ sin(0.4f),  cos(0.4f), 0.0f };
		multivector_type1 test3{    0.0f,      0.0f,    1.0f };

		const std::string input_filename = "../../tmp/test_multivector.in";
		const bool onlytest = (std::cin.gcount() == 0);
		if (onlytest)
		{
			std::cout << "Press 'd' to delete file, press anything else to continue..." << std::endl;
		}
		int data = std::cin.get();
		if (data == 'd')
			std::remove(input_filename.c_str());
		else if (!onlytest)
			std::cin.unget();

		{
			auto file = std::fstream(input_filename, std::ios_base::in | std::ios_base::_Nocreate);

			static const std::string version_string = "version";
			static const size_t version = 0;
			if (file.is_open())
			{
				std::string file_version_string;
				size_t file_version;
				file >> file_version_string >> file_version;
				if (file_version_string != version_string || file_version != version)
					file.close();
			}
			else
			{
				std::cout << "Please enter 9 numbers : ";
			}

			std::istream& in = file.is_open() ? file : std::cin;

			in >> test1;
			in >> test2;
			in >> test3;

			if (!file.is_open())
			{
				file.open(input_filename, std::ios_base::out);
				if (file.is_open())
				{
					file << version_string << " " << version << std::endl;
					file << " " << test1.cget<e0>();
					file << " " << test1.cget<e1>();
					file << " " << test1.cget<e2>();
					file << " " << test2.cget<e0>();
					file << " " << test2.cget<e1>();
					file << " " << test2.cget<e2>();
					file << " " << test3.cget<e0>();
					file << " " << test3.cget<e1>();
					file << " " << test3.cget<e2>();
				}
			}
		}

#if USE_DIRECTX_VECTOR
		DirectX::XMVECTOR test1v{ test1.components.container[0] };
		DirectX::XMVECTOR test2v{ test2.components.container[0] };
#elif USE_DIRECTX_MATH // #if USE_DIRECTX_VECTOR
		DirectX::XMVECTOR test1v{ test1.components.container[0], test1.components.container[1], test1.components.container[2] };
		DirectX::XMVECTOR test2v{ test2.components.container[0], test2.components.container[1], test2.components.container[2] };
#endif // #if USE_DIRECTX_VECTOR

		auto test4 = (test1 ^ test2);
		std::cout << test1 << " ^ " << test2 << " = " << test4 << std::endl;

		auto test5 = CrossProduct(test1, test2);
		std::cout << test1 << " x " << test2 << " = " << test5 << std::endl;

#if USE_DIRECTX_MATH
		DirectX::XMVECTORF32 test5v;
		test5v.v = DirectX::XMVector3Cross(test1v, test2v);
		std::cout << test1 << " x " << test2 << " = " << test5v.f[0] << ", " << test5v.f[1] << ", " << test5v.f[2] << std::endl;
#endif

		auto test6 = InnerProduct(test1, test2);
		std::cout << test1 << " . " << test2 << " = " << test6 << std::endl;

#if USE_DIRECTX_MATH
		DirectX::XMVECTORF32 test6v;
		test6v.v = DirectX::XMVector3Dot(test1v, test2v);
		std::cout << test1 << " . " << test2 << " = " << test6v.f[0] << std::endl;
#endif

		auto test7 = (test1 ^ test2 ^ test3);
		std::cout << "Det{" << test1 << test2 << test3 << "} = " << *test7 << std::endl;

		std::cout << "... run test '" << instance.get_id() << "d' to delete input file..." << std::endl;
	}

	static test_multivector instance;
};
#if USE_CURRENT_TEST
test_multivector test_multivector::instance;
#endif // #if USE_CURRENT_TEST
} // namespace SBLib::Test
