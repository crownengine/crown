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
#include "MathTypes.h"

namespace crown
{

/// Negates @a a and returns the result.
Vector2 operator-(const Vector2& a);

/// Adds the vector @a a to @a b and returns the result.
Vector2 operator+(Vector2 a, const Vector2& b);

/// Subtracts the vector @a b from @a a and returns the result.
Vector2 operator-(Vector2 a, const Vector2& b);

/// Multiplies the vector @a a by the scalar @a k and returns the result.
Vector2 operator*(Vector2 a, float k);

/// @copydoc operator*(Vector2, float)
Vector2 operator*(float k, Vector2 a);

/// Divides the vector @a a by the scalar @a k and returns the result.
Vector2 operator/(Vector2 a, float k);

/// Returns true whether the vectors @a a and @a b are equal.
bool operator==(const Vector2& a, const Vector2& b);

/// Functions to manipulate Vector2.
///
/// @ingroup Math
namespace vector2
{
	const Vector2 ZERO = Vector2(0, 0);

	/// Returns the dot product between the vectors @a a and @a b.
	float dot(const Vector2& a, const Vector2& b);

	/// Returns the lenght of @a a.
	float length(const Vector2& a);

	/// Returns the squared length of @a a.
	float squared_length(const Vector2& a);

	/// Sets the lenght of @a a to @a len.
	void set_length(Vector2& a, float len);

	/// Normalizes @a a and returns the result.
	Vector2 normalize(Vector2& a);

	/// Returns the distance between the points @a a and @a b.
	float distance(const Vector2& a, const Vector2& b);

	/// Returns the angle between the vectors @a a and @a b.
	float angle(const Vector2& a, const Vector2& b);

	/// Returns the pointer to the data of @a a.
	float* to_float_ptr(Vector2& a);

	/// @copydoc to_float_ptr(Vector2&)
	const float* to_float_ptr(const Vector2& a);
} // namespace vector2

inline Vector2 operator-(const Vector2& a)
{
	return Vector2(-a.x, -a.y);
}

inline Vector2 operator+(Vector2 a, const Vector2& b)
{
	a += b;
	return a;
}

inline Vector2 operator-(Vector2 a, const Vector2& b)
{
	a -= b;
	return a;
}

inline Vector2 operator*(Vector2 a, float k)
{
	a *= k;
	return a;
}

inline Vector2 operator*(float k, Vector2 a)
{
	a *= k;
	return a;
}

inline Vector2 operator/(Vector2 a, float k)
{
	a /= k;
	return a;
}

inline bool operator==(const Vector2& a, const Vector2& b)
{
	return math::equals(a.x, b.x) && math::equals(a.y, b.y);
}

namespace vector2
{

	//-----------------------------------------------------------------------------
	inline float dot(const Vector2& a, const Vector2& b)
	{
		return a.x * b.x + a.y * b.y;
	}

	//-----------------------------------------------------------------------------
	inline float length(const Vector2& a)
	{
		return math::sqrt(a.x * a.x + a.y * a.y);
	}

	//-----------------------------------------------------------------------------
	inline float squared_length(const Vector2& a)
	{
		return a.x * a.x + a.y * a.y;
	}

	//-----------------------------------------------------------------------------
	inline void set_length(Vector2& a, float len)
	{
		normalize(a);

		a.x *= len;
		a.y *= len;
	}

	//-----------------------------------------------------------------------------
	inline Vector2 normalize(Vector2& a)
	{
		float inv_len = 1.0 / length(a);

		a.x *= inv_len;
		a.y *= inv_len;

		return a;
	}

	//-----------------------------------------------------------------------------
	inline float distance(const Vector2& a, const Vector2& b)
	{
		return length(b - a);
	}

	//-----------------------------------------------------------------------------
	inline float angle(const Vector2& a, const Vector2& b)
	{
		return math::acos(dot(a, b) / (length(a) * length(b)));
	}

	//-----------------------------------------------------------------------------
	inline float* to_float_ptr(Vector2& a)
	{
		return &a.x;
	}

	//-----------------------------------------------------------------------------
	inline const float* to_float_ptr(const Vector2& a)
	{
		return &a.x;
	}
} // namespace vector2

//-----------------------------------------------------------------------------
inline Vector2::Vector2()
{
	// Do not initialize
}

//-----------------------------------------------------------------------------
inline Vector2::Vector2(float val) : x(val), y(val)
{
}

//-----------------------------------------------------------------------------
inline Vector2::Vector2(float nx, float ny) : x(nx), y(ny)
{
}

//-----------------------------------------------------------------------------
inline Vector2::Vector2(const float a[2]) : x(a[0]), y(a[1])
{
}

//-----------------------------------------------------------------------------
inline float Vector2::operator[](uint32_t i) const
{
	CE_ASSERT(i < 2, "Index must be < 2");

	return (&x)[i];
}

//-----------------------------------------------------------------------------
inline float& Vector2::operator[](uint32_t i)
{
	CE_ASSERT(i < 2, "Index must be < 2");

	return (&x)[i];
}

//-----------------------------------------------------------------------------
inline Vector2& Vector2::operator+=(const Vector2& a)
{
	x += a.x;
	y += a.y;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vector2& Vector2::operator-=(const Vector2& a)
{
	x -= a.x;
	y -= a.y;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vector2& Vector2::operator*=(float k)
{
	x *= k;
	y *= k;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vector2& Vector2::operator/=(float k)
{
	CE_ASSERT(k != (float)0.0, "Division by zero");

	float inv = (float)(1.0 / k);

	x *= inv;
	y *= inv;

	return *this;
}

} // namespace crown
