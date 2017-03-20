#pragma once
#include <Mathematics/canonical_components.h>
#include <Traits/bit_traits.h>

//
// vector_t
//
template<typename scalar_t, size_t space_mask>
struct vector_t
{
private:
	using traits = bit_traits<space_mask>;

	template<size_t subspace_mask>
	struct get_traits_helper
	{
		using reference_type = scalar_t&;
		using const_reference_type = const scalar_t&;
	};
	template<>
	struct get_traits_helper<0>
	{
		using reference_type = scalar_t;
		using const_reference_type = scalar_t;
	};
	template<size_t subspace_mask>
	struct get_traits
	{
		static const size_t mask = (bit_traits<subspace_mask>::population_count == 1) ? (subspace_mask & space_mask) : 0;
		using reference_type = typename get_traits_helper<mask>::reference_type;
		using const_reference_type = typename get_traits_helper<mask>::const_reference_type;
	};

	template<size_t subspace_mask>
	constexpr auto get_helper() -> typename get_traits<subspace_mask>::reference_type
	{
		return components[traits::get_bit_index<subspace_mask>()];
	}
	template<>
	constexpr auto get_helper<0>() -> typename get_traits<0>::reference_type
	{
		return scalar_t(0);
	}
	template<size_t subspace_mask>
	constexpr auto get_helper() const -> typename get_traits<subspace_mask>::const_reference_type
	{
		return components[traits::get_bit_index<subspace_mask>()];
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
		component_assign_helper(vector_t<scalar_dest, space_mask_dest>& u, const vector_t<scalar_src, space_mask_src>& v)
		{
			static_assert((value & space_mask) != 0, "Cannot assign value to orthogonal components of a vector.");
			u.get<value>() = static_cast<scalar_dest>(v.get<value>());
		}
	};

public:
	enum
	{
		space_mask = space_mask,
		dimension_size = traits::population_count,
	};
	using components_type = typename canonical_components_t<scalar_t, dimension_size>;
	using scalar_type = typename canonical_components_t<scalar_t, dimension_size>::scalar_type;

	vector_t() : components() {};
	vector_t(const vector_t& v) : components(v.components) {};
	explicit vector_t(const components_type& v) : components(v) {};
	explicit vector_t(components_type&& v) : components(v) {};

	template<typename... scalars>
	explicit vector_t(scalars&&... coords) : components{ std::forward<scalars>(coords)... } {}

	template<typename alt_scalar_t, size_t alt_space_mask>
	vector_t(const vector_t<alt_scalar_t, alt_space_mask>& v) : components()
	{
		using alt_traits = bit_traits<alt_space_mask>;
		for_each_bit<alt_traits>::iterate<component_assign_helper>(*this, v);
	};

	template<typename alt_scalar_t, size_t alt_space_mask>
	vector_t<scalar_t, alt_space_mask> project() const
	{
		vector_t<alt_scalar_t, alt_space_mask> v;
		using alt_traits = bit_traits<(alt_space_mask & space_mask)>;
		for_each_bit<alt_traits>::iterate<component_assign_helper>(v, *this);
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


template<typename scalar_t, size_t space_mask>
const auto& operator *=(vector_t<scalar_t, space_mask>& u, const scalar_t& scale)
{
	return u.components *= scale;
}
template<typename scalar_t, size_t space_mask>
auto operator *(const vector_t<scalar_t, space_mask>& u, const scalar_t& scale)
{
	return vector_t<scalar_t, space_mask>(std::move(u.components * scale));
}
template<typename scalar_t, size_t space_mask>
auto operator *(const scalar_t& scale, const vector_t<scalar_t, space_mask>& v)
{
	return std::move(v * scale);
}
template<typename scalar_t, size_t space_mask>
const auto& operator /=(vector_t<scalar_t, space_mask>& u, const scalar_t& scale)
{
	return u.components /= scale;
}
template<typename scalar_t, size_t space_mask>
auto operator /(const vector_t<scalar_t, space_mask>& u, const scalar_t& scale)
{
	return vector_t<scalar_t, space_mask>(std::move(v.components / scale));
}
template<typename scalar_t, size_t space_mask>
const auto& operator +=(vector_t<scalar_t, space_mask>& u, vector_t<scalar_t, space_mask>& v)
{
	return u.components += v.components;
}
template<typename scalar_t, size_t space_mask>
auto operator +(const vector_t<scalar_t, space_mask>& u, const vector_t<scalar_t, space_mask>& v)
{
	return vector_t<scalar_t, space_mask>(std::move(u.components + v.components));
}
