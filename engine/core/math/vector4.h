/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "assert.h"
#include "types.h"
#include "math_utils.h"
#include "vector3.h"

namespace crown
{

/// Negates @a a and returns the result.
Vector4 operator-(const Vector4& a);

/// Adds the vector @a a to @a b and returns the result.
Vector4 operator+(Vector4 a, const Vector4& b);

/// Subtracts the vector @a b from @a a and returns the result.
Vector4 operator-(Vector4 a, const Vector4& b);

/// Multiplies the vector @a a by the scalar @a k and returns the result.
Vector4 operator*(Vector4 a, float k);

/// @copydoc operator*(Vector4, float)
Vector4 operator*(float k, Vector4 a);

/// Divides the vector @a a by the scalar @a k and returns the result.
Vector4 operator/(Vector4 a, float k);

/// Returns true whether the vectors @a a and @a b are equal.
bool operator==(const Vector4& a, const Vector4& b);

/// Functions to manipulate Vector4.
///
/// @ingroup Math
namespace vector4
{
	const Vector4 ZERO = Vector4(0, 0, 0, 0);
	const Vector4 XAXIS = Vector4(1, 0, 0, 0);
	const Vector4 YAXIS = Vector4(0, 1, 0, 0);
	const Vector4 ZAXIS = Vector4(0, 0, 1, 0);
	const Vector4 WAXIS = Vector4(0, 0, 0, 1);

	/// Returns the dot product between the vectors @a a and @a b.
	float dot(const Vector4& a, const Vector4& b);

	/// Returns the lenght of @a a.
	float length(const Vector4& a);

	/// Returns the squared length of @a a.
	float squared_length(const Vector4& a);

	/// Sets the lenght of @a a to @a len.
	void set_length(Vector4& a, float len);

	/// Normalizes @a a and returns the result.
	Vector4 normalize(Vector4& a);

	/// Returns the distance between the points @a a and @a b.
	float distance(const Vector4& a, const Vector4& b);

	/// Returns the angle between the vectors @a a and @a b.
	float angle(const Vector4& a, const Vector4& b);

	/// Returns the pointer to the data of @a a.
	float* to_float_ptr(Vector4& a);

	/// @copydoc to_float_ptr(Vector4&)
	const float* to_float_ptr(const Vector4& a);

	/// Returns the Vector3 portion of @a a. (i.e. truncates w)
	Vector3 to_vector3(const Vector4& a);
} // namespace vector4

inline Vector4 operator-(const Vector4& a)
{
	return Vector4(-a.x, -a.y, -a.z, -a.w);
}

inline Vector4 operator+(Vector4 a, const Vector4& b)
{
	a += b;
	return a;
}

inline Vector4 operator-(Vector4 a, const Vector4& b)
{
	a -= b;
	return a;
}

inline Vector4 operator*(Vector4 a, float k)
{
	a *= k;
	return a;
}

inline Vector4 operator*(float k, Vector4 a)
{
	a *= k;
	return a;
}

inline Vector4 operator/(Vector4 a, float k)
{
	a /= k;
	return a;
}

inline bool operator==(const Vector4& a, const Vector4& b)
{
	return equals(a.x, b.x) && equals(a.y, b.y) && equals(a.z, b.z) && equals(a.w, b.w);
}

namespace vector4
{
	inline float dot(const Vector4& a, const Vector4& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
	}

	inline float length(const Vector4& a)
	{
		return sqrt(a.x * a.x + a.y * a.y + a.z * a.z + a.w * a.w);
	}

	inline float squared_length(const Vector4& a)
	{
		return a.x * a.x + a.y * a.y + a.z * a.z + a.w * a.w;
	}

	inline void set_length(Vector4& a, float len)
	{
		normalize(a);

		a.x *= len;
		a.y *= len;
		a.z *= len;
		a.w *= len;
	}

	inline Vector4 normalize(Vector4& a)
	{
		float inv_len = 1.0f / length(a);

		a.x *= inv_len;
		a.y *= inv_len;
		a.z *= inv_len;
		a.w *= inv_len;

		return a;
	}

	inline float distance(const Vector4& a, const Vector4& b)
	{
		return length(b - a);
	}

	inline float angle(const Vector4& a, const Vector4& b)
	{
		return acos(dot(a, b) / (length(a) * length(b)));
	}

	inline float* to_float_ptr(Vector4& a)
	{
		return &a.x;
	}

	inline const float* to_float_ptr(const Vector4& a)
	{
		return &a.x;
	}

	inline Vector3 to_vector3(const Vector4& a)
	{
		return Vector3(a.x, a.y, a.z);
	}
}

inline Vector4::Vector4()
{
	// Do not initialize
}

inline Vector4::Vector4(const Vector3& a, float w) : x(a.x), y(a.y), z(a.z), w(w)
{
}

inline Vector4::Vector4(float val) : x(val), y(val), z(val), w(val)
{
}

inline Vector4::Vector4(float nx, float ny, float nz, float nw) : x(nx), y(ny), z(nz), w(nw)
{
}

inline Vector4::Vector4(const float a[4]) : x(a[0]), y(a[1]), z(a[2]), w(a[3])
{
}

inline const float& Vector4::operator[](uint32_t i) const
{
	CE_ASSERT(i < 4, "Index out of bounds");

	return (&x)[i];
}

inline float& Vector4::operator[](uint32_t i)
{
	CE_ASSERT(i < 4, "Index out of bounds");

	return (&x)[i];
}

inline Vector4& Vector4::operator+=(const Vector4& a)
{
	x += a.x;
	y += a.y;
	z += a.z;
	w += a.w;

	return *this;
}

inline Vector4& Vector4::operator-=(const Vector4& a)
{
	x -= a.x;
	y -= a.y;
	z -= a.z;
	w -= a.w;

	return *this;
}

inline Vector4& Vector4::operator*=(float k)
{
	x *= k;
	y *= k;
	z *= k;
	w *= k;

	return *this;
}

inline Vector4& Vector4::operator/=(float k)
{
	CE_ASSERT(k != (float)0.0, "Division by zero");

	float inv = (float)(1.0 / k);

	x *= inv;
	y *= inv;
	z *= inv;
	w *= inv;

	return *this;
}

} // namespace crown
