/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "math_types.h"
#include "vector3.h"

namespace crown
{

/// Functions to manipulate Plane.
///
/// @ingroup Math
namespace plane
{
	/// Returns the plane defined by @a point and @a normal.
	Plane from_point_and_normal(const Vector3& point, const Vector3& normal);

	/// Normalizes the plane @a p and returns its result.
	Plane& normalize(Plane& p);

	/// Returns the signed distance between plane @a p and point @a point.
	float distance_to_point(const Plane& p, const Vector3& point);

} // namespace plane

namespace plane
{
	inline Plane from_point_and_normal(const Vector3& point, const Vector3& normal)
	{
		Plane p;
		p.n = normal;
		p.d = -dot(normal, point);
		return p;
	}

	inline Plane& normalize(Plane& p)
	{
		const float len = length(p.n);

		if (fequal(len, 0.0f))
			return p;

		const float inv_len = 1.0f / len;

		p.n *= inv_len;
		p.d *= inv_len;

		return p;
	}

	inline float distance_to_point(const Plane& p, const Vector3& point)
	{
		return dot(p.n, point) + p.d;
	}
} // namespace plane

} // namespace crown
