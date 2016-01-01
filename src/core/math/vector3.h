/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "math_types.h"
#include "math_utils.h"

namespace crown
{
/// @addtogroup Math
/// @{

/// Returns a new vector from individual elements.
inline Vector3 vector3(float x, float y, float z)
{
	Vector3 v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}

inline Vector3& operator+=(Vector3& a, const Vector3& b)
{
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	return a;
}

inline Vector3& operator-=(Vector3& a, const Vector3& b)
{
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	return a;
}

inline Vector3& operator*=(Vector3& a, float k)
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
inline Vector3 operator*(Vector3 a, float k)
{
	a *= k;
	return a;
}

/// Multiplies the vector @a a by the scalar @a k and returns the result.
inline Vector3 operator*(float k, Vector3 a)
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
inline float dot(const Vector3& a, const Vector3& b)
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
inline float length_squared(const Vector3& a)
{
	return dot(a, a);
}

/// Returns the length of @a a.
inline float length(const Vector3& a)
{
	return sqrtf(length_squared(a));
}

/// Normalizes @a a and returns the result.
inline Vector3 normalize(Vector3& a)
{
	const float len = length(a);
	const float inv_len = 1.0f / len;
	a.x *= inv_len;
	a.y *= inv_len;
	a.z *= inv_len;
	return a;
}

/// Sets the length of @a a to @a len.
inline void set_length(Vector3& a, float len)
{
	normalize(a);
	a.x *= len;
	a.y *= len;
	a.z *= len;
}

/// Returns the squared distance between the points @a a and @a b.
inline float distance_squared(const Vector3& a, const Vector3& b)
{
	return length_squared(b - a);
}

/// Returns the distance between the points @a a and @a b.
inline float distance(const Vector3& a, const Vector3& b)
{
	return length(b - a);
}

/// Returns the angle between the vectors @a a and @a b.
inline float angle(const Vector3& a, const Vector3& b)
{
	return acos(dot(a, b) / (length(a) * length(b)));
}

/// Returns a vector that contains the largest value for each element from @a a and @a b.
inline Vector3 max(const Vector3& a, const Vector3& b)
{
	Vector3 v;
	v.x = fmax(a.x, b.x);
	v.y = fmax(a.y, b.y);
	v.z = fmax(a.z, b.z);
	return v;
}

/// Returns a vector that contains the smallest value for each element from @a a and @a b.
inline Vector3 min(const Vector3& a, const Vector3& b)
{
	Vector3 v;
	v.x = fmin(a.x, b.x);
	v.y = fmin(a.y, b.y);
	v.z = fmin(a.z, b.z);
	return v;
}

/// Returns the linearly interpolated vector between @a a and @a b at time @a t in [0, 1].
inline Vector3 lerp(const Vector3& a, const Vector3& b, float t)
{
	Vector3 v;
	v.x = lerp(a.x, b.x, t);
	v.y = lerp(a.y, b.y, t);
	v.z = lerp(a.z, b.z, t);
	return v;
}

/// Returns the pointer to the data of @a a.
inline float* to_float_ptr(Vector3& a)
{
	return &a.x;
}

/// Returns the pointer to the data of @a a.
inline const float* to_float_ptr(const Vector3& a)
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

/// @}
} // namespace crown
