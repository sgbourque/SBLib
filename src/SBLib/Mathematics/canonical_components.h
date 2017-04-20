#pragma once
#include <array>
#include <Algorithms/static_for_each.h>
namespace SBLib::Containers::Mathematics
{
//
// Generic fixed dimension
//
template<template<typename, size_t> class canonical_components_type, typename scalar_t, size_t dimension>
struct canonical_components_helper
{
public:
	enum
	{
		dimension_size = dimension,
		parallel_size  = 1,
		parallel_count = (dimension_size + (parallel_size - 1)) / parallel_size,
	};
	using scalar_type       = scalar_t;
	using container_type    = std::array<scalar_type, dimension_size>;
	using raw_type          = scalar_type[dimension_size];

	scalar_type& operator[](const size_t index) { return static_cast<this_type*>(this)->container[index]; }
	const scalar_type& operator[](const size_t index) const { return static_cast<const this_type*>(this)->container[index]; }

private:
	using this_type = canonical_components_type<scalar_t, dimension>;
};

//
// Generic canonical components
//
template<typename scalar_t, size_t dimension>
struct canonical_components_t : canonical_components_helper<canonical_components_t, scalar_t, dimension>
{
	using components_type = canonical_components_helper<SBLib::Containers::Mathematics::canonical_components_t, scalar_t, dimension>;
	using container_type = typename components_type::container_type;
	using raw_type = typename components_type::raw_type;
	using scalar_type = typename components_type::scalar_type;

	enum eUNINITIALIZED : bool { UNINITIALIZED = true, };
	canonical_components_t(eUNINITIALIZED) {};

	canonical_components_t() : container{} {};
	canonical_components_t(canonical_components_t&& v): container(std::move(v.container)) {};
	canonical_components_t(const canonical_components_t& v) : container(v.container) {};

	explicit canonical_components_t(const container_type& v) : container(v) {};
	//explicit canonical_components_t(const raw_type& v) { container.v = v; };
	template<typename... scalars> explicit canonical_components_t(scalar_type first, scalars... coords) : container{{ first, std::forward<scalars>(coords)... }} { static_assert(sizeof...(scalars) < components_type::dimension_size, "Too many initializers."); }

	canonical_components_t(container_type&& v) : container(v) {}
	//canonical_components_t(raw_type&& v) { container.v = std::move(v); }

	const canonical_components_t& operator =(canonical_components_t&& v) { container = std::move(v.container); return *this; };
	const canonical_components_t& operator =(const canonical_components_t& v) { container = v.container; return *this; };

	operator container_type&() { return container; }
	operator const container_type&() const { return container; }

