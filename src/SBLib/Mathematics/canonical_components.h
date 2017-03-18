#pragma once

#include <array>
#include <vector>
#define DIRECTX_VECTOR

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
	};
	using scalar_type       = scalar_t;
	using container_type    = std::array<scalar_type, dimension_size>;
	using intermediate_type = scalar_type[dimension_size];

	scalar_type& operator[](const size_t index) { return static_cast<this_type*>(this)->container[index]; }
	const scalar_type& operator[](const size_t index) const { return static_cast<const this_type*>(this)->container[index]; }

private:
	using this_type = canonical_components_type<scalar_t, dimension>;
};

//
// Generic "infinite" dimension
//
template<template<typename, size_t> class canonical_components_type, typename scalar_t>
struct canonical_components_helper<canonical_components_type, scalar_t, 0>
{
public:
	using scalar_type = scalar_t;
	using container_type = std::vector<scalar_type>;
	using intermediate_type = scalar_type*;

	scalar_type& operator[](const size_t index) { return static_cast<this_type*>(this)->container[index]; }
	const scalar_type& operator[](const size_t index) const { return static_cast<const this_type*>(this)->container[index]; }

private:
	using this_type = canonical_components_type<scalar_t, 0>;
};

//
// Generic canonical components
//
template<typename scalar_t, size_t dimension>
struct canonical_components_t : canonical_components_helper<canonical_components_t, scalar_t, dimension>
{
	canonical_components_t() : container{} {};
	canonical_components_t(canonical_components_t&& v): container(std::move(v.container)) {};
	canonical_components_t(const canonical_components_t& v) : container(v.container) {};

	explicit canonical_components_t(const container_type& v) : container(v) {};
	explicit canonical_components_t(const intermediate_type& v) { container.v = v; };
	template<typename... scalars> explicit canonical_components_t(scalar_type&& first, scalars&&... coords) : container{ first, std::forward<scalars>(coords)... } {}

	canonical_components_t(container_type&& v) : container(v) {}
	canonical_components_t(intermediate_type&& v) { container.v = std::move(v); }

	const canonical_components_t& operator =(canonical_components_t&& v) { container = std::move(v.container); return *this; };
	const canonical_components_t& operator =(const canonical_components_t& v) { container = v.container; return *this; };

	operator container_type&() { return container; }
	operator const container_type&() const { return container; }

	container_type container;
};

template<typename scalar_t, size_t dimension>
auto begin(canonical_components_t<scalar_t, dimension>& u)
{
	return u.container.begin();
}
template<typename scalar_t, size_t dimension>
auto begin(const canonical_components_t<scalar_t, dimension>& u)
{
	return u.container.begin();
}
template<typename scalar_t, size_t dimension>
auto end(const canonical_components_t<scalar_t, dimension>& u)
{
	return u.container.end();
}
template<typename scalar_t, size_t dimension>
const auto& operator *=(canonical_components_t<scalar_t, dimension>& u, const scalar_t& scale)
{
	std::transform(begin(v), end(v), begin(u), [&scale](const auto& u) -> auto { return u * scale; });
	return u;
}
template<typename scalar_t, size_t dimension>
auto operator *(const canonical_components_t<scalar_t, dimension>& v, const scalar_t& scale)
{
	canonical_components_t<scalar_t, dimension> value;
	std::transform(begin(v), end(v), begin(value), [&scale](const auto& u) -> auto { return u * scale; });
	return std::move(value);
}
template<typename scalar_t, size_t dimension>
auto operator *(const scalar_t& scale, const canonical_components_t<scalar_t, dimension>& u)
{
	return std::move(u * scale);
}
template<typename scalar_t, size_t dimension>
const auto& operator /=(canonical_components_t<scalar_t, dimension>& u, const scalar_t& scale)
{
	const scalar_t inverse_scale = scalar_t(1) / scale;
	std::transform(begin(v), end(v), begin(u), [&inverse_scale](const auto& u) -> auto { return u * inverse_scale; });
	return u;
}
template<typename scalar_t, size_t dimension>
auto operator /(const canonical_components_t<scalar_t, dimension>& v, const scalar_t& scale)
{
	const scalar_t inverse_scale = scalar_t(1) / scale;
	canonical_components_t<scalar_t, dimension> value;
	std::transform(begin(v), end(v), begin(value), [&inverse_scale](const auto& u) -> auto { return u * scale; });
	return std::move(value);
}
template<typename scalar_t, size_t dimension>
auto operator +(const canonical_components_t<scalar_t, dimension>& u, const canonical_components_t<scalar_t, dimension>& v)
{
	canonical_components_t<scalar_t, dimension> value;
	std::transform(begin(u), end(u), begin(v), begin(value), [](const auto& a, const auto& b) -> auto { return a + b; });
	return std::move(value);
}
template<typename scalar_t, size_t dimension>
const auto& operator +=(canonical_components_t<scalar_t, dimension>& u, const canonical_components_t<scalar_t, dimension>& v)
{
	std::transform(begin(u), end(u), begin(v), begin(u), [](const auto& a, const auto& b) -> auto { return a + b; });
	return u;
}

