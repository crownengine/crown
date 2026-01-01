/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/error/error.inl"
#include "core/math/aabb.inl"
#include "core/math/constants.h"

namespace crown
{
namespace aabb
{
	void reset(AABB &b)
	{
		b.min = VECTOR3_ZERO;
		b.max = VECTOR3_ZERO;
	}

	void from_points(AABB &b, u32 num, u32 stride, const void *points)
	{
		CE_ENSURE(num > 0);
		CE_ENSURE(points != NULL);

		const char *pts = (char *)points;
		const f32 *point = (f32 *)pts;

		b.min.x = b.max.x = point[0];
		b.min.y = b.max.y = point[1];
		b.min.z = b.max.z = point[2];
		pts += stride;

		for (u32 i = 1; i < num; ++i, pts += stride) {
			point = (f32 *)pts;

			b.min.x = min(b.min.x, point[0]);
			b.min.y = min(b.min.y, point[1]);
			b.min.z = min(b.min.z, point[2]);
			b.max.x = max(b.max.x, point[0]);
			b.max.y = max(b.max.y, point[1]);
			b.max.z = max(b.max.z, point[2]);
		}
	}

	void from_boxes(AABB &b, u32 num, const AABB *boxes)
	{
		CE_ENSURE(num > 0);
		CE_ENSURE(boxes != NULL);

		b.min.x = boxes[0].min.x;
		b.min.y = boxes[0].min.y;
		b.min.z = boxes[0].min.z;
		b.max.x = boxes[0].max.x;
		b.max.y = boxes[0].max.y;
		b.max.z = boxes[0].max.z;

		for (u32 i = 1; i < num; ++i) {
			b.min.x = min(b.min.x, boxes[i].min.x);
			b.min.y = min(b.min.y, boxes[i].min.y);
			b.min.z = min(b.min.z, boxes[i].min.z);
			b.max.x = max(b.max.x, boxes[i].max.x);
			b.max.y = max(b.max.y, boxes[i].max.y);
			b.max.z = max(b.max.z, boxes[i].max.z);
		}
	}

	Vector3 vertex(const AABB &b, u32 index)
	{
		CE_ASSERT(index < 8, "Index out of bounds");

		switch (index) {
		case 0: return { b.min.x, b.min.y, b.min.z };
		case 1: return { b.max.x, b.min.y, b.min.z };
		case 2: return { b.max.x, b.min.y, b.max.z };
		case 3: return { b.min.x, b.min.y, b.max.z };
		case 4: return { b.min.x, b.max.y, b.min.z };
		case 5: return { b.max.x, b.max.y, b.min.z };
		case 6: return { b.max.x, b.max.y, b.max.z };
		case 7: return { b.min.x, b.max.y, b.max.z };
		default: return VECTOR3_ZERO;
		}
	}

} // namespace aabb

} // namespace crown
