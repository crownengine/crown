/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "sphere.h"
#include "vector3.h"
#include "math_types.h"

namespace crown
{

/// Functions to manipulate Plane.
///
/// @ingroup Math
namespace plane
{
	/// Normalizes the plane @a p and returns its result.
	Plane& normalize(Plane& p);

	/// Returns the signed distance between plane @a p and point @a point.
	float distance_to_point(const Plane& p, const Vector3& point);

} // namespace plane

namespace plane
{
	const Plane ZERO = Plane(vector3::ZERO, 0.0);
	const Plane	XAXIS = Plane(vector3::XAXIS, 0.0);
	const Plane	YAXIS = Plane(vector3::YAXIS, 0.0);
	const Plane	ZAXIS = Plane(vector3::ZAXIS, 0.0);

	inline Plane& normalize(Plane& p)
	{
		float len = vector3::length(p.n);

		if (equals(len, (float) 0.0))
		{
			return p;
		}

		const float inv_len = (float) 1.0 / len;

		p.n *= inv_len;
		p.d *= inv_len;

		return p;
	}

	inline float distance_to_point(const Plane& p, const Vector3& point)
	{
		return vector3::dot(p.n, point) + p.d;
	}
} // namespace plane

inline Plane::Plane()
{
	// Do not initialize
}

inline Plane::Plane(const Vector3& normal, float dist)
	: n(normal), d(dist)
{
}

} // namespace crown
