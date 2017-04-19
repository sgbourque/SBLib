#pragma once
#include <Mathematics/canonical_components.h>
#include <Mathematics/combinations.h>
namespace SBLib::Mathematics
{
//
// graded_multivector_t
//
template< typename T > struct get_type_traits;

template<typename scalar_t, size_t space_mask, size_t rank_size> struct graded_multivector_t;
template<typename scalar_t, size_t space_mask, size_t rank_size>
struct get_type_traits< graded_multivector_t<scalar_t, space_mask, rank_size> >
{
	using reference_type       = scalar_t&;
	using const_reference_type = const scalar_t&;
};
template<typename scalar_t, size_t space_mask>
struct get_type_traits< graded_multivector_t<scalar_t, space_mask, 0> >
{
	using reference_type       = scalar_t;
	using const_reference_type = scalar_t;
};

template<typename scalar_t, size_t space_mask, size_t rank_size>
struct graded_multivector_t
{
private:
	using traits = select_combinations<space_mask, rank_size>;

	//template<size_t subspace_mask>
	//struct get_traits_helper
	//{
	//	using reference_type       = scalar_t&;
	//	using const_reference_type = const scalar_t&;
	//};
	//template<>
	//struct get_traits_helper<0>
	//{
	//	using reference_type       = scalar_t;
	//	using const_reference_type = scalar_t;
	//};
	template<size_t subspace_mask>
	struct get_traits
	{
		enum : size_t { mask = (bit_count(subspace_mask) == rank_size) ? (subspace_mask & space_mask) : 0 };
		using reference_type       = typename get_type_traits< graded_multivector_t<scalar_t, mask, rank_size> >::reference_type;
		using const_reference_type = typename get_type_traits< graded_multivector_t<scalar_t, mask, rank_size> >::const_reference_type;
	};
	//template<size_t subspace_mask/*, typename = std::enable_if<subspace_mask != 0>*/>
	//constexpr auto get_helper() -> typename get_traits<subspace_mask>::reference_type
	//{
	//	return components[get_components_index<subspace_mask>( traits() )];
	//}
	//template</*size_t subspace_mask, typename std::enable_if_t<subspace_mask == 0>*/>
	//constexpr auto get_helper<0>() -> typename get_traits<0>::reference_type
	//{
	//	return scalar_t(0);
	//}
	//template<size_t subspace_mask>
	//constexpr auto get_helper() const -> typename get_traits<subspace_mask>::const_reference_type
	//{
	//	return components[get_components_index<subspace_mask>( traits() )];
	//}
	//template</*size_t subspace_mask, typename std::enable_if_t<subspace_mask == 0>*/>
	//constexpr auto get_helper<0>() const -> typename get_traits<0>::const_reference_type
	//{
	//	return scalar_t(0);
	//}

	template<size_t value, size_t loop>
	struct component_assign_helper
	{
		template<typename scalar_dest, size_t space_mask_dest, typename scalar_src, size_t space_mask_src>
		component_assign_helper(graded_multivector_t<scalar_dest, space_mask_dest, rank_size>& u, const graded_multivector_t<scalar_src, space_mask_src, rank_size>& v)
		{
			static_assert((value & space_mask) != 0, "Cannot assign value to orthogonal components of a multivector.");
			u.template get<value>() = static_cast<scalar_dest>(v.template get<value>());
		}
	};

public:
	enum : size_t
	{
		space_mask     = space_mask,
		dimension_size = traits::count,
		rank_size      = rank_size,
	};
	using components_type = canonical_components_t<scalar_t, dimension_size>;
	using scalar_type     = typename canonical_components_t<scalar_t, dimension_size>::scalar_type;

	enum eUNINITIALIZED : bool { UNINITIALIZED = true, };
	graded_multivector_t(eUNINITIALIZED) : components(components_type::UNINITIALIZED) {};

	graded_multivector_t() : components() {};
	graded_multivector_t(const graded_multivector_t& v) : components(v.components) {};
	explicit graded_multivector_t(const components_type& v) : components(v) {};
	explicit graded_multivector_t(components_type&& v) : components(v) {};

