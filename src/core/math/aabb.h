/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "math_types.h"
#include "vector3.h"
#include "matrix4x4.h"
#include "error.h"

namespace crown
{

/// Functions to manipulate AABB.
///
/// @ingroup Math
namespace aabb
{
	/// Resets the AABB @a b.
	void reset(AABB& b);

	/// Returns the center of the box @a b.
	Vector3 center(const AABB& b);

	/// Returns the radius of the box @a b.
	float radius(const AABB& b);

	/// Returns the volume of the box @a b.
	float volume(const AABB& b);

	/// Adds @a num @a points to the box @a b, expanding its bounds if necessary.
	void add_points(AABB& a, uint32_t num, uint32_t stride, const void* points);

	/// Adds @a num @a points to the box @a b, expanding its bounds if necessary.
	void add_points(AABB& b, uint32_t num, const Vector3* points);

	/// Adds @a num @a boxes to the box @a b, expanding its bounds if necessary.
	void add_boxes(AABB& b, uint32_t num, const AABB* boxes);

	/// Returns whether point @a p is contained in the box @a b.
	bool contains_point(const AABB& b, const Vector3& p);

	/// Returns the @a index -th vertex of the box.
	Vector3 vertex(const AABB& b, uint32_t index);

	/// Returns the box enclosing @a b transformed by @a m.
	AABB transformed(const AABB& b, const Matrix4x4& m);

	/// Returns the eight vertices of the box @a b.
	void to_vertices(const AABB& b, Vector3 v[8]);

	/// Returns the sphere enclosing the box @a b.
	Sphere to_sphere(const AABB& b);
}

namespace aabb
{
	inline void reset(AABB& b)
	{
		b.min = VECTOR3_ZERO;
		b.max = VECTOR3_ZERO;
	}

	inline Vector3 center(const AABB& b)
	{
		return (b.min + b.max) * 0.5f;
	}

	inline float radius(const AABB& b)
	{
		return length(b.max - (b.min + b.max) * 0.5f);
	}
	inline float volume(const AABB& b)
	{
		return (b.max.x - b.min.x) * (b.max.y - b.min.y) * (b.max.z - b.min.z);
	}

	inline void add_points(AABB& b, uint32_t num, uint32_t stride, const void* points)
	{
		for (uint32_t i = 0; i < num; ++i)
		{
			const Vector3* p = (const Vector3*)points;

			if (p->x < b.min.x) b.min.x = p->x;
			if (p->y < b.min.y) b.min.y = p->y;
			if (p->z < b.min.z) b.min.z = p->z;
			if (p->x > b.max.x) b.max.x = p->x;
			if (p->y > b.max.y) b.max.y = p->y;
			if (p->z > b.max.z) b.max.z = p->z;

			points = (const void*)((const char*)points + stride);
		}
	}

	inline void add_points(AABB& b, uint32_t num, const Vector3* points)
	{
		add_points(b, num, sizeof(Vector3), points);
	}

	inline void add_boxes(AABB& b, uint32_t num, const AABB* boxes)
	{
		for (uint32_t i = 0; i < num; ++i)
		{
			const AABB& box = boxes[i];

			if (box.min.x < b.min.x) b.min.x = box.min.x;
			if (box.min.y < b.min.y) b.min.y = box.min.y;
			if (box.min.z < b.min.z) b.min.z = box.min.z;
			if (box.max.x > b.max.x) b.max.x = box.max.x;
			if (box.max.y > b.max.y) b.max.y = box.max.y;
			if (box.max.z > b.max.z) b.max.z = box.max.z;
		}
	}

	inline bool contains_point(const AABB& b, const Vector3& p)
	{
		return p.x > b.min.x
			&& p.y > b.min.y
			&& p.z > b.min.z
			&& p.x < b.max.x
			&& p.y < b.max.y
			&& p.z < b.max.z
			;
	}

	inline Vector3 vertex(const AABB& b, uint32_t index)
	{
		switch (index)
		{
			case 0: return vector3(b.min.x, b.min.y, b.min.z);
			case 1: return vector3(b.max.x, b.min.y, b.min.z);
			case 2: return vector3(b.max.x, b.min.y, b.max.z);
			case 3: return vector3(b.min.x, b.min.y, b.max.z);
			case 4: return vector3(b.min.x, b.max.y, b.min.z);
			case 5: return vector3(b.max.x, b.max.y, b.min.z);
			case 6: return vector3(b.max.x, b.max.y, b.max.z);
			case 7: return vector3(b.min.x, b.max.y, b.max.z);
			default: CE_FATAL("Bad index"); return vector3(0.0f, 0.0f, 0.0f);
		}
	}

	inline AABB transformed(const AABB& b, const Matrix4x4& m)
	{
		Vector3 vertices[8];
		to_vertices(b, vertices);

		vertices[0] = vertices[0] * m;
		vertices[1] = vertices[1] * m;
		vertices[2] = vertices[2] * m;
		vertices[3] = vertices[3] * m;
		vertices[4] = vertices[4] * m;
		vertices[5] = vertices[5] * m;
		vertices[6] = vertices[6] * m;
		vertices[7] = vertices[7] * m;

		AABB res;
		reset(res);
		add_points(res, 8, vertices);
		return res;
	}

	inline void to_vertices(const AABB& b, Vector3 v[8])
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
		v[0].x = b.min.x;
		v[0].y = b.min.y;
		v[0].z = b.max.z;

		v[1].x = b.max.x;
		v[1].y = b.min.y;
		v[1].z = b.max.z;

		v[2].x = b.max.x;
		v[2].y = b.min.y;
		v[2].z = b.min.z;

		v[3].x = b.min.x;
		v[3].y = b.min.y;
		v[3].z = b.min.z;

		v[4].x = b.min.x;
		v[4].y = b.max.y;
		v[4].z = b.max.z;

		v[5].x = b.max.x;
		v[5].y = b.max.y;
		v[5].z = b.max.z;

		v[6].x = b.max.x;
		v[6].y = b.max.y;
		v[6].z = b.min.z;

		v[7].x = b.min.x;
		v[7].y = b.max.y;
		v[7].z = b.min.z;
	}

	inline Sphere to_sphere(const AABB& b)
	{
		Sphere s;
		s.c = center(b);
		s.r = radius(b);
		return s;
	}
} // namespace aabb

} // namespace crown
