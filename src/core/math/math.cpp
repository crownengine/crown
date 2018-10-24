/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/math/math.h"
#include "core/types.h"
#include <math.h>

namespace crown
{
bool fequal(f32 a, f32 b, f32 epsilon)
{
	return b <= (a + epsilon)
		&& b >= (a - epsilon)
		;
}

f32 ffract(f32 a)
{
	return a - floorf(a);
}

f32 fabs(f32 a)
{
	return ::fabsf(a);
}

f32 fsin(f32 a)
{
	return sinf(a);
}

f32 fcos(f32 a)
{
	return cosf(a);
}

f32 facos(f32 a)
{
	return ::acosf(a);
}

f32 ftan(f32 a)
{
	return tanf(a);
}

f32 fsqrt(f32 a)
{
	return sqrtf(a);
}

f32 frad(f32 deg)
{
	return deg * PI / 180.0f;
}

f32 fdeg(f32 rad)
{
	return rad * 180.0f / PI;
}

f32 lerp(const f32 p0, const f32 p1, f32 t)
{
	return (1.0f - t) * p0 + t * p1;
}

f32 cosine(const f32 p0, const f32 p1, f32 t)
{
	const f32 f = t * PI;
	const f32 g = (1.0f - fcos(f)) * 0.5f;

	return p0 + g * (p1 - p0);
}

f32 cubic(const f32 p0, const f32 p1, f32 t)
{
	const f32 tt  = t * t;
	const f32 ttt = tt * t;

	return p0 * (2.0f * ttt - 3.0f * tt + 1.0f) + p1 * (3.0f * tt  - 2.0f * ttt);
}

f32 bezier(const f32 p0, const f32 p1, const f32 p2, const f32 p3, f32 t)
{
	const f32 u   = 1.0f - t;
	const f32 tt  = t * t ;
	const f32 uu  = u * u;
	const f32 uuu = uu * u;
	const f32 ttt = tt * t;

	const f32 tmp = (uuu * p0)
		+ (3.0f * uu * t * p1)
		+ (3.0f * u * tt * p2)
		+ (ttt * p3);

	return tmp;
}

f32 catmull_rom(const f32 p0, const f32 p1, const f32 p2, const f32 p3, f32 t)
{
	const f32 tt  = t * t;
	const f32 ttt = tt * t;

	const f32 tmp = (2.0f * p1)
		+ (-p0 + p2) * t
		+ ((2.0f * p0) - (5.0f * p1) + (4.0f * p2) - p3) * tt
		+ (-p0 + (3.0f * p1) + (-3.0f * p2) + p3) * ttt;

	return tmp * 0.5f;
}

} // namespace crown