#if defined( DIRECTX_VECTOR )
#include <DirectXMath.h>
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
	};
	using scalar_type       = float;
	using container_type = DirectX::XMVECTORF32;
	using intermediate_type = DirectX::XMVECTOR;

	scalar_type& operator[](const size_t index) { return static_cast<this_type*>(this)->container.f[index]; }
	const scalar_type& operator[](const size_t index) const { return static_cast<const this_type*>(this)->container.f[index]; }

private:
	using this_type = canonical_components_type<scalar_type, dimension_size>;
};

auto begin(canonical_components_t<float, 4>& u)
{
	return &u.container.f[0];
}
auto begin(const canonical_components_t<float, 4>& u)
{
	return &u.container.f[0];
}
auto end(const canonical_components_t<float, 4>& u)
{
	return &u.container.f[u.dimension_size];
}
const auto& operator *=(canonical_components_t<float, 4>& u, const float scale)
{
	using namespace DirectX;
	return u.container.v *= scale;
}
auto operator *(const canonical_components_t<float, 4> u, const float scale)
{
	return std::move(u.container * scale);
}
auto operator *(const float scale, const canonical_components_t<float, 4> v)
{
	return std::move(v * scale);
}
const auto& operator /=(canonical_components_t<float, 4>& u, const float scale)
{
	using namespace DirectX;
	return u.container.v /= scale;
}
auto operator /(const canonical_components_t<float, 4> u, const float scale)
{
	return std::move(u.container / scale);
}
const auto& operator +=(canonical_components_t<float, 4>& u, const canonical_components_t<float, 4> v)
{
	using namespace DirectX;
	return u.container.v += v.container.v;
}
auto operator +(const canonical_components_t<float, 4> u, const canonical_components_t<float, 4> v)
{
	return std::move(u.container + v.container);
}

//
// Specialized parallel 4 x float
//
template<template<typename, size_t> class canonical_components_type, size_t dimension>
struct canonical_components_helper<canonical_components_type, float, dimension> : protected canonical_components_helper<canonical_components_type, canonical_components_type<float, 4>, (dimension + 3) / 4>
{
public:
	enum
	{
		dimension_size = dimension,
		parallel_dimension_size  = 4,
		parallel_dimension_count = (dimension + 3) / parallel_dimension_size,
	};
	using scalar_type = float;
	using parallel_type = canonical_components_type<float, 4>;
	using container_type = std::array<parallel_type, parallel_dimension_count>;
	using intermediate_type = parallel_type[parallel_dimension_count];

	scalar_type& operator[](const size_t index) { return this_type::operator[](index / parallel_dimension_size)[index % parallel_dimension_size]; }
	const scalar_type& operator[](const size_t index) const { return this_type::operator[](index / parallel_dimension_size)[index % parallel_dimension_size]; }

private:
	using this_type = canonical_components_helper<canonical_components_type, parallel_type, parallel_dimension_count>;
};
#endif // #if defined( DIRECTX_VECTOR )
