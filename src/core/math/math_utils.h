/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "error.h"
#include "types.h"
#include <math.h>

namespace crown
{

/// @addtogroup Math
/// @{
const float PI                = 3.1415926535897932f;
const float TWO_PI            = PI * 2.0f;
const float HALF_PI           = PI * 0.5f;
const float FLOAT_PRECISION   = 1.0e-7f;

inline bool fequal(float a, float b, float precision = FLOAT_PRECISION)
{
	return ((b <= (a + precision)) && (b >= (a - precision)));
}

template <typename T>
inline T min(const T& a, const T& b)
{
	return a < b ? a : b;
}

template <typename T>
inline T max(const T& a, const T& b)
{
	return a < b ? b : a;
}

template <typename T>
inline T clamp(const T& min, const T& max, const T& val)
{
	CE_ASSERT(min < max, "Min must be < max");
	return val > max ? max : val < min ? min : val;
}

inline float to_rad(float deg)
{
	return deg * PI / 180.0f;
}

inline float to_deg(float rad)
{
	return rad * 180.0f / PI;
}

inline uint32_t next_pow_2(uint32_t x)
{
	x--;

	x = (x >> 1) | x;
	x = (x >> 2) | x;
	x = (x >> 4) | x;
	x = (x >> 8) | x;
	x = (x >> 16) | x;

	return ++x;
}

inline bool is_pow_2(uint32_t x)
{
	return !(x & (x - 1)) && x;
}

inline float ceil(float x)
{
	return ceilf(x);
}

inline float floor(float x)
{
	return floorf(x);
}

inline float sqrt(float x)
{
	return sqrtf(x);
}

inline float inv_sqrt(float x)
{
	return 1.0f / sqrt(x);
}

inline float sin(float x)
{
	return sinf(x);
}

inline float cos(float x)
{
	return cosf(x);
}

inline float asin(float x)
{
	return asinf(x);
}

inline float acos(float x)
{
	return acosf(x);
}

inline float tan(float x)
{
	return tanf(x);
}

inline float atan2(float y, float x)
{
	return atan2f(y, x);
}

inline float abs(float x)
{
	return fabs(x);
}

inline float fmod(float n, float d)
{
	return ::fmod(n, d);
}

/// Returns the linear interpolated value between @a p0 and @a p1 at time @a t
template <typename T>
inline T linear(const T& p0, const T& p1, float t)
{
	return p0 + (t * (p1 - p0));
}

/// Returns the cosine interpolated value between @a p0 and @a p1 at time @a t
template <typename T>
inline T cosine(const T& p0, const T& p1, float t)
{
	const float f = t * PI;
	const float g = (1.0f - cos(f)) * 0.5f;

	return p0 + (g * (p1 - p0));
}

/// Returns the cubic interpolated value between @a p0 and @a p1 at time @a t
template <typename T>
inline T cubic(const T& p0, const T& p1, float t)
{
	const float tt = t * t;
	const float ttt = tt * t;

	return p0 * (2.0f * ttt - 3.0f * tt + 1.0f) + p1 * (3.0f * tt  - 2.0f * ttt);
}

/// Bezier interpolation
template <typename T>
inline T bezier(const T& p0, const T& p1, const T& p2, const T& p3, float t)
{
	const float u = 1.0f - t;
	const float tt = t * t ;
	const float uu = u * u;
	const float uuu = uu * u;
	const float ttt = tt * t;

	T tmp = (uuu * p0) +
			(3.0f * uu * t * p1) +
			(3.0f * u * tt * p2) +
			(ttt * p3);

	return tmp;
}

/// Catmull-Rom interpolation
template <typename T>
inline T catmull_rom(const T& p0, const T& p1, const T& p2, const T& p3, float t)
{
	const float tt = t * t;
	const float ttt = tt * t;

	T tmp = (2.0f * p1) +
			((-p0 + p2) * t) +
			(((2.0f * p0) - (5.0f * p1) + (4.0f * p2) - p3) * tt) +
			((-p0 + (3.0f * p1) + (-3.0f * p2) + p3) * ttt);

	return tmp * 0.5f;
}

/// @}

} // namespace crown
