/*
 * Copyright (c) 2012-2020 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/math/aabb.inl"
#include "core/math/intersection.h"
#include "core/math/plane3.inl"

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
	Vector3 vertex(const Frustum& f, u32 index);

	/// Returns the AABB enclosing the frustum @a f.
	AABB to_aabb(const Frustum& f);

} // namespace frustum

namespace frustum
{
	inline void from_matrix(Frustum& f, const Matrix4x4& m)
	{
		f.plane_left.n.x   = m.x.w + m.x.x;
		f.plane_left.n.y   = m.y.w + m.y.x;
		f.plane_left.n.z   = m.z.w + m.z.x;
		f.plane_left.d     = m.t.w + m.t.x;

		f.plane_right.n.x  = m.x.w - m.x.x;
		f.plane_right.n.y  = m.y.w - m.y.x;
		f.plane_right.n.z  = m.z.w - m.z.x;
		f.plane_right.d    = m.t.w - m.t.x;

		f.plane_bottom.n.x = m.x.w + m.x.y;
		f.plane_bottom.n.y = m.y.w + m.y.y;
		f.plane_bottom.n.z = m.z.w + m.z.y;
		f.plane_bottom.d   = m.t.w + m.t.y;

		f.plane_top.n.x    = m.x.w - m.x.y;
		f.plane_top.n.y    = m.y.w - m.y.y;
		f.plane_top.n.z    = m.z.w - m.z.y;
		f.plane_top.d      = m.t.w - m.t.y;

		f.plane_near.n.x   = m.x.z;
		f.plane_near.n.y   = m.y.z;
		f.plane_near.n.z   = m.z.z;
		f.plane_near.d     = m.t.z;

		f.plane_far.n.x    = m.x.w - m.x.z;
		f.plane_far.n.y    = m.y.w - m.y.z;
		f.plane_far.n.z    = m.z.w - m.z.z;
		f.plane_far.d      = m.t.w - m.t.z;

		plane3::normalize(f.plane_left);
		plane3::normalize(f.plane_right);
		plane3::normalize(f.plane_bottom);
		plane3::normalize(f.plane_top);
		plane3::normalize(f.plane_near);
		plane3::normalize(f.plane_far);
	}

	inline bool contains_point(const Frustum& f, const Vector3& p)
	{
		return !(plane3::distance_to_point(f.plane_left, p) < 0.0f
			|| plane3::distance_to_point(f.plane_right, p) < 0.0f
			|| plane3::distance_to_point(f.plane_bottom, p) < 0.0f
			|| plane3::distance_to_point(f.plane_top, p) < 0.0f
			|| plane3::distance_to_point(f.plane_near, p) < 0.0f
			|| plane3::distance_to_point(f.plane_far, p) < 0.0f
			);
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
		aabb::from_points(r, countof(vertices), vertices);
		return r;
	}

} // namespace frustum

} // namespace crown
