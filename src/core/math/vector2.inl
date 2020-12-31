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
inline Vector2 vector2(f32 x, f32 y)
{
	Vector2 v;
	v.x = x;
	v.y = y;
	return v;
}

/// Adds the vector @a a to @a b and returns the result.
inline Vector2& operator+=(Vector2& a, const Vector2& b)
{
	a.x += b.x;
	a.y += b.y;
	return a;
}

/// Subtracts the vector @a b from @a a and returns the result.
inline Vector2& operator-=(Vector2& a, const Vector2& b)
{
	a.x -= b.x;
	a.y -= b.y;
	return a;
}

/// Multiplies the vector @a a by the scalar @a k and returns the result.
inline Vector2& operator*=(Vector2& a, f32 k)
{
	a.x *= k;
	a.y *= k;
	return a;
}

/// Negates @a a and returns the result.
inline Vector2 operator-(const Vector2& a)
{
	Vector2 v;
	v.x = -a.x;
	v.y = -a.y;
	return v;
}

/// Adds the vector @a a to @a b and returns the result.
inline Vector2 operator+(Vector2 a, const Vector2& b)
{
	a += b;
	return a;
}

/// Subtracts the vector @a b from @a a and returns the result.
inline Vector2 operator-(Vector2 a, const Vector2& b)
{
	a -= b;
	return a;
}

/// Multiplies the vector @a a by the scalar @a k and returns the result.
inline Vector2 operator*(Vector2 a, f32 k)
{
	a *= k;
	return a;
}

/// Multiplies the vector @a a by the scalar @a k and returns the result.
inline Vector2 operator*(f32 k, Vector2 a)
{
	a *= k;
	return a;
}

/// Returns true whether the vectors @a a and @a b are equal.
inline bool operator==(const Vector2& a, const Vector2& b)
{
	return fequal(a.x, b.x) && fequal(a.y, b.y);
}

/// Returns the dot product between the vectors @a a and @a b.
inline f32 dot(const Vector2& a, const Vector2& b)
{
	return a.x * b.x + a.y * b.y;
}

/// Returns the squared length of @a a.
inline f32 length_squared(const Vector2& a)
{
	return dot(a, a);
}

/// Returns the length of @a a.
inline f32 length(const Vector2& a)
{
	return fsqrt(length_squared(a));
}

/// Normalizes @a a and returns the result.
inline Vector2 normalize(Vector2& a)
{
	const f32 len = length(a);
	const f32 inv_len = 1.0f / len;
	a.x *= inv_len;
	a.y *= inv_len;
	return a;
}

/// Sets the length of @a a to @a len.
inline void set_length(Vector2& a, f32 len)
{
	normalize(a);
	a.x *= len;
	a.y *= len;
}

/// Returns the squared distance between the points @a a and @a b.
inline f32 distance_squared(const Vector2& a, const Vector2& b)
{
	return length_squared(b - a);
}

/// Returns the distance between the points @a a and @a b.
inline f32 distance(const Vector2& a, const Vector2& b)
{
	return length(b - a);
}

/// Returns the angle between the vectors @a a and @a b.
inline f32 angle(const Vector2& a, const Vector2& b)
{
	return facos(dot(a, b) / (length(a) * length(b)));
}

/// Returns a vector that contains the largest value for each element from @a a and @a b.
inline Vector2 max(const Vector2& a, const Vector2& b)
{
	Vector2 v;
	v.x = max(a.x, b.x);
	v.y = max(a.y, b.y);
	return v;
}

/// Returns a vector that contains the smallest value for each element from @a a and @a b.
inline Vector2 min(const Vector2& a, const Vector2& b)
{
	Vector2 v;
	v.x = min(a.x, b.x);
	v.y = min(a.y, b.y);
	return v;
}

/// Returns the linearly interpolated vector between @a a and @a b at time @a t in [0, 1].
inline Vector2 lerp(const Vector2& a, const Vector2& b, f32 t)
{
	Vector2 v;
	v.x = lerp(a.x, b.x, t);
	v.y = lerp(a.y, b.y, t);
	return v;
}

/// Returns the pointer to the data of @a a.
inline f32* to_float_ptr(Vector2& a)
{
	return &a.x;
}

/// Returns the pointer to the data of @a a.
inline const f32* to_float_ptr(const Vector2& a)
{
	return &a.x;
}

/// @}

} // namespace crown
