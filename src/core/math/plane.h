/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
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
	/// Normalizes the plane @a p and returns its result.
	Plane& normalize(Plane& p);

	/// Returns the signed distance between plane @a p and point @a point.
	float distance_to_point(const Plane& p, const Vector3& point);

} // namespace plane

namespace plane
{
	// const Plane ZERO = Plane(vector3::ZERO, 0.0f);
	// const Plane XAXIS = Plane(vector3::XAXIS, 0.0f);
	// const Plane YAXIS = Plane(vector3::YAXIS, 0.0f);
	// const Plane ZAXIS = Plane(vector3::ZAXIS, 0.0f);

	const Plane ZERO  = { VECTOR3_ZERO, 0.0f };
	const Plane XAXIS = { VECTOR3_XAXIS, 0.0f };
	const Plane YAXIS = { VECTOR3_YAXIS, 0.0f };
	const Plane ZAXIS = { VECTOR3_ZAXIS, 0.0f };

	inline Plane& normalize(Plane& p)
	{
		const float len = length(p.n);

		if (equals(len, 0.0f))
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
