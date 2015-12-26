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

/// Returns a new quaternion from individual sjson::

elements.
inline Quaternion quaternion(float x, float y, float z, float w)
{
	Quaternion q;
	q.x = x;
	q.y = y;
	q.z = z;
	q.w = w;
	return q;
}

/// Returns a new quaternion from @a axis and @a angle.
inline Quaternion quaternion(const Vector3& axis, float angle)
{
	const float ha = angle * 0.5f;
	const float sa = sinf(ha);
	const float ca = cosf(ha);
	Quaternion q;
	q.x = axis.x * sa;
	q.y = axis.y * sa;
	q.z = axis.z * sa;
	q.w = ca;
	return q;
}

/// Returns a new quaternion from matrix @a m.
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
	Quaternion r;
	r.x = -q.x;
	r.y = -q.y;
	r.z = -q.z;
	r.w = -q.w;
	return r;
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
	Quaternion r;
	r.x = q.x * k;
	r.y = q.y * k;
	r.z = q.z * k;
	r.w = q.w * k;
	return r;
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
	Quaternion r;
	r.x = -q.x;
	r.y = -q.y;
	r.z = -q.z;
	r.w = q.w;
	return r;
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

		Quaternion r;
		r.w = cosf(new_alpha);
		r.x = q.x * mult;
		r.y = q.y * mult;
		r.z = q.z * mult;
		return r;
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

/// Returns the right axis of the rotation represented by @a q.
inline Vector3 right(const Quaternion& q)
{
	const Matrix3x3 m = matrix3x3(q);
	return m.x;
}

/// Returns the up axis of the rotation represented by @a q.
inline Vector3 up(const Quaternion& q)
{
	const Matrix3x3 m = matrix3x3(q);
	return m.y;
}

/// Returns the forward axis of the rotation represented by @a q.
inline Vector3 forward(const Quaternion& q)
{
	const Matrix3x3 m = matrix3x3(q);
	return m.z;
}

/// Returns the linearly interpolated quaternion between *a* and *b* at time *t* in [0, 1]. It uses NLerp.
inline Quaternion lerp(const Quaternion& a, const Quaternion& b, float t)
{
	const float t1 = 1.0f - t;

	Quaternion r;

	if (dot(a, b) < 0.0f)
	{
		r.x = t1*a.x + t*-b.x;
		r.y = t1*a.y + t*-b.y;
		r.z = t1*a.z + t*-b.z;
		r.w = t1*a.w + t*-b.w;
	}
	else
	{
		r.x = t1*a.x + t*b.x;
		r.y = t1*a.y + t*b.y;
		r.z = t1*a.z + t*b.z;
		r.w = t1*a.w + t*b.w;
	}

	return normalize(r);
}

// @}
} // namespace crown
