/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/math/math.h"
#include "core/math/types.h"
#include "core/types.h"
#include <math.h>

namespace crown
{
inline bool fequal(f32 a, f32 b, f32 epsilon)
{
	return b <= (a + epsilon)
		&& b >= (a - epsilon)
		;
}

inline f32 fsign(f32 a)
{
	return (a > 0.0f) - (a < 0.0f);
}

inline f32 fceil(f32 a)
{
	return ceilf(a);
}

inline f32 ffloor(f32 a)
{
	return floorf(a);
}

inline f32 fround(f32 a)
{
	return ffloor(0.5f + a);
}

inline f32 ffract(f32 a)
{
	return a - ffloor(a);
}

inline f32 fabs(f32 a)
{
	return ::fabsf(a);
}

inline f32 fmod(f32 a, f32 b)
{
	return ::fmod(a, b);
}

inline f32 fsin(f32 a)
{
	return sinf(a);
}

inline f32 fcos(f32 a)
{
	return cosf(a);
}

inline f32 facos(f32 a)
{
	return ::acosf(a);
}

inline f32 ftan(f32 a)
{
	return tanf(a);
}

inline f32 fsqrt(f32 a)
{
	return sqrtf(a);
}

inline f32 fpow(f32 x, f32 y)
{
	return powf(x, y);
}

inline f32 frad(f32 deg)
{
	return deg * PI / 180.0f;
}

inline f32 fdeg(f32 rad)
{
	return rad * 180.0f / PI;
}

inline f32 lerp(const f32 p0, const f32 p1, f32 t)
{
	return (1.0f - t) * p0 + t * p1;
}

} // namespace crown
