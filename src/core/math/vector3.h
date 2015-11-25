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

inline Vector3& operator/=(Vector3& a, float k)
{
	CE_ASSERT(k != 0.0f, "Division by zero");
	float inv = 1.0f / k;
	a.x *= inv;
	a.y *= inv;
	a.z *= inv;
	return a;
}

/// Negates @a a and returns the result.
inline Vector3 operator-(const Vector3& a)
{
	Vector3 res;
	res.x = -a.x;
	res.y = -a.y;
	res.z = -a.z;
	return res;
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

/// Divides the vector @a a by the scalar @a k and returns the result.
inline Vector3 operator/(Vector3 a, float k)
{
	a /= k;
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
	return vector3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

/// Returns the lenght of @a a.
inline float length(const Vector3& a)
{
	return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
}

/// Returns the squared length of @a a.
inline float squared_length(const Vector3& a)
{
	return a.x * a.x + a.y * a.y + a.z * a.z;
}

/// Normalizes @a a and returns the result.
inline Vector3 normalize(Vector3& a)
{
	float inv_len = 1.0f / length(a);
	a.x *= inv_len;
	a.y *= inv_len;
	a.z *= inv_len;
	return a;
}

/// Sets the lenght of @a a to @a len.
inline void set_length(Vector3& a, float len)
{
	normalize(a);
	a.x *= len;
	a.y *= len;
	a.z *= len;
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

/// Returns a vector that contains the largest value for each component from @a a and @a b.
inline Vector3 max(const Vector3& a, const Vector3& b)
{
	Vector3 v;
	v.x = fmax(a.x, b.x);
	v.y = fmax(a.y, b.y);
	v.z = fmax(a.z, b.z);
	return v;
}

/// Returns a vector that contains the smallest value for each component from @a a and @a b.
inline Vector3 min(const Vector3& a, const Vector3& b)
{
	Vector3 v;
	v.x = fmin(a.x, b.x);
	v.y = fmin(a.y, b.y);
	v.z = fmin(a.z, b.z);
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
