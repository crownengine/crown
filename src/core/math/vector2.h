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

const Vector2 VECTOR2_ZERO = { 0.0f, 0.0f };

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
	return vector2(-a.x, -a.y);
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
	return equals(a.x, b.x) && equals(a.y, b.y);
}

/// Returns the dot product between the vectors @a a and @a b.
inline float dot(const Vector2& a, const Vector2& b)
{
	return a.x * b.x + a.y * b.y;
}

/// Returns the lenght of @a a.
inline float length(const Vector2& a)
{
	return sqrt(a.x * a.x + a.y * a.y);
}

/// Returns the squared length of @a a.
inline float squared_length(const Vector2& a)
{
	return a.x * a.x + a.y * a.y;
}

/// Normalizes @a a and returns the result.
inline Vector2 normalize(Vector2& a)
{
	float inv_len = 1.0f / length(a);
	a.x *= inv_len;
	a.y *= inv_len;
	return a;
}

/// Sets the lenght of @a a to @a len.
inline void set_length(Vector2& a, float len)
{
	normalize(a);

	a.x *= len;
	a.y *= len;
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
