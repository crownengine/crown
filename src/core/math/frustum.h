/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "math_types.h"
#include "plane.h"
#include "aabb.h"
#include "intersection.h"

namespace crown
{

/// Functions to manipulate Frustum.
///
/// @ingroup Math
namespace frustum
{
	/// Builds the frustum @a f from the view matrix @a m.
	void from_matrix(Frustum& f, const Matrix4x4& m);

	/// Returns whether the frustum @a f contains the point @a p.
	bool contains_point(const Frustum& f, const Vector3& p);

	/// Returns the corner @a index of the frustum @a f.
	/// @note
	/// Index to corner table:
	/// 0 = Near bottom left
	/// 1 = Near bottom right
	/// 2 = Near top right
	/// 3 = Near top left
	/// 4 = Far bottom left
	/// 5 = Far bottom right
	/// 6 = Far top right
	/// 7 = Far top left
	Vector3 vertex(const Frustum& f, uint32_t index);

	/// Returns the AABB enclosing the frustum @a f.
	AABB to_aabb(const Frustum& f);
} // namespace frustum

namespace frustum
{
	inline void from_matrix(Frustum& f, const Matrix4x4& m)
	{
		f.left.n.x   = m.x.w + m.x.x;
		f.left.n.y   = m.y.w + m.y.x;
		f.left.n.z   = m.z.w + m.z.x;
		f.left.d     = m.t.w + m.t.x;

		f.right.n.x  = m.x.w - m.x.x;
		f.right.n.y  = m.y.w - m.y.x;
		f.right.n.z  = m.z.w - m.z.x;
		f.right.d    = m.t.w - m.t.x;

		f.bottom.n.x = m.x.w + m.x.y;
		f.bottom.n.y = m.y.w + m.y.y;
		f.bottom.n.z = m.z.w + m.z.y;
		f.bottom.d   = m.t.w + m.t.y;

		f.top.n.x    = m.x.w - m.x.y;
		f.top.n.y    = m.y.w - m.y.y;
		f.top.n.z    = m.z.w - m.z.y;
		f.top.d      = m.t.w - m.t.y;

		f.near.n.x   = m.x.w + m.x.z;
		f.near.n.y   = m.y.w + m.y.z;
		f.near.n.z   = m.z.w + m.z.z;
		f.near.d     = m.t.w + m.t.z;

		f.far.n.x    = m.x.w - m.x.z;
		f.far.n.y    = m.y.w - m.y.z;
		f.far.n.z    = m.z.w - m.z.z;
		f.far.d      = m.t.w - m.t.z;

		plane::normalize(f.left);
		plane::normalize(f.right);
		plane::normalize(f.bottom);
		plane::normalize(f.top);
		plane::normalize(f.near);
		plane::normalize(f.far);
	}

	inline bool contains_point(const Frustum& f, const Vector3& p)
	{
		return !(plane::distance_to_point(f.left, p) < 0.0f
			|| plane::distance_to_point(f.right, p) < 0.0f
			|| plane::distance_to_point(f.bottom, p) < 0.0f
			|| plane::distance_to_point(f.top, p) < 0.0f
			|| plane::distance_to_point(f.near, p) < 0.0f
			|| plane::distance_to_point(f.far, p) < 0.0f
			);
	}

	inline Vector3 vertex(const Frustum& f, uint32_t index)
	{
		CE_ASSERT(index < 8, "Index out of bounds");

		// 0 = Near bottom left
		// 1 = Near bottom right
		// 2 = Near top right
		// 3 = Near top left
		// 4 = Far bottom left
		// 5 = Far bottom right
		// 6 = Far top right
		// 7 = Far top left

		const Plane* side = &f.left;
		Vector3 ip;

		switch (index)
		{
			case 0: plane_3_intersection(side[4], side[0], side[2], ip); break;
			case 1: plane_3_intersection(side[4], side[1], side[2], ip); break;
			case 2: plane_3_intersection(side[4], side[1], side[3], ip); break;
			case 3: plane_3_intersection(side[4], side[0], side[3], ip); break;
			case 4: plane_3_intersection(side[5], side[0], side[2], ip); break;
			case 5: plane_3_intersection(side[5], side[1], side[2], ip); break;
			case 6: plane_3_intersection(side[5], side[1], side[3], ip); break;
			case 7: plane_3_intersection(side[5], side[0], side[3], ip); break;
			default: break;
		}

		return ip;
	}

	inline AABB to_aabb(const Frustum& f)
	{
		Vector3 vertices[8];
		vertices[0] = vertex(f, 0);
		vertices[1] = vertex(f, 1);
		vertices[2] = vertex(f, 2);
		vertices[3] = vertex(f, 3);
		vertices[4] = vertex(f, 4);
		vertices[5] = vertex(f, 5);
		vertices[6] = vertex(f, 6);
		vertices[7] = vertex(f, 7);

		AABB r;
		aabb::reset(r);
		aabb::add_points(r, 8, vertices);
		return r;
	}
} // namespace frustum

} // namespace crown