	container_type container;
};
//template<typename scalar_t, size_t dimension>
//inline auto begin(canonical_components_t<scalar_t, dimension>& u)
//{
//	return u.container.begin();
//}
//template<typename scalar_t, size_t dimension>
//inline auto begin(const canonical_components_t<scalar_t, dimension>& u)
//{
//	return u.container.begin();
//}
//template<typename scalar_t, size_t dimension>
//inline auto end(const canonical_components_t<scalar_t, dimension>& u)
//{
//	return u.container.end();
//}

template<size_t index, size_t loop>
struct multiply_component_helper
{
	template<typename scalar_t, size_t dimension>
	multiply_component_helper(canonical_components_t<scalar_t, dimension>& result, const canonical_components_t<scalar_t, dimension>& u, const scalar_t& scale)
	{
		result.container[index] = u.container[index] * scale;
	}
};
template<size_t index, size_t loop>
struct add_component_helper
{
	template<typename scalar_t, size_t dimension>
	add_component_helper(canonical_components_t<scalar_t, dimension>& result, const canonical_components_t<scalar_t, dimension>& u, const canonical_components_t<scalar_t, dimension>& v)
	{
		result.container[index] = u.container[index] + v.container[index];
	}
};
template<size_t index, size_t loop>
struct sub_component_helper
{
	template<typename scalar_t, size_t dimension>
	sub_component_helper(canonical_components_t<scalar_t, dimension>& result, const canonical_components_t<scalar_t, dimension>& u, const canonical_components_t<scalar_t, dimension>& v)
	{
		result.container[index] = u.container[index] - v.container[index];
	}
};
template<typename scalar_t, size_t dimension>
inline auto& operator *=(canonical_components_t<scalar_t, dimension>& u, const scalar_t& scale)
{
	using compoments_t = canonical_components_t<scalar_t, dimension>;
	static_for_each<0, compoments_t::parallel_count>::template iterate<multiply_component_helper>(u, u, scale);
	return u;
}
template<typename scalar_t, size_t dimension>
inline auto operator *(const canonical_components_t<scalar_t, dimension>& u, const scalar_t& scale)
{
	using compoments_t = canonical_components_t<scalar_t, dimension>;
	compoments_t result(compoments_t::UNINITIALIZED);
	static_for_each<0, compoments_t::parallel_count>::template iterate<multiply_component_helper>(result, u, scale);
	return std::move(result);
}
template<typename scalar_t, size_t dimension>
inline auto operator *(const scalar_t& scale, const canonical_components_t<scalar_t, dimension>& u)
{
	return std::move(u * scale);
}
template<typename scalar_t, size_t dimension>
inline const auto& operator /=(canonical_components_t<scalar_t, dimension>& u, const scalar_t& scale)
{
	const scalar_t inverse_scale = scalar_t(1) / scale;
	using compoments_t = canonical_components_t<scalar_t, dimension>;
	static_for_each<0, compoments_t::parallel_count>::template iterate<multiply_component_helper>(u, u, inverse_scale);
	return u;
}
template<typename scalar_t, size_t dimension>
inline auto operator /(const canonical_components_t<scalar_t, dimension>& u, const scalar_t& scale)
{
	const scalar_t inverse_scale = scalar_t(1) / scale;
	using compoments_t = canonical_components_t<scalar_t, dimension>;
	compoments_t result(compoments_t::UNINITIALIZED);
	static_for_each<0, compoments_t::parallel_count>::template iterate<multiply_component_helper>(result, u, inverse_scale);
	return std::move(result);
}

template<typename scalar_t, size_t dimension>
inline const auto& operator +=(canonical_components_t<scalar_t, dimension>& u, const canonical_components_t<scalar_t, dimension>& v)
{
	using compoments_t = canonical_components_t<scalar_t, dimension>;
	static_for_each<0, compoments_t::parallel_count>::template iterate<add_component_helper>(u, u, v);
	return u;
}
template<typename scalar_t, size_t dimension>
inline auto operator +(const canonical_components_t<scalar_t, dimension>& u, const canonical_components_t<scalar_t, dimension>& v)
{
	using compoments_t = canonical_components_t<scalar_t, dimension>;
	compoments_t result(compoments_t::UNINITIALIZED);
	static_for_each<0, compoments_t::parallel_count>::template iterate<add_component_helper>(result, u, v);
	return std::move(result);
}
template<typename scalar_t, size_t dimension>
inline const auto& operator -=(canonical_components_t<scalar_t, dimension>& u, const canonical_components_t<scalar_t, dimension>& v)
{
	using compoments_t = canonical_components_t<scalar_t, dimension>;
	static_for_each<0, compoments_t::parallel_count>::template iterate<sub_component_helper>(u, u, v);
	return u;
}
template<typename scalar_t, size_t dimension>
inline auto operator -(const canonical_components_t<scalar_t, dimension>& u, const canonical_components_t<scalar_t, dimension>& v)
{
	using compoments_t = canonical_components_t<scalar_t, dimension>;
	compoments_t result(compoments_t::UNINITIALIZED);
	static_for_each<0, compoments_t::parallel_count>::template iterate<sub_component_helper>(result, u, v);
	return std::move(result);
}
} // namespace SBLib::Containers::Mathematics
namespace SBLib { using namespace Containers::Mathematics; }

//
// TODO : put this in a separate file since it is optimizations overload
//
#if USE_DIRECTX_VECTOR
#include <DirectXMath.h>
namespace SBLib::Containers::Mathematics
{
using DirectX::operator*=;
using DirectX::operator*;
using DirectX::operator/=;
using DirectX::operator/;
using DirectX::operator+=;
using DirectX::operator+;
using DirectX::operator-=;
using DirectX::operator-;

//
// Specialized 4 x float
//
template<template<typename, size_t> class canonical_components_type>
struct canonical_components_helper<canonical_components_type, float, 4>
{
public:
	enum
	{
		dimension_size = 4,
		parallel_size  = 4,
		parallel_count = (dimension_size + (parallel_size - 1)) / parallel_size,
	};
	using scalar_type       = float;
	using parallel_type     = DirectX::XMVECTOR;
	using container_type    = DirectX::XMVECTORF32;
	using raw_type          = DirectX::XMVECTOR;

