/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/math/constants.h"
#include "core/math/sphere.inl"

namespace crown
{
namespace sphere
{
	void reset(Sphere& s)
	{
		s.c = VECTOR3_ZERO;
		s.r = 0.0f;
	}

	void add_points(Sphere& s, u32 num, u32 stride, const void* points)
	{
		float rr = s.r*s.r;

		const char* pts = (const char*)points;
		for (u32 i = 0; i < num; ++i, pts += stride)
		{
			const Vector3& pi = *(const Vector3*)pts;

			rr = max(rr, length_squared(pi - s.c));
		}

		s.r = fsqrt(rr);
	}

	void add_spheres(Sphere& s, u32 num, const Sphere* spheres)
	{
		for (u32 i = 0; i < num; ++i)
		{
			const Sphere& si = spheres[i];
			const f32 dist = length_squared(si.c - s.c);

			if (dist < (si.r + s.r) * (si.r + s.r))
			{
				if (si.r*si.r > s.r*s.r)
					s.r = fsqrt(dist + si.r*si.r);
			}
		}
	}

} // namespace sphere

} // namespace crown
