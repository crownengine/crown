/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/math/math.h"
#include "core/math/types.h"
#include "core/types.h"
#include <math.h>
#include <stdio.h> // snprintf

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

const char* to_string(const Vector3& v, char* buf, u32 buf_len)
{
	snprintf(buf, buf_len, "( %.4f, %.4f, %.4f )", v.x, v.y, v.z);
	return buf;
}

const char* to_string(const Vector4& v, char* buf, u32 buf_len)
{
	snprintf(buf, buf_len, "( %.4f, %.4f, %.4f, %.4f )", v.x, v.y, v.z, v.w);
	return buf;
}

const char* to_string(const Quaternion& q, char* buf, u32 buf_len)
{
	snprintf(buf, buf_len, "( %.4f, %.4f, %.4f, %.4f )", q.x, q.y, q.z, q.w);
	return buf;
}

const char* to_string(const Matrix4x4& m, char* buf, u32 buf_len)
{
	char bufx[256];
	char bufy[256];
	char bufz[256];
	char bufw[256];
	snprintf(buf, buf_len,
		"( %s, %s, %s, %s )"
		, to_string(m.x, bufx, sizeof(bufx))
		, to_string(m.y, bufy, sizeof(bufy))
		, to_string(m.z, bufz, sizeof(bufz))
		, to_string(m.t, bufw, sizeof(bufw))
		);
	return buf;
}

} // namespace crown
