/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/math/math.h"
#include "core/math/types.h"
#include "core/math/vector3.h"

namespace crown
{
/// Functions to manipulate Plane3.
///
/// @ingroup Math
namespace plane3
{
	/// Returns the plane defined by @a point and @a normal.
	Plane3 from_point_and_normal(const Vector3& point, const Vector3& normal);

	/// Normalizes the plane @a p and returns its result.
	Plane3& normalize(Plane3& p);

	/// Returns the signed distance between plane @a p and point @a point.
	f32 distance_to_point(const Plane3& p, const Vector3& point);

} // namespace plane3

namespace plane3
{
	inline Plane3 from_point_and_normal(const Vector3& point, const Vector3& normal)
	{
		Plane3 p;
		p.n = normal;
		p.d = -dot(normal, point);
		return p;
	}

	inline Plane3& normalize(Plane3& p)
	{
		const f32 len = length(p.n);

		if (fequal(len, 0.0f))
			return p;

		const f32 inv_len = 1.0f / len;

		p.n *= inv_len;
		p.d *= inv_len;

		return p;
	}

	inline f32 distance_to_point(const Plane3& p, const Vector3& point)
	{
		return dot(p.n, point) + p.d;
	}

} // namespace plane3

} // namespace crown
