/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/math/math.h"
#include "core/math/matrix3x3.inl"
#include "core/math/types.h"

namespace crown
{
/// @addtogroup Math
/// @{

/// Returns a new quaternion from individual elements.
inline Quaternion from_elements(f32 x, f32 y, f32 z, f32 w)
{
	Quaternion q;
	q.x = x;
	q.y = y;
	q.z = z;
	q.w = w;
	return q;
}

/// Returns a new quaternion from @a axis and @a angle.
inline Quaternion from_axis_angle(const Vector3& axis, f32 angle)
{
	const f32 ha = angle * 0.5f;
	const f32 sa = fsin(ha);
	const f32 ca = fcos(ha);
	Quaternion q;
	q.x = axis.x * sa;
	q.y = axis.y * sa;
	q.z = axis.z * sa;
	q.w = ca;
	return q;
}

/// Returns a new quaternion from rotation matrix @a m.
Quaternion quaternion(const Matrix3x3& m);

/// Multiplies the quaternions @a a by @a b and returns the result. (i.e. rotates first by @a a then by @a b).
inline Quaternion& operator*=(Quaternion& a, const Quaternion& b)
{
	const f32 tx = a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y;
	const f32 ty = a.w*b.y + a.y*b.w + a.z*b.x - a.x*b.z;
	const f32 tz = a.w*b.z + a.z*b.w + a.x*b.y - a.y*b.x;
	const f32 tw = a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z;
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

/// Multiplies the quaternions @a a by @a b and returns the result. (i.e. rotates first by @a a then by @a b).
inline Quaternion operator*(Quaternion a, const Quaternion& b)
{
	a *= b;
	return a;
}

/// Multiplies the quaternion @a a by the scalar @a k.
inline Quaternion operator*(const Quaternion& q, f32 k)
{
	Quaternion r;
	r.x = q.x * k;
	r.y = q.y * k;
	r.z = q.z * k;
	r.w = q.w * k;
	return r;
}

/// Returns the dot product between quaternions @a a and @a b.
inline f32 dot(const Quaternion& a, const Quaternion& b)
{
	return a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z;
}

/// Returns the length of @a q.
inline f32 length(const Quaternion& q)
{
	return fsqrt(dot(q, q));
}

/// Normalizes the quaternion @a q and returns the result.
inline Quaternion& normalize(Quaternion& q)
{
	const f32 len = length(q);
	const f32 inv_len = 1.0f / len;
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
	const f32 len = length(q);
	const f32 inv_len = 1.0f / len;
	return conjugate(q) * inv_len;
}

/// Returns the quaternion @a q raised to the power of @a exp.
inline Quaternion power(const Quaternion& q, f32 exp)
{
	if (fabs(q.w) < 0.9999)
	{
		const f32 alpha = facos(q.w); // alpha = theta/2
		const f32 new_alpha = alpha * exp;
		const f32 mult = fsin(new_alpha) / fsin(alpha);

		Quaternion r;
		r.w = fcos(new_alpha);
		r.x = q.x * mult;
		r.y = q.y * mult;
		r.z = q.z * mult;
		return r;
	}

	return q;
}

/// Returns the quaternion describing the rotation needed to face towards @a dir.
inline Quaternion look(const Vector3& dir, const Vector3& up)
{
	const Vector3 xaxis = cross(up, dir);
	const Vector3 yaxis = cross(dir, xaxis);

	Matrix3x3 m;
	m.x = xaxis;
	m.y = yaxis;
	m.z = dir;
	Quaternion q = quaternion(m);
	normalize(q);
	return q;
}

/// Returns the right axis of the rotation represented by @a q.
inline Vector3 right(const Quaternion& q)
{
	const Matrix3x3 m = from_quaternion(q);
	return m.x;
}

/// Returns the up axis of the rotation represented by @a q.
inline Vector3 up(const Quaternion& q)
{
	const Matrix3x3 m = from_quaternion(q);
	return m.y;
}

/// Returns the forward axis of the rotation represented by @a q.
inline Vector3 forward(const Quaternion& q)
{
	const Matrix3x3 m = from_quaternion(q);
	return m.z;
}

/// Returns the linearly interpolated quaternion between *a* and *b* at time *t* in [0, 1]. It uses NLerp.
inline Quaternion lerp(const Quaternion& a, const Quaternion& b, f32 t)
{
	const f32 t1 = 1.0f - t;

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

/// Returns a string representing the quaternion @q.
/// @note This function is for debugging purposes only and doesn't
/// output round-trip safe ASCII conversions. Do not use in production.
const char* to_string(const Quaternion& q, char* buf, u32 buf_len);

// @}

} // namespace crown
