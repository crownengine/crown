/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/math/math.inl"
#include <stb_sprintf.h>

namespace crown
{
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

	return p0 * (2.0f * ttt - 3.0f * tt + 1.0f) + p1 * (3.0f * tt - 2.0f * ttt);
}

f32 bezier(const f32 p0, const f32 p1, const f32 p2, const f32 p3, f32 t)
{
	const f32 u   = 1.0f - t;
	const f32 tt  = t * t;
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

const char *to_string(char *buf, u32 buf_len, const Vector3 &v)
{
	stbsp_snprintf(buf, buf_len, "( %.4f, %.4f, %.4f )", v.x, v.y, v.z);
	return buf;
}

const char *to_string(char *buf, u32 buf_len, const Vector4 &v)
{
	stbsp_snprintf(buf, buf_len, "( %.4f, %.4f, %.4f, %.4f )", v.x, v.y, v.z, v.w);
	return buf;
}

const char *to_string(char *buf, u32 buf_len, const Quaternion &q)
{
	stbsp_snprintf(buf, buf_len, "( %.4f, %.4f, %.4f, %.4f )", q.x, q.y, q.z, q.w);
	return buf;
}

const char *to_string(char *buf, u32 buf_len, const Matrix4x4 &m)
{
	char bufx[256];
	char bufy[256];
	char bufz[256];
	char bufw[256];
	stbsp_snprintf(buf, buf_len,
		"( %s, %s, %s, %s )"
		, to_string(bufx, sizeof(bufx), m.x)
		, to_string(bufy, sizeof(bufy), m.y)
		, to_string(bufz, sizeof(bufz), m.z)
		, to_string(bufw, sizeof(bufw), m.t)
		);
	return buf;
}

} // namespace crown
