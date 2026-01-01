/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/math/obb.h"
#include "core/math/matrix4x4.inl"

namespace crown
{
namespace obb
{
	inline void reset(OBB &obb)
	{
		obb.tm = MATRIX4X4_IDENTITY;
		obb.half_extents = VECTOR3_ZERO;
	}

	inline Vector3 center(const OBB &obb)
	{
		return translation(obb.tm);
	}

	inline void to_vertices(Vector3 vertices[8], const OBB &obb)
	{
		const Vector3 center = translation(obb.tm);
		const Vector3 axis_x = x(obb.tm) * obb.half_extents.x;
		const Vector3 axis_y = y(obb.tm) * obb.half_extents.y;
		const Vector3 axis_z = z(obb.tm) * obb.half_extents.z;

		// 7 ---- 6
		// |      |
		// |      |  <--- Top face
		// 4 ---- 5
		//
		// 3 ---- 2
		// |      |
		// |      |  <--- Bottom face
		// 0 ---- 1
		vertices[0] = center - axis_x - axis_y - axis_z;
		vertices[1] = center + axis_x - axis_y - axis_z;
		vertices[2] = center + axis_x - axis_y + axis_z;
		vertices[3] = center - axis_x - axis_y + axis_z;

		vertices[4] = center - axis_x + axis_y - axis_z;
		vertices[5] = center + axis_x + axis_y - axis_z;
		vertices[6] = center + axis_x + axis_y + axis_z;
		vertices[7] = center - axis_x + axis_y + axis_z;
	}

} // namespace obb

} // namespace crown
