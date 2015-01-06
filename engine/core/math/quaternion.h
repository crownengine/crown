/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "vector3.h"
#include "math_types.h"

namespace crown
{

/// Negates the quaternion @a q and returns the result.
Quaternion operator-(const Quaternion& q);

/// Multiplies the quaternions @a a and @a b. (i.e. rotates first by @a a then by @a b).
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

	/// Normalizes the quaternion @a q and returns the result.
	Quaternion& normalize(Quaternion& q);

	/// Returns the conjugate of quaternion @a q.
	Quaternion conjugate(const Quaternion& q);

	/// Returns the inverse of quaternion @a q.
	Quaternion inverse(const Quaternion& q);

	/// Returns the quaternion @a q raised to the power of @a exp.
	Quaternion power(const Quaternion& q, float exp);
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
	inline float dot(const Quaternion& a, const Quaternion& b)
	{
		return a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z;
	}

	inline float length(const Quaternion& q)
	{
		return sqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
	}

	inline Quaternion& normalize(Quaternion& q)
	{
		const float inv_len = 1.0f / length(q);
		q.x *= inv_len;
		q.y *= inv_len;
		q.z *= inv_len;
		q.w *= inv_len;
		return q;
	}

	inline Quaternion conjugate(const Quaternion& q)
	{
		return Quaternion(-q.x, -q.y, -q.z, q.w);
	}

	inline Quaternion inverse(const Quaternion& q)
	{
		return conjugate(q) * (1.0f / length(q));
	}

	inline Quaternion power(const Quaternion& q, float exp)
	{
		if (abs(q.w) < 0.9999)
		{
			Quaternion tmp;
			float alpha = acos(q.w); // alpha = theta/2
			float new_alpha = alpha * exp;
			tmp.w = cos(new_alpha);
			float mult = sin(new_alpha) / sin(alpha);
			tmp.x = q.x * mult;
			tmp.y = q.y * mult;
			tmp.z = q.z * mult;
			return tmp;
		}

		return q;
	}
} // namespace quaternion

inline Quaternion::Quaternion()
{
	// Do not initialize
}

inline Quaternion::Quaternion(float nx, float ny, float nz, float nw)
	: x(nx)
	, y(ny)
	, z(nz)
	, w(nw)
{
}

inline Quaternion::Quaternion(const Vector3& axis, float angle)
	: x(axis.x * sin(angle * 0.5f))
	, y(axis.y * sin(angle * 0.5f))
	, z(axis.z * sin(angle * 0.5f))
	, w(cos(angle * 0.5f))
{
}

inline float& Quaternion::operator[](uint32_t i)
{
	CE_ASSERT(i < 4, "Index out of bounds");
	return (&x)[i];
}

inline const float& Quaternion::operator[](uint32_t i) const
{
	CE_ASSERT(i < 4, "Index out of bounds");
	return (&x)[i];
}

inline Quaternion& Quaternion::operator*=(const Quaternion& a)
{
	const float t_w = w*a.w - x*a.x - y*a.y - z*a.z;
	const float t_x = w*a.x + x*a.w + y*a.z - z*a.y;
	const float t_y = w*a.y + y*a.w + z*a.x - x*a.z;
	const float t_z = w*a.z + z*a.w + x*a.y - y*a.x;

	x = t_x;
	y = t_y;
	z = t_z;
	w = t_w;

	return *this;
}

} // namespace crown