	template<typename... scalars>
	explicit graded_multivector_t(scalars&&... coords) : components{ std::forward<scalars>(coords)... } {}

	template<typename alt_scalar_t, size_t alt_space_mask, size_t rank_size>
	graded_multivector_t(const graded_multivector_t<alt_scalar_t, alt_space_mask, rank_size>& v) : components()
	{
		for_each_combination<select_combinations<alt_space_mask, rank_size>>::template iterate<graded_multivector_t::component_assign_helper>(*this, v);
	};

	template<typename alt_scalar_t, size_t alt_space_mask>
	graded_multivector_t<scalar_t, alt_space_mask, rank_size> project() const
	{
		graded_multivector_t<alt_scalar_t, alt_space_mask, rank_size> v;
		for_each_combination<select_combinations<(alt_space_mask & space_mask), rank_size>>::template iterate<graded_multivector_t::component_assign_helper>(v, *this);
		return std::move(v);
	};
	template<typename alt_vector_t>
	alt_vector_t project() const
	{
		return project<alt_vector_t::scalar_type, alt_vector_t::space_mask>();
	};

	//template<size_t subspace_mask>
	//constexpr auto get() -> typename get_traits<subspace_mask>::reference_type
	//{
	//	return get_helper<get_traits<subspace_mask>::mask>();
	//}
	//template<size_t subspace_mask>
	//constexpr auto get() const
	//{
	//	return get_helper<get_traits<subspace_mask>::mask>();
	//}
	//template<size_t subspace_mask>
	//constexpr auto cget() const
	//{
	//	return get<subspace_mask>();
	//}

	components_type components;
};


template<typename scalar_t, size_t space_mask, size_t rank_size>
inline const auto& operator *=(graded_multivector_t<scalar_t, space_mask, rank_size>& u, const scalar_t& scale)
{
	return u.components *= scale;
}
template<typename scalar_t, size_t space_mask, size_t rank_size>
inline auto operator *(const graded_multivector_t<scalar_t, space_mask, rank_size>& u, const scalar_t& scale)
{
	return graded_multivector_t<scalar_t, space_mask, rank_size>(std::move(u.components * scale));
}
template<typename scalar_t, size_t space_mask, size_t rank_size>
inline auto operator *(const scalar_t& scale, const graded_multivector_t<scalar_t, space_mask, rank_size>& v)
{
	return std::move(v * scale);
}
template<typename scalar_t, size_t space_mask, size_t rank_size>
inline const auto& operator /=(graded_multivector_t<scalar_t, space_mask, rank_size>& u, const scalar_t& scale)
{
	return u.components /= scale;
}
template<typename scalar_t, size_t space_mask, size_t rank_size>
inline auto operator /(const graded_multivector_t<scalar_t, space_mask, rank_size>& u, const scalar_t& scale)
{
	return graded_multivector_t<scalar_t, space_mask, rank_size>(std::move(u.components / scale));
}
template<typename scalar_t, size_t space_mask, size_t rank_size>
inline const auto& operator +=(graded_multivector_t<scalar_t, space_mask, rank_size>& u, graded_multivector_t<scalar_t, space_mask, rank_size>& v)
{
	return u.components += v.components;
}
template<typename scalar_t, size_t space_mask, size_t rank_size>
inline auto operator +(const graded_multivector_t<scalar_t, space_mask, rank_size>& u, const graded_multivector_t<scalar_t, space_mask, rank_size>& v)
{
	return graded_multivector_t<scalar_t, space_mask, rank_size>(std::move(u.components + v.components));
}

//
// vector_t specialization
//
template<typename scalar_t, size_t space_mask>
using vector_t = graded_multivector_t<scalar_t, space_mask, 1>;


//
// scalar specialization
//
template<typename scalar_t, size_t space_mask>
struct graded_multivector_t<scalar_t, space_mask, 0>
{
private:
	//template<size_t subspace_mask/*, typename = std::enable_if<subspace_mask != 0>*/>
	//struct get_traits_helper
	//{
	//	using reference_type       = scalar_t;
	//	using const_reference_type = scalar_t;
	//};
	//template</*size_t subspace_mask, typename std::enable_if_t<subspace_mask == 0>*/>
	//struct get_traits_helper<0>
	//{
	//	using reference_type       = scalar_t&;
	//	using const_reference_type = const scalar_t&;
	//};
	//template<size_t subspace_mask>
	//struct get_traits
	//{
	//	enum : size_t { mask = (subspace_mask == 0) ? subspace_mask : ~0u, };
	//	using reference_type       = typename get_traits_helper<mask>::reference_type;
	//	using const_reference_type = typename get_traits_helper<mask>::const_reference_type;
	//};

