/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include <math.h>

namespace crown
{

/// @addtogroup Math
/// @{
const float PI              = 3.1415926535897932f;
const float TWO_PI          = PI * 2.0f;
const float HALF_PI         = PI * 0.5f;
const float FLOAT_PRECISION = 1.0e-7f;

inline bool fequal(float a, float b, float precision = FLOAT_PRECISION)
{
	return ((b <= (a + precision)) && (b >= (a - precision)));
}

inline float fmin(float a, float b)
{
	return a < b ? a : b;
}

inline float fmax(float a, float b)
{
	return a < b ? b : a;
}

inline float fclamp(float min, float max, float val)
{
	return fmin(fmax(min, val), max);
}

inline float to_rad(float deg)
{
	return deg * PI / 180.0f;
}

inline float to_deg(float rad)
{
	return rad * 180.0f / PI;
}

/// Returns the linear interpolated value between @a p0 and @a p1 at time @a t
inline float lerp(const float p0, const float p1, float t)
{
	return (1.0f - t) * p0 + t * p1;
}

/// Returns the cosine interpolated value between @a p0 and @a p1 at time @a t
inline float cosine(const float p0, const float p1, float t)
{
	const float f = t * PI;
	const float g = (1.0f - cosf(f)) * 0.5f;

	return p0 + g * (p1 - p0);
}

/// Returns the cubic interpolated value between @a p0 and @a p1 at time @a t
inline float cubic(const float p0, const float p1, float t)
{
	const float tt  = t * t;
	const float ttt = tt * t;

	return p0 * (2.0f * ttt - 3.0f * tt + 1.0f) + p1 * (3.0f * tt  - 2.0f * ttt);
}

/// Bezier interpolation
inline float bezier(const float p0, const float p1, const float p2, const float p3, float t)
{
	const float u   = 1.0f - t;
	const float tt  = t * t ;
	const float uu  = u * u;
	const float uuu = uu * u;
	const float ttt = tt * t;

	const float tmp = (uuu * p0)
		+ (3.0f * uu * t * p1)
		+ (3.0f * u * tt * p2)
		+ (ttt * p3);

	return tmp;
}

/// Catmull-Rom interpolation
inline float catmull_rom(const float p0, const float p1, const float p2, const float p3, float t)
{
	const float tt  = t * t;
	const float ttt = tt * t;

	const float tmp = (2.0f * p1)
		+ (-p0 + p2) * t
		+ ((2.0f * p0) - (5.0f * p1) + (4.0f * p2) - p3) * tt
		+ (-p0 + (3.0f * p1) + (-3.0f * p2) + p3) * ttt;

	return tmp * 0.5f;
}

/// @}

} // namespace crown