	scalar_type& operator[](const size_t index) { return static_cast<this_type*>(this)->container.f[index]; }
	const scalar_type& operator[](const size_t index) const { return static_cast<const this_type*>(this)->container.f[index]; }

private:
	using this_type = canonical_components_type<scalar_type, dimension_size>;
};

inline const auto& operator *=(canonical_components_t<float, 4>& u, const float scale)
{
	return u.container.v *= scale;
}
inline auto operator *(const canonical_components_t<float, 4> u, const float scale)
{
	return std::move(u.container.v * scale);
}
inline auto operator *(const float scale, const canonical_components_t<float, 4> v)
{
	return std::move(v * scale);
}
inline const auto& operator /=(canonical_components_t<float, 4>& u, const float scale)
{
	return u.container.v /= scale;
}
inline auto operator /(const canonical_components_t<float, 4> u, const float scale)
{
	return std::move(u.container.v / scale);
}
inline const auto& operator +=(canonical_components_t<float, 4>& u, const canonical_components_t<float, 4> v)
{
	return u.container.v += v.container.v;
}
inline auto operator +(const canonical_components_t<float, 4> u, const canonical_components_t<float, 4> v)
{
	return std::move(u.container.v + v.container.v);
}
inline const auto& operator -=(canonical_components_t<float, 4>& u, const canonical_components_t<float, 4> v)
{
	return u.container.v -= v.container.v;
}
inline auto operator -(const canonical_components_t<float, 4> u, const canonical_components_t<float, 4> v)
{
	return std::move(u.container.v - v.container.v);
}

//
// Specialized parallel 4 x float (dimension != 0)
//
template<template<typename, size_t> class canonical_components_type, size_t dimension>
struct canonical_components_helper<canonical_components_type, float, dimension>
{
public:
	enum
	{
		dimension_size = dimension,
		parallel_size  = 4,
		parallel_count = (dimension_size + (parallel_size - 1)) / parallel_size,
	};
	using scalar_type       = float;
	using parallel_type     = typename canonical_components_type<float, 4>::parallel_type;
	using container_type    = std::array<parallel_type, parallel_count>;
	using raw_type          = parallel_type[parallel_count];

	scalar_type& operator[](const size_t index)
	{
		acessor_type& container = reinterpret_cast<acessor_type&>(static_cast<this_type*>(this)->container[index / parallel_size]);
		return container.f[index % parallel_size];
	}
	const scalar_type& operator[](const size_t index) const
	{
		const acessor_type& container = reinterpret_cast<const acessor_type&>(static_cast<const this_type*>(this)->container[index / parallel_size]);
		return container.f[index % parallel_size];
	}

private:
	using this_type    = canonical_components_type<scalar_type, dimension_size>;
	using acessor_type = typename canonical_components_type<float, 4>::container_type;
};
template<size_t dimension>
struct canonical_components_t<float, dimension> : canonical_components_helper<canonical_components_t, float, dimension>
{
private:
	using parallel_type = typename canonical_components_helper<Mathematics::canonical_components_t, float, dimension>::parallel_type;
	template<size_t count, size_t index>
	struct unpack
	{
		template<typename... scalars>
		static void assign(container_type& dest, scalar_type&& s1, scalar_type&& s2, scalar_type&& s3, scalar_type&& s4, scalars&&... coords)
		{
			static_assert(parallel_size == 4, "Expecting floats packed by 4.");
			static_assert(count <= dimension, "Too many initializers.");
			parallel_type result = parallel_type{ s1, s2, s3, s4 };
			dest[index] = std::move(result);
			unpack<(count > parallel_size) ? count - parallel_size : 0, index + 1>::assign(dest, std::forward<scalars>(coords)...);
		}
		template<typename... scalars>
		static void assign(container_type& dest, scalars&&... coords)
		{
			static_assert(count <= dimension, "Too many initializers.");
			parallel_type result = parallel_type{std::forward<scalars>(coords)...};
			dest[index] = std::move(result);
			unpack<(count > parallel_size) ? count - parallel_size : 0, index + 1>::assign(dest, std::forward<scalars>(coords + parallel_size)...);
		}
	};
	template<size_t index>
	struct unpack<0, index>
	{
		template<typename... scalars>
		static void assign(container_type&, scalars&&... )
		{
		}
	};

public:
	enum eUNINITIALIZED : bool { UNINITIALIZED = true, };
	canonical_components_t(eUNINITIALIZED) {};

	canonical_components_t() : container{} {};
	canonical_components_t(canonical_components_t&& v) : container(std::move(v.container)) {};
	canonical_components_t(const canonical_components_t& v) : container(v.container) {};

	explicit canonical_components_t(const container_type& v) : container(v) {};
	explicit canonical_components_t(const raw_type& v) { container.v = v; };
	template<typename... scalars> explicit canonical_components_t(scalar_type&& first, scalars&&... coords) : container()
	{
		static_assert( sizeof...(scalars) < dimension, "Too many initializers." );
		unpack<sizeof...(coords) + 1, 0>::assign(container, std::move(first), std::forward<scalars>(coords)...);
	}

	canonical_components_t(container_type&& v) : container(v) {}
	canonical_components_t(raw_type&& v) { container.v = std::move(v); }

	const canonical_components_t& operator =(canonical_components_t&& v) { container = std::move(v.container); return *this; };
	const canonical_components_t& operator =(const canonical_components_t& v) { container = v.container; return *this; };

	operator container_type&() { return container; }
	operator const container_type&() const { return container; }

	container_type container;
};
} // namespace SBLib::Containers::Mathematics
#endif // #if USE_DIRECTX_VECTOR
