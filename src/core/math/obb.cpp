/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/math/matrix4x4.inl"
#include "core/math/obb.h"
#include "core/math/vector3.inl"

namespace crown
{
namespace obb
{
	OBB merge(const OBB &a, const OBB &b)
	{
		Vector3 a_pos = translation(a.tm);
		Vector3 a_scl  = scale(a.tm);
		Vector3 a_x = x(a.tm);
		Vector3 a_y = y(a.tm);
		Vector3 a_z = z(a.tm);
		normalize(a_x);
		normalize(a_y);
		normalize(a_z);
		Vector3 a_world_half;
		a_world_half.x = a.half_extents.x * a_scl.x;
		a_world_half.y = a.half_extents.y * a_scl.y;
		a_world_half.z = a.half_extents.z * a_scl.z;
		Vector3 a_min = -a_world_half;
		Vector3 a_max =  a_world_half;

		Vector3 b_pos = translation(b.tm);
		Vector3 b_scl = scale(b.tm);
		Vector3 b_x = x(b.tm);
		Vector3 b_y = y(b.tm);
		Vector3 b_z = z(b.tm);
		normalize(b_x);
		normalize(b_y);
		normalize(b_z);
		Vector3 b_world_half;
		b_world_half.x = b.half_extents.x * b_scl.x;
		b_world_half.y = b.half_extents.y * b_scl.y;
		b_world_half.z = b.half_extents.z * b_scl.z;

		// Project b's extents onto a's local axes.
		Vector3 r;
		r.x = fabs(dot(b_x * b_world_half.x, a_x))
			+ fabs(dot(b_y * b_world_half.y, a_x))
			+ fabs(dot(b_z * b_world_half.z, a_x))
			;
		r.y = fabs(dot(b_x * b_world_half.x, a_y))
			+ fabs(dot(b_y * b_world_half.y, a_y))
			+ fabs(dot(b_z * b_world_half.z, a_y))
			;
		r.z = fabs(dot(b_x * b_world_half.x, a_z))
			+ fabs(dot(b_y * b_world_half.y, a_z))
			+ fabs(dot(b_z * b_world_half.z, a_z))
			;

		// Transform b's extents to a's local space.
		Vector3 delta = b_pos - a_pos;
		Vector3 b_local_pos;
		b_local_pos.x = dot(delta, a_x);
		b_local_pos.y = dot(delta, a_y);
		b_local_pos.z = dot(delta, a_z);
		Vector3 b_min = b_local_pos - r;
		Vector3 b_max = b_local_pos + r;

		// Merge the intervals along each axis.
		Vector3 ab_min = min(a_min, b_min);
		Vector3 ab_max = max(a_max, b_max);

		Vector3 ab_pos = (ab_min + ab_max) * 0.5f;
		Vector3 ab_world_half = (ab_max - ab_min) * 0.5f;

		// Transform merged pos back to world space.
		Vector3 ab_world_pos = a_pos
			+ a_x * ab_pos.x
			+ a_y * ab_pos.y
			+ a_z * ab_pos.z
			;

		// Transform merged extents back into a's local space.
		Vector3 ab_half;
		ab_half.x = ab_world_half.x / a_scl.x;
		ab_half.y = ab_world_half.y / a_scl.y;
		ab_half.z = ab_world_half.z / a_scl.z;

		OBB o;
		o.tm = a.tm;
		set_translation(o.tm, ab_world_pos);
		o.half_extents = ab_half;
		return o;
	}

} // namespace obb

} // namespace crown
