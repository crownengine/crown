/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/math/vector3.inl"
#include "core/math/vector4.inl"

namespace crown
{
namespace plane3
{
	inline Plane3 from_point_and_normal(const Vector3 &point, const Vector3 &normal)
	{
		Plane3 p;
		p.n = normal;
		p.d = dot(normal, point);
		return p;
	}

	inline Plane3 &normalize(Plane3 &p)
	{
		const f32 len = length(p.n);

		if (fequal(len, 0.0f))
			return p;

		const f32 inv_len = 1.0f / len;

		p.n *= inv_len;
		p.d *= inv_len;

		return p;
	}

	inline f32 distance_to_point(const Plane3 &p, const Vector3 &point)
	{
		return dot(p.n, point) - p.d;
	}

	inline Plane3 transform(const Plane3 &p, const Matrix4x4 &m)
	{
		Vector4 n = { p.n.x, p.n.y, p.n.z, 0.0f };
		Vector4 o = n * p.d;
		o.w = 1.0f;

		n = n * m;
		o = o * m;

		Plane3 out;
		out.n = { n.x, n.y, n.z };
		out.d = dot(out.n, { o.x, o.y, o.z });
		return out;
	}

} // namespace plane3

} // namespace crown
