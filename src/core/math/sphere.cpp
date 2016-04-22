/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "sphere.h"

namespace crown
{
namespace sphere
{
	void add_points(Sphere& s, u32 num, u32 stride, const void* points)
	{
		const char* pts = (const char*)points;
		for (u32 i = 0; i < num; ++i, pts += stride)
		{
			const Vector3* p = (const Vector3*)pts;

			const f32 dist = length_squared(*p - s.c);
			if (dist > s.r*s.r)
				s.r = sqrtf(dist);
		}
	}

	void add_spheres(Sphere& s, u32 num, const Sphere* spheres)
	{
		for (u32 i = 0; i < num; ++i)
		{
			const Sphere si = spheres[i];
			const f32 dist = length_squared(si.c - s.c);

			if (dist < (si.r + s.r) * (si.r + s.r))
			{
				if (si.r*si.r > s.r*s.r)
					s.r = sqrtf(dist + si.r*si.r);
			}
		}
	}
} // namespace sphere

} // namespace crown
