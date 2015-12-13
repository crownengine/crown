/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "math_types.h"
#include "math_utils.h"
#include "matrix3x3.h"

namespace crown
{
/// @addtogroup Math
/// @{

inline Quaternion quaternion(float x, float y, float z, float w)
{
	Quaternion q;
	q.x = x;
	q.y = y;
	q.z = z;
	q.w = w;
	return q;
}

inline Quaternion quaternion(const Vector3& axis, float angle)
{
	Quaternion q;
	q.x = axis.x * sinf(angle * 0.5f);
	q.y = axis.y * sinf(angle * 0.5f);
	q.z = axis.z * sinf(angle * 0.5f);
	q.w = cosf(angle * 0.5f);
	return q;
}

Quaternion quaternion(const Matrix3x3& m);

inline Quaternion& operator*=(Quaternion& a, const Quaternion& b)
{
	const float tx = a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y;
	const float ty = a.w*b.y + a.y*b.w + a.z*b.x - a.x*b.z;
	const float tz = a.w*b.z + a.z*b.w + a.x*b.y - a.y*b.x;
	const float tw = a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z;
	a.x = tx;
	a.y = ty;
	a.z = tz;
	a.w = tw;
	return a;
}

/// Negates the quaternion @a q and returns the result.
inline Quaternion operator-(const Quaternion& q)
{
	Quaternion res;
	res.x = -q.x;
	res.y = -q.y;
	res.z = -q.z;
	res.w = -q.w;
	return res;
}

/// Multiplies the quaternions @a a and @a b. (i.e. rotates first by @a a then by @a b).
inline Quaternion operator*(Quaternion a, const Quaternion& b)
{
	a *= b;
	return a;
}

/// Multiplies the quaternion @a a by the scalar @a k.
inline Quaternion operator*(const Quaternion& q, float k)
{
	Quaternion res;
	res.x = q.x * k;
	res.y = q.y * k;
	res.z = q.z * k;
	res.w = q.w * k;
	return res;
}

/// Returns the dot product between quaternions @a a and @a b.
inline float dot(const Quaternion& a, const Quaternion& b)
{
	return a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z;
}

/// Returns the length of @a q.
inline float length(const Quaternion& q)
{
	return sqrtf(dot(q, q));
}

/// Normalizes the quaternion @a q and returns the result.
inline Quaternion& normalize(Quaternion& q)
{
	const float len = length(q);
	const float inv_len = 1.0f / len;
	q.x *= inv_len;
	q.y *= inv_len;
	q.z *= inv_len;
	q.w *= inv_len;
	return q;
}

/// Returns the conjugate of quaternion @a q.
inline Quaternion conjugate(const Quaternion& q)
{
	Quaternion res;
	res.x = -q.x;
	res.y = -q.y;
	res.z = -q.z;
	res.w = q.w;
	return res;
}

/// Returns the inverse of quaternion @a q.
inline Quaternion inverse(const Quaternion& q)
{
	const float len = length(q);
	const float inv_len = 1.0f / len;
	return conjugate(q) * inv_len;
}

/// Returns the quaternion @a q raised to the power of @a exp.
inline Quaternion power(const Quaternion& q, float exp)
{
	if (fabs(q.w) < 0.9999)
	{
		const float alpha = acos(q.w); // alpha = theta/2
		const float new_alpha = alpha * exp;
		const float mult = sinf(new_alpha) / sinf(alpha);

		Quaternion tmp;
		tmp.w = cosf(new_alpha);
		tmp.x = q.x * mult;
		tmp.y = q.y * mult;
		tmp.z = q.z * mult;
		return tmp;
	}

	return q;
}

inline Quaternion look(const Vector3& dir, const Vector3& up = VECTOR3_YAXIS)
{
	const Vector3 right = cross(dir, up);
	const Vector3 nup = cross(right, dir);

	Matrix3x3 m;
	m.x = -right;
	m.y = nup;
	m.z = dir;
	return quaternion(m);
}

inline Vector3 right(const Quaternion& q)
{
	const Matrix3x3 m = matrix3x3(q);
	return m.x;
}

inline Vector3 up(const Quaternion& q)
{
	const Matrix3x3 m = matrix3x3(q);
	return m.y;
}

inline Vector3 forward(const Quaternion& q)
{
	const Matrix3x3 m = matrix3x3(q);
	return m.z;
}

// @}
} // namespace crown
