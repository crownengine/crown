/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
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

	/// Returns the @a vertices of the frustum @a f.
	/// @note
	/// 0 = Near bottom left
	/// 1 = Near bottom right
	/// 2 = Near top right
	/// 3 = Near top left
	/// 4 = Far bottom left
	/// 5 = Far bottom right
	/// 6 = Far top right
	/// 7 = Far top left
	void vertices(Vector3 vertices[8], const Frustum& f);

	/// Returns the AABB enclosing the frustum @a f.
	AABB to_aabb(const Frustum& f);

} // namespace frustum

namespace frustum
{
	inline void from_matrix(Frustum& f, const Matrix4x4& m)
	{
		f.planes[0].n.x = m.x.w + m.x.y;
		f.planes[0].n.y = m.y.w + m.y.y;
		f.planes[0].n.z = m.z.w + m.z.y;
		f.planes[0].d = -(m.t.w + m.t.y);
		plane3::normalize(f.planes[0]);

		f.planes[1].n.x = m.x.w - m.x.x;
		f.planes[1].n.y = m.y.w - m.y.x;
		f.planes[1].n.z = m.z.w - m.z.x;
		f.planes[1].d = -(m.t.w - m.t.x);
		plane3::normalize(f.planes[1]);

		f.planes[2].n.x = m.x.w - m.x.y;
		f.planes[2].n.y = m.y.w - m.y.y;
		f.planes[2].n.z = m.z.w - m.z.y;
		f.planes[2].d = -(m.t.w - m.t.y);
		plane3::normalize(f.planes[2]);

		f.planes[3].n.x = m.x.w + m.x.x;
		f.planes[3].n.y = m.y.w + m.y.x;
		f.planes[3].n.z = m.z.w + m.z.x;
		f.planes[3].d = -(m.t.w + m.t.x);
		plane3::normalize(f.planes[3]);

		f.planes[4].n.x = m.x.z;
		f.planes[4].n.y = m.y.z;
		f.planes[4].n.z = m.z.z;
		f.planes[4].d = -(m.t.z);
		plane3::normalize(f.planes[4]);

		f.planes[5].n.x = m.x.w - m.x.z;
		f.planes[5].n.y = m.y.w - m.y.z;
		f.planes[5].n.z = m.z.w - m.z.z;
		f.planes[5].d = -(m.t.w - m.t.z);
		plane3::normalize(f.planes[5]);
	}

	inline bool contains_point(const Frustum& f, const Vector3& p)
	{
		for (u32 ii = 0; ii < 6; ++ii)
		{
			if (plane3::distance_to_point(f.planes[ii], p) < 0.0f)
				return false;
		}

		return true;
	}

	inline void vertices(Vector3 vertices[8], const Frustum& f)
	{
		// p3 ---- p2  Front face.
		//  |      |
		//  |      |
		// p0 ---- p1
		plane_3_intersection(vertices[0], f.planes[4], f.planes[0], f.planes[3]);
		plane_3_intersection(vertices[1], f.planes[4], f.planes[0], f.planes[1]);
		plane_3_intersection(vertices[2], f.planes[4], f.planes[2], f.planes[1]);
		plane_3_intersection(vertices[3], f.planes[4], f.planes[2], f.planes[3]);

		// p7 ---- p6  Back face.
		//  |      |
		//  |      |
		// p4 ---- p5
		plane_3_intersection(vertices[4], f.planes[5], f.planes[0], f.planes[3]);
		plane_3_intersection(vertices[5], f.planes[5], f.planes[0], f.planes[1]);
		plane_3_intersection(vertices[6], f.planes[5], f.planes[2], f.planes[1]);
		plane_3_intersection(vertices[7], f.planes[5], f.planes[2], f.planes[3]);
	}

	inline AABB to_aabb(const Frustum& f)
	{
		Vector3 vertices[8];
		frustum::vertices(vertices, f);

		AABB r;
		aabb::from_points(r, countof(vertices), vertices);
		return r;
	}

} // namespace frustum

} // namespace crown
