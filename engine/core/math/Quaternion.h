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

#include "Types.h"
#include "Vector3.h"
#include "MathTypes.h"
#include "Matrix3x3.h"
#include "Matrix4x4.h"

namespace crown
{

/// Negates the quaternion @a q and returns the result.
Quaternion operator-(const Quaternion& q);

/// Multiplies the quaternions @a and @a b. (i.e. rotates first by @a a then by @a b).
Quaternion operator*(Quaternion a, const Quaternion& b);

/// Multiplies the quaternion @a a by the scalar @a k.
Quaternion operator*(const Quaternion& a, float k);

/// Functions to manipulate Quaternion.
///
/// @ingroup Math
namespace quaternion
{
	const Quaternion IDENTITY = Quaternion(0.0, 0.0, 0.0, 1.0);

	/// Returns the dot product between quaternions @a a and @a b.
	float dot(const Quaternion& a, const Quaternion& b);

	/// Returns the length of @a q.
	float length(const Quaternion& q);

	/// Returns the conjugate of quaternion @a q.
	Quaternion conjugate(const Quaternion& q);

	/// Returns the inverse of quaternion @a q.
	Quaternion inverse(const Quaternion& q);

	/// Returns the quaternion @a q raised to the power of @a exp.
	Quaternion power(const Quaternion& q, float exp);

	/// Returns the Matrix3x3 representation of the quaternion @a q.
	Matrix3x3 to_matrix3x3(const Quaternion& q);

	/// Returns the Matrix4x4 representation of the quaternion @a q.
	Matrix4x4 to_matrix4x4(const Quaternion& q);

} // namespace quaternion

inline Quaternion operator-(const Quaternion& q)
{
	return Quaternion(-q.x, -q.y, -q.z, -q.w);
}

inline Quaternion operator*(Quaternion a, const Quaternion& b)
{
	a *= b;
	return a;
}

inline Quaternion operator*(const Quaternion& a, float k)
{
	return Quaternion(a.x * k, a.y * k, a.z * k, a.w * k);
}

namespace quaternion
{
	//-----------------------------------------------------------------------------
	inline float dot(const Quaternion& a, const Quaternion& b)
	{
		return a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z;
	}

	//-----------------------------------------------------------------------------
	inline float length(const Quaternion& q)
	{
		return math::sqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
	}

	inline Quaternion conjugate(const Quaternion& q)
	{
		return Quaternion(-q.x, -q.y, -q.z, q.w);
	}

	inline Quaternion inverse(const Quaternion& q)
	{
		return conjugate(q) * (1.0 / length(q));
	}

	inline Quaternion power(const Quaternion& q, float exp)
	{
		if (math::abs(q.w) < 0.9999)
		{
			Quaternion tmp;
			float alpha = math::acos(q.w); // alpha = theta/2
			float new_alpha = alpha * exp;
			tmp.w = math::cos(new_alpha);
			float mult = math::sin(new_alpha) / math::sin(alpha);
			tmp.x = q.x * mult;
			tmp.y = q.y * mult;
			tmp.z = q.z * mult;
			return tmp;
		}

		return q;
	}

	inline Matrix3x3 to_matrix3x3(const Quaternion& q)
	{
		const float x = q.x;
		const float y = q.y;
		const float z = q.z;
		const float w = q.w;

		Matrix3x3 tmp;

		tmp.m[0] = (float)(1.0 - 2.0*y*y - 2.0*z*z);
		tmp.m[1] = (float)(2.0*x*y + 2.0*w*z);
		tmp.m[2] = (float)(2.0*x*z - 2.0*w*y);
		tmp.m[3] = (float)(2.0*x*y - 2.0*w*z);
		tmp.m[4] = (float)(1.0 - 2.0*x*x - 2.0*z*z);
		tmp.m[5] = (float)(2.0*y*z + 2.0*w*x);
		tmp.m[6] = (float)(2.0*x*z + 2.0*w*y);
		tmp.m[7] = (float)(2.0*y*z - 2.0*w*x);
		tmp.m[8] = (float)(1.0 - 2.0*x*x - 2.0*y*y);

		return tmp;
	}

	inline Matrix4x4 to_matrix4x4(const Quaternion& q)
	{
		const float x = q.x;
		const float y = q.y;
		const float z = q.z;
		const float w = q.w;

		Matrix4x4 tmp;

		tmp.m[0] = (float)(1.0 - 2.0*y*y - 2.0*z*z);
		tmp.m[1] = (float)(2.0*x*y + 2.0*w*z);
		tmp.m[2] = (float)(2.0*x*z - 2.0*w*y);
		tmp.m[3] = 0;
		tmp.m[4] = (float)(2.0*x*y - 2.0*w*z);
		tmp.m[5] = (float)(1.0 - 2.0*x*x - 2.0*z*z);
		tmp.m[6] = (float)(2.0*y*z + 2.0*w*x);
		tmp.m[7] = 0.0;
		tmp.m[8] = (float)(2.0*x*z + 2.0*w*y);
		tmp.m[9] = (float)(2.0*y*z - 2.0*w*x);
		tmp.m[10] = (float)(1.0 - 2.0*x*x - 2.0*y*y);
		tmp.m[11] = 0.0;
		tmp.m[12] = 0.0;
		tmp.m[13] = 0.0;
		tmp.m[14] = 0.0;
		tmp.m[15] = 1.0;

		return tmp;
	}
} // namespace quaternion

//-----------------------------------------------------------------------------
inline Quaternion::Quaternion()
{
	// Do not initialize
}

//-----------------------------------------------------------------------------
inline Quaternion::Quaternion(float nx, float ny, float nz, float nw)
	: x(nx)
	, y(ny)
	, z(nz)
	, w(nw)
{
}

//-----------------------------------------------------------------------------
inline Quaternion::Quaternion(const Vector3& axis, float angle)
	: x(axis.x * math::sin(angle * 0.5))
	, y(axis.y * math::sin(angle * 0.5))
	, z(axis.z * math::sin(angle * 0.5))
	, w(math::cos(angle * 0.5))
{
}

//-----------------------------------------------------------------------------
inline Quaternion& Quaternion::operator*=(const Quaternion& a)
{
	const float t_x = w * a.x + x * a.w + z * a.y - y * a.z;
	const float t_y = w * a.y + y * a.w + x * a.z - z * a.x;
	const float t_z = w * a.z + z * a.w + y * a.x - x * a.y;
	const float t_w = w * a.w - x * a.x - y * a.y - z * a.z;

	x = t_x;
	y = t_y;
	z = t_z;
	w = t_w;

	return *this;
}

} // namespace crown

