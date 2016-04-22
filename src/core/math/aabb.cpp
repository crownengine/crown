/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "aabb.h"

namespace crown
{
namespace aabb
{
	void add_points(AABB& b, u32 num, u32 stride, const void* points)
	{
		const char* pts = (const char*)points;
		for (u32 i = 0; i < num; ++i, pts += stride)
		{
			const Vector3* p = (const Vector3*)pts;

			if (p->x < b.min.x) b.min.x = p->x;
			if (p->y < b.min.y) b.min.y = p->y;
			if (p->z < b.min.z) b.min.z = p->z;
			if (p->x > b.max.x) b.max.x = p->x;
			if (p->y > b.max.y) b.max.y = p->y;
			if (p->z > b.max.z) b.max.z = p->z;
		}
	}

	void add_boxes(AABB& b, u32 num, const AABB* boxes)
	{
		for (u32 i = 0; i < num; ++i)
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
} // namespace aabb

} // namespace crown
