#pragma once
#include <Mathematics/canonical_components.h>
#include <Mathematics/combinations.h>
namespace SBLib::Mathematics
{
//
// multivector_t
//
template<typename scalar_t, size_t space_mask, size_t rank_size>
struct multivector_t
{
private:
	using traits = typename select_combinations<space_mask, rank_size>;

	template<size_t subspace_mask>
	struct get_traits_helper
	{
		using reference_type       = scalar_t&;
		using const_reference_type = const scalar_t&;
	};
	template<>
	struct get_traits_helper<0>
	{
		using reference_type       = scalar_t;
		using const_reference_type = scalar_t;
	};
	template<size_t subspace_mask>
	struct get_traits
	{
		enum : size_t { mask = (bit_traits<subspace_mask>::population_count == rank_size) ? (subspace_mask & space_mask) : 0 };
		using reference_type       = typename get_traits_helper<mask>::reference_type;
		using const_reference_type = typename get_traits_helper<mask>::const_reference_type;
	};
	template<size_t subspace_mask>
	constexpr auto get_helper() -> typename get_traits<subspace_mask>::reference_type
	{
		return components[traits::get_components_index<subspace_mask>()];
	}
	template<>
	constexpr auto get_helper<0>() -> typename get_traits<0>::reference_type
	{
		return scalar_t(0);
	}
	template<size_t subspace_mask>
	constexpr auto get_helper() const -> typename get_traits<subspace_mask>::const_reference_type
	{
		return components[traits::get_components_index<subspace_mask>()];
	}
	template<>
	constexpr auto get_helper<0>() const -> typename get_traits<0>::const_reference_type
	{
		return scalar_t(0);
	}

	template<size_t value, size_t loop>
	struct component_assign_helper
	{
		template<typename scalar_dest, size_t space_mask_dest, typename scalar_src, size_t space_mask_src>
		component_assign_helper(multivector_t<scalar_dest, space_mask_dest, rank_size>& u, const multivector_t<scalar_src, space_mask_src, rank_size>& v)
		{
			static_assert((value & space_mask) != 0, "Cannot assign value to orthogonal components of a multivector.");
			u.get<value>() = static_cast<scalar_dest>(v.get<value>());
		}
	};

public:
	enum : size_t
	{
		space_mask     = space_mask,
		dimension_size = traits::count,
		rank_size      = rank_size,
	};
	using components_type = typename canonical_components_t<scalar_t, dimension_size>;
	using scalar_type     = typename canonical_components_t<scalar_t, dimension_size>::scalar_type;

	enum eUNINITIALIZED : bool { UNINITIALIZED = true, };
	multivector_t(eUNINITIALIZED) : components(components_type::UNINITIALIZED) {};

	multivector_t() : components() {};
	multivector_t(const multivector_t& v) : components(v.components) {};
	explicit multivector_t(const components_type& v) : components(v) {};
	explicit multivector_t(components_type&& v) : components(v) {};

	template<typename... scalars>
	explicit multivector_t(scalars&&... coords) : components{ std::forward<scalars>(coords)... } {}

	template<typename alt_scalar_t, size_t alt_space_mask, size_t rank_size>
	multivector_t(const multivector_t<alt_scalar_t, alt_space_mask, rank_size>& v) : components()
	{
		for_each_combination<select_combinations<alt_space_mask, rank_size>>::iterate<component_assign_helper>(*this, v);
	};

	template<typename alt_scalar_t, size_t alt_space_mask>
	multivector_t<scalar_t, alt_space_mask, rank_size> project() const
	{
		multivector_t<alt_scalar_t, alt_space_mask, rank_size> v;
		for_each_combination<select_combinations<(alt_space_mask & space_mask), rank_size>>::iterate<component_assign_helper>(v, *this);
		return std::move(v);
	};
	template<typename alt_vector_t>
	alt_vector_t project() const
	{
		return project<alt_vector_t::scalar_type, alt_vector_t::space_mask>();
	};

	template<size_t subspace_mask>
	constexpr auto get() -> typename get_traits<subspace_mask>::reference_type
	{
		return get_helper<get_traits<subspace_mask>::mask>();
	}
	template<size_t subspace_mask>
	constexpr auto get() const
	{
		return get_helper<get_traits<subspace_mask>::mask>();
	}
	template<size_t subspace_mask>
	constexpr auto cget() const
	{
		return get<subspace_mask>();
	}

