/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/math/math.h"
#include "core/math/types.h"

namespace crown
{
/// @addtogroup Math
/// @{

/// Returns a new vector from individual elements.
inline Vector4 vector4(f32 x, f32 y, f32 z, f32 w)
{
	Vector4 v;
	v.x = x;
	v.y = y;
	v.z = z;
	v.w = w;
	return v;
}

/// Adds the vector @a a to @a b and returns the result.
inline Vector4& operator+=(Vector4& a, const Vector4& b)
{
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	a.w += b.w;
	return a;
}

/// Subtracts the vector @a b from @a a and returns the result.
inline Vector4& operator-=(Vector4& a, const Vector4& b)
{
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	a.w -= b.w;
	return a;
}

/// Multiplies the vector @a a by the scalar @a k and returns the result.
inline Vector4& operator*=(Vector4& a, f32 k)
{
	a.x *= k;
	a.y *= k;
	a.z *= k;
	a.w *= k;
	return a;
}

/// Negates @a a and returns the result.
inline Vector4 operator-(const Vector4& a)
{
	Vector4 v;
	v.x = -a.x;
	v.y = -a.y;
	v.z = -a.z;
	v.w = -a.w;
	return v;
}

/// Adds the vector @a a to @a b and returns the result.
inline Vector4 operator+(Vector4 a, const Vector4& b)
{
	a += b;
	return a;
}

/// Subtracts the vector @a b from @a a and returns the result.
inline Vector4 operator-(Vector4 a, const Vector4& b)
{
	a -= b;
	return a;
}

/// Multiplies the vector @a a by the scalar @a k and returns the result.
inline Vector4 operator*(Vector4 a, f32 k)
{
	a *= k;
	return a;
}

/// Multiplies the vector @a a by the scalar @a k and returns the result.
inline Vector4 operator*(f32 k, Vector4 a)
{
	a *= k;
	return a;
}

/// Returns true whether the vectors @a a and @a b are equal.
inline bool operator==(const Vector4& a, const Vector4& b)
{
	return fequal(a.x, b.x)
		&& fequal(a.y, b.y)
		&& fequal(a.z, b.z)
		&& fequal(a.w, b.w)
		;
}

/// Returns the dot product between the vectors @a a and @a b.
inline f32 dot(const Vector4& a, const Vector4& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

/// Returns the squared length of @a a.
inline f32 length_squared(const Vector4& a)
{
	return dot(a, a);
}

/// Returns the length of @a a.
inline f32 length(const Vector4& a)
{
	return fsqrt(length_squared(a));
}

/// Normalizes @a a and returns the result.
inline Vector4 normalize(Vector4& a)
{
	const f32 len = length(a);
	const f32 inv_len = 1.0f / len;
	a.x *= inv_len;
	a.y *= inv_len;
	a.z *= inv_len;
	a.w *= inv_len;
	return a;
}

/// Sets the length of @a a to @a len.
inline void set_length(Vector4& a, f32 len)
{
	normalize(a);
	a.x *= len;
	a.y *= len;
	a.z *= len;
	a.w *= len;
}

/// Returns the squared distance between the points @a a and @a b.
inline f32 distance_squared(const Vector4& a, const Vector4& b)
{
	return length_squared(b - a);
}

/// Returns the distance between the points @a a and @a b.
inline f32 distance(const Vector4& a, const Vector4& b)
{
	return length(b - a);
}

/// Returns the angle between the vectors @a a and @a b.
inline f32 angle(const Vector4& a, const Vector4& b)
{
	return facos(dot(a, b) / (length(a) * length(b)));
}

/// Returns a vector that contains the largest value for each element from @a a and @a b.
inline Vector4 max(const Vector4& a, const Vector4& b)
{
	Vector4 v;
	v.x = max(a.x, b.x);
	v.y = max(a.y, b.y);
	v.z = max(a.z, b.z);
	v.w = max(a.w, b.w);
	return v;
}

/// Returns a vector that contains the smallest value for each element from @a a and @a b.
inline Vector4 min(const Vector4& a, const Vector4& b)
{
	Vector4 v;
	v.x = min(a.x, b.x);
	v.y = min(a.y, b.y);
	v.z = min(a.z, b.z);
	v.w = min(a.w, b.w);
	return v;
}

/// Returns the linearly interpolated vector between @a a and @a b at time @a t in [0, 1].
inline Vector4 lerp(const Vector4& a, const Vector4& b, f32 t)
{
	Vector4 v;
	v.x = lerp(a.x, b.x, t);
	v.y = lerp(a.y, b.y, t);
	v.z = lerp(a.z, b.z, t);
	v.w = lerp(a.w, b.w, t);
	return v;
}

/// Returns the pointer to the data of @a a.
inline f32* to_float_ptr(Vector4& a)
{
	return &a.x;
}

/// Returns the pointer to the data of @a a.
inline const f32* to_float_ptr(const Vector4& a)
{
	return &a.x;
}

/// Returns the Vector3 portion of @a a. (i.e. truncates w)
inline Vector3 to_vector3(const Vector4& a)
{
	Vector3 v;
	v.x = a.x;
	v.y = a.y;
	v.z = a.z;
	return v;
}

/// Returns a string representing the vector @v.
/// @note This function is for debugging purposes only and doesn't
/// output round-trip safe ASCII conversions. Do not use in production.
const char* to_string(const Vector4& v, char* buf, u32 buf_len);

/// @}

} // namespace crown
