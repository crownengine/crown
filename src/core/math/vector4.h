/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "math_types.h"
#include "math_utils.h"
#include "vector3.h"
#include "error.h"

namespace crown
{
/// @addtogroup Math
/// @{

/// Returns the Vector3 portion of @a a. (i.e. truncates w)
Vector3 to_vector3(const Vector4& a);

inline Vector4 vector4(float x, float y, float z, float w)
{
	Vector4 v;
	v.x = x;
	v.y = y;
	v.z = z;
	v.w = w;
	return v;
}

inline Vector4 vector4(const Vector3& a, float w)
{
	Vector4 v;
	v.x = a.x;
	v.y = a.y;
	v.z = a.z;
	v.w = w;
	return v;
}

inline Vector4& operator+=(Vector4& a,	const Vector4& b)
{
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	a.w += b.w;
	return a;
}

inline Vector4& operator-=(Vector4& a,	const Vector4& b)
{
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	a.w -= b.w;
	return a;
}

inline Vector4& operator*=(Vector4& a, float k)
{
	a.x *= k;
	a.y *= k;
	a.z *= k;
	a.w *= k;
	return a;
}

inline Vector4& operator/=(Vector4& a, float k)
{
	CE_ASSERT(k != 0.0f, "Division by zero");
	float inv = 1.0f / k;
	a.x *= inv;
	a.y *= inv;
	a.z *= inv;
	a.w *= inv;
	return a;
}

/// Negates @a a and returns the result.
inline Vector4 operator-(const Vector4& a)
{
	Vector4 res;
	res.x = -a.x;
	res.y = -a.y;
	res.z = -a.z;
	res.w = -a.w;
	return res;
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
inline Vector4 operator*(Vector4 a, float k)
{
	a *= k;
	return a;
}

/// Multiplies the vector @a a by the scalar @a k and returns the result.
inline Vector4 operator*(float k, Vector4 a)
{
	a *= k;
	return a;
}

/// Divides the vector @a a by the scalar @a k and returns the result.
inline Vector4 operator/(Vector4 a, float k)
{
	a /= k;
	return a;
}

/// Returns true whether the vectors @a a and @a b are equal.
inline bool operator==(const Vector4& a, const Vector4& b)
{
	return fequal(a.x, b.x)
		&& fequal(a.y, b.y)
		&& fequal(a.z, b.z)
		&& fequal(a.w, b.w);
}

/// Returns the dot product between the vectors @a a and @a b.
inline float dot(const Vector4& a, const Vector4& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

/// Returns the lenght of @a a.
inline float length(const Vector4& a)
{
	return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z + a.w * a.w);
}

/// Returns the squared length of @a a.
inline float squared_length(const Vector4& a)
{
	return a.x * a.x + a.y * a.y + a.z * a.z + a.w * a.w;
}

/// Normalizes @a a and returns the result.
inline Vector4 normalize(Vector4& a)
{
	float inv_len = 1.0f / length(a);
	a.x *= inv_len;
	a.y *= inv_len;
	a.z *= inv_len;
	a.w *= inv_len;
	return a;
}

/// Sets the lenght of @a a to @a len.
inline void set_length(Vector4& a, float len)
{
	normalize(a);
	a.x *= len;
	a.y *= len;
	a.z *= len;
	a.w *= len;
}

/// Returns the distance between the points @a a and @a b.
inline float distance(const Vector4& a, const Vector4& b)
{
	return length(b - a);
}

/// Returns the angle between the vectors @a a and @a b.
inline float angle(const Vector4& a, const Vector4& b)
{
	return acos(dot(a, b) / (length(a) * length(b)));
}

/// Returns the pointer to the data of @a a.
inline float* to_float_ptr(Vector4& a)
{
	return &a.x;
}

/// Returns the pointer to the data of @a a.
inline const float* to_float_ptr(const Vector4& a)
{
	return &a.x;
}

inline Vector3 to_vector3(const Vector4& a)
{
	return vector3(a.x, a.y, a.z);
}

/// @}
} // namespace crown
