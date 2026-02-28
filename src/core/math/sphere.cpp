/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/math/constants.h"
#include "core/math/sphere.inl"

namespace crown
{
namespace sphere
{
	void reset(Sphere &s)
	{
		s.c = VECTOR3_ZERO;
		s.r = 0.0f;
	}

	Sphere from_cone(const Vector3 &tip, const Vector3 &dir, f32 range, f32 angle)
	{
		const f32 sin_a = fsin(angle);
		const f32 cos_a = fcos(angle);
		const f32 slant = range / cos_a;
		const f32 sotca = slant / (2.0f * cos_a);
		Sphere s;

		if (angle > PI_FOURTH) {
			s.c = tip + dir * range;
			s.r = sin_a * slant;
		} else {
			s.c = tip + dir * sotca;
			s.r = sotca;
		}

		return s;
	}

	// Reference:
	// https://web.archive.org/web/20170808221957/https://sharif.edu/~zarrabi/papers/cccg-06/meb.pdf
	void add_points(Sphere &s, u32 num, u32 stride, const void *points)
	{
		const char *pts = (char *)points;

		if (CE_UNLIKELY(num == 0))
			return;

		s.c = *(Vector3 *)pts;
		s.r = 0.0f;
		pts += stride;

		for (u32 i = 1; i < num; ++i, pts += stride) {
			const Vector3 &p = *(Vector3 *)pts;

			if (!sphere::contains_point(s, p)) {
				const f32 dist  = distance(p, s.c);
				const f32 delta = 0.5f * (dist - s.r);
				const f32 alpha = delta / dist;

				s.c += alpha * (p - s.c);
				s.r += delta + FLOAT_EPSILON * 10.0f;
			}
		}
	}

} // namespace sphere

} // namespace crown
