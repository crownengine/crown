/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include <math.h>

namespace crown
{
/// @addtogroup Math
/// @{

const f32 PI            = 3.1415926535897932f;
const f32 PI_TWO        = PI * 2.0f;
const f32 PI_HALF       = PI * 0.5f;
const f32 FLOAT_EPSILON = 1.0e-7f;

/// Returns whether @a a and @a b are equal according to @a epsilon.
inline bool fequal(f32 a, f32 b, f32 epsilon = FLOAT_EPSILON)
{
	return b <= (a + epsilon)
		&& b >= (a - epsilon)
		;
}

/// Returns the minimum of @a a and @a b.
inline f32 fmin(f32 a, f32 b)
{
	return a < b ? a : b;
}

/// Returns the maximum of @a a and @a b.
inline f32 fmax(f32 a, f32 b)
{
	return a < b ? b : a;
}

/// Clamps @a val to @a min and @a max.
inline f32 fclamp(f32 min, f32 max, f32 val)
{
	return fmin(fmax(min, val), max);
}

/// Returns the fractional part of @a a.
inline f32 ffract(f32 a)
{
	return a - floorf(a);
}

/// Returns @a deg in radians.
inline f32 frad(f32 deg)
{
	return deg * PI / 180.0f;
}

/// Returns @a rad in degrees.
inline f32 fdeg(f32 rad)
{
	return rad * 180.0f / PI;
}

/// Returns the linear interpolated value between @a p0 and @a p1 at time @a t
inline f32 lerp(const f32 p0, const f32 p1, f32 t)
{
	return (1.0f - t) * p0 + t * p1;
}

/// Returns the cosine interpolated value between @a p0 and @a p1 at time @a t
inline f32 cosine(const f32 p0, const f32 p1, f32 t)
{
	const f32 f = t * PI;
	const f32 g = (1.0f - cosf(f)) * 0.5f;

	return p0 + g * (p1 - p0);
}

/// Returns the cubic interpolated value between @a p0 and @a p1 at time @a t
inline f32 cubic(const f32 p0, const f32 p1, f32 t)
{
	const f32 tt  = t * t;
	const f32 ttt = tt * t;

	return p0 * (2.0f * ttt - 3.0f * tt + 1.0f) + p1 * (3.0f * tt  - 2.0f * ttt);
}

/// Bezier interpolation
inline f32 bezier(const f32 p0, const f32 p1, const f32 p2, const f32 p3, f32 t)
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

/// Catmull-Rom interpolation
inline f32 catmull_rom(const f32 p0, const f32 p1, const f32 p2, const f32 p3, f32 t)
{
	const f32 tt  = t * t;
	const f32 ttt = tt * t;

	const f32 tmp = (2.0f * p1)
		+ (-p0 + p2) * t
		+ ((2.0f * p0) - (5.0f * p1) + (4.0f * p2) - p3) * tt
		+ (-p0 + (3.0f * p1) + (-3.0f * p2) + p3) * ttt;

	return tmp * 0.5f;
}

/// @}
} // namespace crown
