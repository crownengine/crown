/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
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
	/// Builds the frustum @a f from the matrix @a m.
	void from_matrix(Frustum &f, const Matrix4x4 &m, bool homogeneous_ndc = false, int handedness = 1);

	/// Returns whether the frustum @a f contains the point @a p.
	bool contains_point(const Frustum &f, const Vector3 &p);

	/// Splits the frustum @a f into @a num sub-frustum @a splits.
	/// @a weight controls splits distribution: 0 means evenly-spaced
	/// splits; higher values push splits towards the original near plane.
	/// @a overlap sets how much a given far plane should overlap the next
	/// split's near plane.
	void split(Frustum *splits, u32 num, const Frustum &f, f32 weight = 0.75f, f32 overlap = 1.005f);

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
	void vertices(Vector3 vertices[8], const Frustum &f);

	/// Returns the AABB enclosing the frustum @a f.
	AABB to_aabb(const Frustum &f);

} // namespace frustum

namespace frustum
{
	inline bool contains_point(const Frustum &f, const Vector3 &p)
	{
		for (u32 ii = 0; ii < 6; ++ii) {
			if (plane3::distance_to_point(f.planes[ii], p) < 0.0f)
				return false;
		}

		return true;
	}

	inline void transform(Frustum &out, const Frustum &f, const Matrix4x4 &m)
	{
		out.planes[0] = plane3::transform(f.planes[0], m);
		out.planes[1] = plane3::transform(f.planes[1], m);
		out.planes[2] = plane3::transform(f.planes[2], m);
		out.planes[3] = plane3::transform(f.planes[3], m);
		out.planes[4] = plane3::transform(f.planes[4], m);
		out.planes[5] = plane3::transform(f.planes[5], m);
	}

	inline void vertices(Vector3 vertices[8], const Frustum &f)
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

	inline AABB to_aabb(const Frustum &f)
	{
		Vector3 verts[8];
		frustum::vertices(verts, f);

		AABB r;
		aabb::from_points(r, countof(verts), verts);
		return r;
	}

} // namespace frustum

} // namespace crown
