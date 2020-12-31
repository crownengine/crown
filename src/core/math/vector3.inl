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
inline Vector3 vector3(f32 x, f32 y, f32 z)
{
	Vector3 v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}

/// Adds the vector @a a to @a b and returns the result.
inline Vector3& operator+=(Vector3& a, const Vector3& b)
{
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	return a;
}

/// Subtracts the vector @a b from @a a and returns the result.
inline Vector3& operator-=(Vector3& a, const Vector3& b)
{
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	return a;
}

/// Multiplies the vector @a a by the scalar @a k and returns the result.
inline Vector3& operator*=(Vector3& a, f32 k)
{
	a.x *= k;
	a.y *= k;
	a.z *= k;
	return a;
}

/// Negates @a a and returns the result.
inline Vector3 operator-(const Vector3& a)
{
	Vector3 v;
	v.x = -a.x;
	v.y = -a.y;
	v.z = -a.z;
	return v;
}

/// Adds the vector @a a to @a b and returns the result.
inline Vector3 operator+(Vector3 a, const Vector3& b)
{
	a += b;
	return a;
}

/// Subtracts the vector @a b from @a a and returns the result.
inline Vector3 operator-(Vector3 a, const Vector3& b)
{
	a -= b;
	return a;
}

/// Multiplies the vector @a a by the scalar @a k and returns the result.
inline Vector3 operator*(Vector3 a, f32 k)
{
	a *= k;
	return a;
}

/// Multiplies the vector @a a by the scalar @a k and returns the result.
inline Vector3 operator*(f32 k, Vector3 a)
{
	a *= k;
	return a;
}

/// Returns true whether the vectors @a a and @a b are equal.
inline bool operator==(const Vector3& a, const Vector3& b)
{
	return fequal(a.x, b.x) && fequal(a.y, b.y) && fequal(a.z, b.z);
}

/// Returns the dot product between the vectors @a a and @a b.
inline f32 dot(const Vector3& a, const Vector3& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

/// Returns the cross product between the vectors @a a and @a b.
inline Vector3 cross(const Vector3& a, const Vector3& b)
{
	Vector3 v;
	v.x = a.y * b.z - a.z * b.y;
	v.y = a.z * b.x - a.x * b.z;
	v.z = a.x * b.y - a.y * b.x;
	return v;
}

/// Returns the squared length of @a a.
inline f32 length_squared(const Vector3& a)
{
	return dot(a, a);
}

/// Returns the length of @a a.
inline f32 length(const Vector3& a)
{
	return fsqrt(length_squared(a));
}

/// Normalizes @a a and returns the result.
inline Vector3 normalize(Vector3& a)
{
	const f32 len = length(a);
	const f32 inv_len = 1.0f / len;
	a.x *= inv_len;
	a.y *= inv_len;
	a.z *= inv_len;
	return a;
}

/// Sets the length of @a a to @a len.
inline void set_length(Vector3& a, f32 len)
{
	normalize(a);
	a.x *= len;
	a.y *= len;
	a.z *= len;
}

/// Returns the squared distance between the points @a a and @a b.
inline f32 distance_squared(const Vector3& a, const Vector3& b)
{
	return length_squared(b - a);
}

/// Returns the distance between the points @a a and @a b.
inline f32 distance(const Vector3& a, const Vector3& b)
{
	return length(b - a);
}

/// Returns the angle between the vectors @a a and @a b.
inline f32 angle(const Vector3& a, const Vector3& b)
{
	return facos(dot(a, b) / (length(a) * length(b)));
}

/// Returns a vector that contains the largest value for each element from @a a and @a b.
template <>
inline Vector3 max<Vector3>(const Vector3& a, const Vector3& b)
{
	Vector3 v;
	v.x = max(a.x, b.x);
	v.y = max(a.y, b.y);
	v.z = max(a.z, b.z);
	return v;
}

/// Returns a vector that contains the smallest value for each element from @a a and @a b.
template <>
inline Vector3 min<Vector3>(const Vector3& a, const Vector3& b)
{
	Vector3 v;
	v.x = min(a.x, b.x);
	v.y = min(a.y, b.y);
	v.z = min(a.z, b.z);
	return v;
}

/// Returns the linearly interpolated vector between @a a and @a b at time @a t in [0, 1].
inline Vector3 lerp(const Vector3& a, const Vector3& b, f32 t)
{
	Vector3 v;
	v.x = lerp(a.x, b.x, t);
	v.y = lerp(a.y, b.y, t);
	v.z = lerp(a.z, b.z, t);
	return v;
}

/// Returns the pointer to the data of @a a.
inline f32* to_float_ptr(Vector3& a)
{
	return &a.x;
}

/// Returns the pointer to the data of @a a.
inline const f32* to_float_ptr(const Vector3& a)
{
	return &a.x;
}

/// Returns the Vector2 portion of @a a. (i.e. truncates z)
inline Vector2 to_vector2(const Vector3& a)
{
	Vector2 v;
	v.x = a.x;
	v.y = a.y;
	return v;
}

/// Returns a string representing the vector @v.
/// @note This function is for debugging purposes only and doesn't
/// output round-trip safe ASCII conversions. Do not use in production.
const char* to_string(const Vector3& v, char* buf, u32 buf_len);

/// @}

} // namespace crown