	components_type components;
};


template<typename scalar_t, size_t space_mask, size_t rank_size>
inline const auto& operator *=(multivector_t<scalar_t, space_mask, rank_size>& u, const scalar_t& scale)
{
	return u.components *= scale;
}
template<typename scalar_t, size_t space_mask, size_t rank_size>
inline auto operator *(const multivector_t<scalar_t, space_mask, rank_size>& u, const scalar_t& scale)
{
	return multivector_t<scalar_t, space_mask, rank_size>(std::move(u.components * scale));
}
template<typename scalar_t, size_t space_mask, size_t rank_size>
inline auto operator *(const scalar_t& scale, const multivector_t<scalar_t, space_mask, rank_size>& v)
{
	return std::move(v * scale);
}
template<typename scalar_t, size_t space_mask, size_t rank_size>
inline const auto& operator /=(multivector_t<scalar_t, space_mask, rank_size>& u, const scalar_t& scale)
{
	return u.components /= scale;
}
template<typename scalar_t, size_t space_mask, size_t rank_size>
inline auto operator /(const multivector_t<scalar_t, space_mask, rank_size>& u, const scalar_t& scale)
{
	return multivector_t<scalar_t, space_mask, rank_size>(std::move(v.components / scale));
}
template<typename scalar_t, size_t space_mask, size_t rank_size>
inline const auto& operator +=(multivector_t<scalar_t, space_mask, rank_size>& u, multivector_t<scalar_t, space_mask, rank_size>& v)
{
	return u.components += v.components;
}
template<typename scalar_t, size_t space_mask, size_t rank_size>
inline auto operator +(const multivector_t<scalar_t, space_mask, rank_size>& u, const multivector_t<scalar_t, space_mask, rank_size>& v)
{
	return multivector_t<scalar_t, space_mask, rank_size>(std::move(u.components + v.components));
}

//
// vector_t specialization
//
template<typename scalar_t, size_t space_mask>
using vector_t = multivector_t<scalar_t, space_mask, 1>;


//
// scalar specialization
//
template<typename scalar_t, size_t space_mask>
struct multivector_t<scalar_t, space_mask, 0>
{
private:
	template<size_t subspace_mask>
	struct get_traits_helper
	{
		using reference_type       = scalar_t;
		using const_reference_type = scalar_t;
	};
	template<>
	struct get_traits_helper<0>
	{
		using reference_type       = scalar_t&;
		using const_reference_type = const scalar_t&;
	};
	template<size_t subspace_mask>
	struct get_traits
	{
		enum : size_t { mask = (subspace_mask == 0) ? subspace_mask : ~0u, };
		using reference_type       = typename get_traits_helper<mask>::reference_type;
		using const_reference_type = typename get_traits_helper<mask>::const_reference_type;
	};

	template<size_t subspace_mask>
	constexpr auto get_helper() -> typename get_traits<subspace_mask>::reference_type
	{
		return scalar_t(0);
	}
	template<>
	constexpr auto get_helper<0>() -> typename get_traits<0>::reference_type
	{
		return components[0];
	}
	template<size_t subspace_mask>
	constexpr auto get_helper() const -> typename get_traits<subspace_mask>::const_reference_type
	{
		return scalar_t(0);
	}
	template<>
	constexpr auto get_helper<0>() const -> typename get_traits<0>::const_reference_type
	{
		return components[0];
	}

public:
	enum : size_t
	{
		space_mask     = space_mask,
		dimension_size = 1,
		rank_size      = 0,
	};
	using components_type = typename canonical_components_t<scalar_t, dimension_size>;
	using scalar_type     = typename canonical_components_t<scalar_t, dimension_size>::scalar_type;

	enum eUNINITIALIZED : bool { UNINITIALIZED = true, };
	multivector_t(eUNINITIALIZED) : components(components_type::UNINITIALIZED) {};

	multivector_t() : components() {};
	multivector_t(const multivector_t& v) : components(v.components) {};
	explicit multivector_t(const components_type& v) : components(v) {};
	explicit multivector_t(components_type&& v) : components(v) {};

	template<typename... scalars>
	explicit multivector_t(scalars&&... coords) : components{ std::forward<scalars>(coords)... } {}

	template<typename alt_scalar_t, size_t alt_space_mask, size_t rank_size>
	multivector_t(const multivector_t<alt_scalar_t, alt_space_mask, rank_size>& v) : components()
	{
		for_each_combination<select_combinations<alt_space_mask, 0>>::iterate<component_assign_helper>(*this, v);
	};

	template<typename alt_scalar_t, size_t alt_space_mask>
	multivector_t<scalar_t, alt_space_mask, rank_size> project() const
	{
		multivector_t<alt_scalar_t, alt_space_mask, rank_size> v;
		v.components = components;
		return std::move(v);
	};
	template<typename alt_vector_t>
	alt_vector_t project() const
	{
		return project<alt_vector_t::scalar_type, alt_vector_t::space_mask>();
	};


	template<size_t subspace_mask>
	constexpr auto get() -> typename get_traits<subspace_mask>::reference_type
	{
		return get_helper<get_traits<subspace_mask>::mask>();
	}
	template<size_t subspace_mask>
	constexpr auto get() const
	{
		return get_helper<get_traits<subspace_mask>::mask>();
	}
	template<size_t subspace_mask>
	constexpr auto cget() const
	{
		return get<subspace_mask>();
	}

	components_type components;
};
} // namespace SBLib::Mathematics
namespace SBLib { using namespace Mathematics; }