	//template<size_t subspace_mask>
	//constexpr auto get_helper() -> typename get_traits<subspace_mask>::reference_type
	//{
	//	return scalar_t(0);
	//}
	//template</*size_t subspace_mask, typename std::enable_if_t<subspace_mask == 0>*/>
	//constexpr auto get_helper<0>() -> typename get_traits<0>::reference_type
	//{
	//	return components[0];
	//}
	//template<size_t subspace_mask>
	//constexpr auto get_helper() const -> typename get_traits<subspace_mask>::const_reference_type
	//{
	//	return scalar_t(0);
	//}
	//template</*size_t subspace_mask, typename std::enable_if_t<subspace_mask == 0>*/>
	//constexpr auto get_helper<0>() const -> typename get_traits<0>::const_reference_type
	//{
	//	return components[0];
	//}

public:
	enum : size_t
	{
		space_mask     = space_mask,
		dimension_size = 1,
		rank_size      = 0,
	};
	using components_type = canonical_components_t<scalar_t, dimension_size>;
	using scalar_type     = typename canonical_components_t<scalar_t, dimension_size>::scalar_type;

	enum eUNINITIALIZED : bool { UNINITIALIZED = true, };
	graded_multivector_t(eUNINITIALIZED) : components(components_type::UNINITIALIZED) {};

	graded_multivector_t() : components() {};
	graded_multivector_t(const graded_multivector_t& v) : components(v.components) {};
	explicit graded_multivector_t(const components_type& v) : components(v) {};
	explicit graded_multivector_t(components_type&& v) : components(v) {};

	template<typename... scalars>
	explicit graded_multivector_t(scalars&&... coords) : components{ std::forward<scalars>(coords)... } {}

	//template<typename alt_scalar_t, size_t alt_space_mask, size_t rank_size>
	//graded_multivector_t(const graded_multivector_t<alt_scalar_t, alt_space_mask, rank_size>& v) : components()
	//{
	//	for_each_combination<select_combinations<alt_space_mask, 0>>::template iterate<graded_multivector_t::component_assign_helper>(*this, v);
	//};

	template<typename alt_scalar_t, size_t alt_space_mask>
	graded_multivector_t<scalar_t, alt_space_mask, rank_size> project() const
	{
		graded_multivector_t<alt_scalar_t, alt_space_mask, rank_size> v;
		v.components = components;
		return std::move(v);
	};
	template<typename alt_vector_t>
	alt_vector_t project() const
	{
		return project<alt_vector_t::scalar_type, alt_vector_t::space_mask>();
	};


	//template<size_t subspace_mask>
	//constexpr auto get() -> typename get_traits<subspace_mask>::reference_type
	//{
	//	return get_helper<get_traits<subspace_mask>::mask>();
	//}
	//template<size_t subspace_mask>
	//constexpr auto get() const
	//{
	//	return get_helper<get_traits<subspace_mask>::mask>();
	//}
	//template<size_t subspace_mask>
	//constexpr auto cget() const
	//{
	//	return get<subspace_mask>();
	//}

	components_type components;
};
} // namespace SBLib::Mathematics
namespace SBLib { using namespace SBLib::Mathematics; }
