/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/math/aabb.h"

namespace crown
{
namespace aabb
{
	void from_points(AABB& b, u32 num, u32 stride, const void* points)
	{
		const char* pts = (const char*)points;
		const f32* point = (f32*)pts;

		b.min.x = b.max.x = point[0];
		b.min.y = b.max.y = point[1];
		b.min.z = b.max.z = point[2];
		pts += stride;

		for (u32 i = 1; i < num; ++i, pts += stride)
		{
			point = (f32*)pts;

			b.min.x = min(b.min.x, point[0]);
			b.min.y = min(b.min.y, point[1]);
			b.min.z = min(b.min.z, point[2]);
			b.max.x = max(b.max.x, point[0]);
			b.max.y = max(b.max.y, point[1]);
			b.max.z = max(b.max.z, point[2]);
		}
	}

	void from_boxes(AABB& b, u32 num, const AABB* boxes)
	{
		b.min.x = boxes[0].min.x;
		b.min.y = boxes[0].min.y;
		b.min.z = boxes[0].min.z;
		b.max.x = boxes[0].max.x;
		b.max.y = boxes[0].max.y;
		b.max.z = boxes[0].max.z;

		for (u32 i = 1; i < num; ++i)
		{
			b.min.x = min(b.min.x, boxes[i].min.x);
			b.min.y = min(b.min.y, boxes[i].min.y);
			b.min.z = min(b.min.z, boxes[i].min.z);
			b.max.x = max(b.max.x, boxes[i].max.x);
			b.max.y = max(b.max.y, boxes[i].max.y);
			b.max.z = max(b.max.z, boxes[i].max.z);
		}
	}

} // namespace aabb

} // namespace crown
