/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "math_types.h"
#include "math_utils.h"
#include "error.h"

namespace crown
{
/// @addtogroup Math
/// @{

inline Vector2 vector2(float x, float y)
{
	Vector2 v;
	v.x = x;
	v.y = y;
	return v;
}

inline Vector2& operator+=(Vector2& a, const Vector2& b)
{
	a.x += b.x;
	a.y += b.y;
	return a;
}

inline Vector2& operator-=(Vector2& a, const Vector2& b)
{
	a.x -= b.x;
	a.y -= b.y;
	return a;
}

inline Vector2& operator*=(Vector2& a, float k)
{
	a.x *= k;
	a.y *= k;
	return a;
}

inline Vector2& operator/=(Vector2& a, float k)
{
	CE_ASSERT(k != 0.0f, "Division by zero");
	float inv = 1.0f / k;
	a.x *= inv;
	a.y *= inv;
	return a;
}

/// Negates @a a and returns the result.
inline Vector2 operator-(const Vector2& a)
{
	Vector2 res;
	res.x = -a.x;
	res.y = -a.y;
	return res;
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
inline Vector2 operator*(Vector2 a, float k)
{
	a *= k;
	return a;
}

/// Multiplies the vector @a a by the scalar @a k and returns the result.
inline Vector2 operator*(float k, Vector2 a)
{
	a *= k;
	return a;
}

/// Divides the vector @a a by the scalar @a k and returns the result.
inline Vector2 operator/(Vector2 a, float k)
{
	a /= k;
	return a;
}

/// Returns true whether the vectors @a a and @a b are equal.
inline bool operator==(const Vector2& a, const Vector2& b)
{
	return fequal(a.x, b.x) && fequal(a.y, b.y);
}

/// Returns the dot product between the vectors @a a and @a b.
inline float dot(const Vector2& a, const Vector2& b)
{
	return a.x * b.x + a.y * b.y;
}

/// Returns the squared length of @a a.
inline float length_squared(const Vector2& a)
{
	return dot(a, a);
}

/// Returns the length of @a a.
inline float length(const Vector2& a)
{
	return sqrtf(length_squared(a));
}

/// Normalizes @a a and returns the result.
inline Vector2 normalize(Vector2& a)
{
	const float len = length(a);
	const float inv_len = 1.0f / len;
	a.x *= inv_len;
	a.y *= inv_len;
	return a;
}

/// Sets the length of @a a to @a len.
inline void set_length(Vector2& a, float len)
{
	normalize(a);
	a.x *= len;
	a.y *= len;
}

/// Returns the squared distance between the points @a a and @a b.
inline float distance_squared(const Vector2& a, const Vector2& b)
{
	return length_squared(b - a);
}

/// Returns the distance between the points @a a and @a b.
inline float distance(const Vector2& a, const Vector2& b)
{
	return length(b - a);
}

/// Returns the angle between the vectors @a a and @a b.
inline float angle(const Vector2& a, const Vector2& b)
{
	return acos(dot(a, b) / (length(a) * length(b)));
}

/// Returns a vector that contains the largest value for each component from @a a and @a b.
inline Vector2 max(const Vector2& a, const Vector2& b)
{
	Vector2 v;
	v.x = fmax(a.x, b.x);
	v.y = fmax(a.y, b.y);
	return v;
}

/// Returns a vector that contains the smallest value for each component from @a a and @a b.
inline Vector2 min(const Vector2& a, const Vector2& b)
{
	Vector2 v;
	v.x = fmin(a.x, b.x);
	v.y = fmin(a.y, b.y);
	return v;
}

/// Returns the linearly interpolated vector between @a and @b at time @a t in [0, 1].
inline Vector2 lerp(const Vector2& a, const Vector2& b, float t)
{
	Vector2 v;
	v.x = lerp(a.x, b.x, t);
	v.y = lerp(a.y, b.y, t);
	return v;
}

/// Returns the pointer to the data of @a a.
inline float* to_float_ptr(Vector2& a)
{
	return &a.x;
}

/// Returns the pointer to the data of @a a.
inline const float* to_float_ptr(const Vector2& a)
{
	return &a.x;
}

/// @}
} // namespace crown
