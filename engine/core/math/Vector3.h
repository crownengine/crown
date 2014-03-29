/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "Assert.h"
#include "Types.h"
#include "MathUtils.h"
#include "Vector2.h"
#include "MathTypes.h"

namespace crown
{

/// Negates @a a and returns the result.
Vector3 operator-(const Vector3& a);

/// Adds the vector @a a to @a b and returns the result.
Vector3 operator+(Vector3 a, const Vector3& b);

/// Subtracts the vector @a b from @a a and returns the result.
Vector3 operator-(Vector3 a, const Vector3& b);

/// Multiplies the vector @a a by the scalar @a k and returns the result.
Vector3 operator*(Vector3 a, float k);

/// @copydoc operator*(Vector3, float)
Vector3 operator*(float k, Vector3 a);

/// Divides the vector @a a by the scalar @a k and returns the result.
Vector3 operator/(Vector3 a, float k);

/// Returns true whether the vectors @a a and @a b are equal.
bool operator==(const Vector3& a, const Vector3& b);

namespace vector3
{
	const Vector3 ZERO = Vector3(0, 0, 0);
	const Vector3 XAXIS = Vector3(1, 0, 0);
	const Vector3 YAXIS = Vector3(0, 1, 0);
	const Vector3 ZAXIS = Vector3(0, 0, 1);

	/// Returns the dot product between the vectors @a a and @a b.
	float dot(const Vector3& a, const Vector3& b);

	/// Returns the cross product between the vectors @a a and @a b.
	Vector3 cross(const Vector3& a, const Vector3& b);

	/// Returns the lenght of @a a.
	float length(const Vector3& a);

	/// Returns the squared length of @a a.
	float squared_length(const Vector3& a);

	/// Sets the lenght of @a a to @a len.
	void set_length(Vector3& a, float len);

	/// Normalizes @a a and returns the result.
	Vector3 normalize(Vector3& a);

	/// Returns the distance between the points @a a and @a b.
	float distance(const Vector3& a, const Vector3& b);

	/// Returns the angle between the vectors @a a and @a b.
	float angle(const Vector3& a, const Vector3& b);

	/// Returns the pointer to the data of @a a.
	float* to_float_ptr(Vector3& a);

	/// @copydoc to_float_ptr(Vector3&)
	const float* to_float_ptr(const Vector3& a);

	/// Returns the Vector2 portion of @a a. (i.e. truncates z)
	Vector2 to_vector2(const Vector3& a);
} // namespace vector3

inline Vector3 operator-(const Vector3& a)
{
	return Vector3(-a.x, -a.y, -a.z);
}

inline Vector3 operator+(Vector3 a, const Vector3& b)
{
	a += b;
	return a;
}

inline Vector3 operator-(Vector3 a, const Vector3& b)
{
	a -= b;
	return a;
}

inline Vector3 operator*(Vector3 a, float k)
{
	a *= k;
	return a;
}

inline Vector3 operator*(float k, Vector3 a)
{
	a *= k;
	return a;
}

inline Vector3 operator/(Vector3 a, float k)
{
	a /= k;
	return a;
}

inline bool operator==(const Vector3& a, const Vector3& b)
{
	return math::equals(a.x, b.x) && math::equals(a.y, b.y) && math::equals(a.z, b.z);
}

namespace vector3
{
	//-----------------------------------------------------------------------------
	inline float dot(const Vector3& a, const Vector3& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	//-----------------------------------------------------------------------------
	inline Vector3 cross(const Vector3& a, const Vector3& b)
	{
		return Vector3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
	}

	//-----------------------------------------------------------------------------
	inline float length(const Vector3& a)
	{
		return math::sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
	}

	//-----------------------------------------------------------------------------
	inline float squared_length(const Vector3& a)
	{
		return a.x * a.x + a.y * a.y + a.z * a.z;
	}

	//-----------------------------------------------------------------------------
	inline void set_length(Vector3& a, float len)
	{
		normalize(a);

		a.x *= len;
		a.y *= len;
		a.z *= len;
	}

	//-----------------------------------------------------------------------------
	inline Vector3 normalize(Vector3& a)
	{
		float inv_len = 1.0 / length(a);

		a.x *= inv_len;
		a.y *= inv_len;
		a.z *= inv_len;

		return a;
	}

	//-----------------------------------------------------------------------------
	inline float distance(const Vector3& a, const Vector3& b)
	{
		return length(b - a);
	}

	//-----------------------------------------------------------------------------
	inline float angle(const Vector3& a, const Vector3& b)
	{
		return math::acos(dot(a, b) / (length(a) * length(b)));
	}

	//-----------------------------------------------------------------------------
	inline float* to_float_ptr(Vector3& a)
	{
		return &a.x;
	}

	//-----------------------------------------------------------------------------
	inline const float* to_float_ptr(const Vector3& a)
	{
		return &a.x;
	}

	//-----------------------------------------------------------------------------
	inline Vector2 to_vector2(const Vector3& a)
	{
		return Vector2(a.x, a.y);
	}
} // namespace vector3

//-----------------------------------------------------------------------------
inline Vector3::Vector3()
{
	// Do not initialize
}

//-----------------------------------------------------------------------------
inline Vector3::Vector3(float val) : x(val), y(val), z(val)
{
}

//-----------------------------------------------------------------------------
inline Vector3::Vector3(float nx, float ny, float nz) : x(nx), y(ny), z(nz)
{
}

//-----------------------------------------------------------------------------
inline Vector3::Vector3(const float v[3]) : x(v[0]), y(v[1]), z(v[2])
{
}

//-----------------------------------------------------------------------------
inline Vector3::Vector3(const Vector3& a) : x(a.x), y(a.y), z(a.z)
{
}

//-----------------------------------------------------------------------------
inline float Vector3::operator[](uint32_t i) const
{
	CE_ASSERT(i < 3, "Index must be < 3");

	return (&x)[i];
}

//-----------------------------------------------------------------------------
inline float& Vector3::operator[](uint32_t i)
{
	CE_ASSERT(i < 3, "Index must be < 3");

	return (&x)[i];
}

//-----------------------------------------------------------------------------
inline Vector3& Vector3::operator+=(const Vector3& a)
{
	x += a.x;
	y += a.y;
	z += a.z;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vector3& Vector3::operator-=(const Vector3& a)
{
	x -= a.x;
	y -= a.y;
	z -= a.z;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vector3& Vector3::operator*=(float k)
{
	x *= k;
	y *= k;
	z *= k;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vector3& Vector3::operator/=(float k)
{
	CE_ASSERT(k != (float)0.0, "Division by zero");

	float inv = (float)(1.0 / k);

	x *= inv;
	y *= inv;
	z *= inv;

	return *this;
}

} // namespace crown
