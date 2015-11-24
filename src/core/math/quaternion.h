/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "math_types.h"
#include "math_utils.h"

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
	const float t_w = a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z;
	const float t_x = a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y;
	const float t_y = a.w*b.y + a.y*b.w + a.z*b.x - a.x*b.z;
	const float t_z = a.w*b.z + a.z*b.w + a.x*b.y - a.y*b.x;
	a.x = t_x;
	a.y = t_y;
	a.z = t_z;
	a.w = t_w;
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
	return sqrtf(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
}

/// Normalizes the quaternion @a q and returns the result.
inline Quaternion& normalize(Quaternion& q)
{
	const float inv_len = 1.0f / length(q);
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
	return conjugate(q) * (1.0f / length(q));
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

// @}
} // namespace crown
