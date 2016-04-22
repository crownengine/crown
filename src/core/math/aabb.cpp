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
			const Vector3& pi = *(const Vector3*)pts;

			b.min.x = fmin(b.min.x, pi.x);
			b.min.y = fmin(b.min.y, pi.y);
			b.min.z = fmin(b.min.z, pi.z);
			b.max.x = fmax(b.max.x, pi.x);
			b.max.y = fmax(b.max.y, pi.y);
			b.max.z = fmax(b.max.z, pi.z);
		}
	}

	void add_boxes(AABB& b, u32 num, const AABB* boxes)
	{
		for (u32 i = 0; i < num; ++i)
		{
			const AABB& bi = boxes[i];

			b.min.x = fmin(b.min.x, bi.min.x);
			b.min.y = fmin(b.min.y, bi.min.y);
			b.min.z = fmin(b.min.z, bi.min.z);
			b.max.x = fmax(b.max.x, bi.max.x);
			b.max.y = fmax(b.max.y, bi.max.y);
			b.max.z = fmax(b.max.z, bi.max.z);
		}
	}
} // namespace aabb

} // namespace crown
