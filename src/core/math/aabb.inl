/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/math/aabb.h"
#include "core/math/matrix4x4.inl"
#include "core/math/vector3.inl"

namespace crown
{
namespace aabb
{
	inline void from_points(AABB &b, u32 num, const Vector3 *points)
	{
		aabb::from_points(b, num, sizeof(Vector3), points);
	}

	inline Vector3 center(const AABB &b)
	{
		return (b.min + b.max) * 0.5f;
	}

	inline f32 radius(const AABB &b)
	{
		return length(b.max - (b.min + b.max) * 0.5f);
	}

	inline f32 volume(const AABB &b)
	{
		return (b.max.x - b.min.x) * (b.max.y - b.min.y) * (b.max.z - b.min.z);
	}

	inline bool contains_point(const AABB &b, const Vector3 &p)
	{
		return p.x > b.min.x
			&& p.y > b.min.y
			&& p.z > b.min.z
			&& p.x < b.max.x
			&& p.y < b.max.y
			&& p.z < b.max.z
			;
	}

	inline AABB transformed(const AABB &b, const Matrix4x4 &m)
	{
		Vector3 vertices[8];
		to_vertices(vertices, b);

		vertices[0] = vertices[0] * m;
		vertices[1] = vertices[1] * m;
		vertices[2] = vertices[2] * m;
		vertices[3] = vertices[3] * m;
		vertices[4] = vertices[4] * m;
		vertices[5] = vertices[5] * m;
		vertices[6] = vertices[6] * m;
		vertices[7] = vertices[7] * m;

		AABB r;
		aabb::from_points(r, countof(vertices), vertices);
		return r;
	}

	inline void to_vertices(Vector3 vertices[8], const AABB &b)
	{
		// 7 ---- 6
		// |      |
		// |      |  <--- Top face
		// 4 ---- 5
		//
		// 3 ---- 2
		// |      |
		// |      |  <--- Bottom face
		// 0 ---- 1
		vertices[0].x = b.min.x;
		vertices[0].y = b.min.y;
		vertices[0].z = b.max.z;

		vertices[1].x = b.max.x;
		vertices[1].y = b.min.y;
		vertices[1].z = b.max.z;

		vertices[2].x = b.max.x;
		vertices[2].y = b.min.y;
		vertices[2].z = b.min.z;

		vertices[3].x = b.min.x;
		vertices[3].y = b.min.y;
		vertices[3].z = b.min.z;

		vertices[4].x = b.min.x;
		vertices[4].y = b.max.y;
		vertices[4].z = b.max.z;

		vertices[5].x = b.max.x;
		vertices[5].y = b.max.y;
		vertices[5].z = b.max.z;

		vertices[6].x = b.max.x;
		vertices[6].y = b.max.y;
		vertices[6].z = b.min.z;

		vertices[7].x = b.min.x;
		vertices[7].y = b.max.y;
		vertices[7].z = b.min.z;
	}

	inline Sphere to_sphere(const AABB &b)
	{
		Sphere s;
		s.c = aabb::center(b);
		s.r = aabb::radius(b);
		return s;
	}

} // namespace aabb

} // namespace crown
