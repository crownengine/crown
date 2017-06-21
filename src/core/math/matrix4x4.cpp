/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/math/matrix4x4.h"

namespace crown
{
Matrix4x4& invert(Matrix4x4& m)
{
	const f32 xx = m.x.x;
	const f32 xy = m.x.y;
	const f32 xz = m.x.z;
	const f32 xw = m.x.w;
	const f32 yx = m.y.x;
	const f32 yy = m.y.y;
	const f32 yz = m.y.z;
	const f32 yw = m.y.w;
	const f32 zx = m.z.x;
	const f32 zy = m.z.y;
	const f32 zz = m.z.z;
	const f32 zw = m.z.w;
	const f32 tx = m.t.x;
	const f32 ty = m.t.y;
	const f32 tz = m.t.z;
	const f32 tw = m.t.w;

	f32 det = 0.0f;
	det += xx * (yy * (zz*tw - tz*zw) - zy * (yz*tw - tz*yw) + ty * (yz*zw - zz*yw));
	det -= yx * (xy * (zz*tw - tz*zw) - zy * (xz*tw - tz*xw) + ty * (xz*zw - zz*xw));
	det += zx * (xy * (yz*tw - tz*yw) - yy * (xz*tw - tz*xw) + ty * (xz*yw - yz*xw));
	det -= tx * (xy * (yz*zw - zz*yw) - yy * (xz*zw - zz*xw) + zy * (xz*yw - yz*xw));

	const f32 inv_det = 1.0f / det;

	m.x.x = + (yy * (zz*tw - tz*zw) - zy * (yz*tw - tz*yw) + ty * (yz*zw - zz*yw)) * inv_det;
	m.x.y = - (xy * (zz*tw - tz*zw) - zy * (xz*tw - tz*xw) + ty * (xz*zw - zz*xw)) * inv_det;
	m.x.z = + (xy * (yz*tw - tz*yw) - yy * (xz*tw - tz*xw) + ty * (xz*yw - yz*xw)) * inv_det;
	m.x.w = - (xy * (yz*zw - zz*yw) - yy * (xz*zw - zz*xw) + zy * (xz*yw - yz*xw)) * inv_det;

	m.y.x = - (yx * (zz*tw - tz*zw) - zx * (yz*tw - tz*yw) + tx * (yz*zw - zz*yw)) * inv_det;
	m.y.y = + (xx * (zz*tw - tz*zw) - zx * (xz*tw - tz*xw) + tx * (xz*zw - zz*xw)) * inv_det;
	m.y.z = - (xx * (yz*tw - tz*yw) - yx * (xz*tw - tz*xw) + tx * (xz*yw - yz*xw)) * inv_det;
	m.y.w = + (xx * (yz*zw - zz*yw) - yx * (xz*zw - zz*xw) + zx * (xz*yw - yz*xw)) * inv_det;

	m.z.x = + (yx * (zy*tw - ty*zw) - zx * (yy*tw - ty*yw) + tx * (yy*zw - zy*yw)) * inv_det;
	m.z.y = - (xx * (zy*tw - ty*zw) - zx * (xy*tw - ty*xw) + tx * (xy*zw - zy*xw)) * inv_det;
	m.z.z = + (xx * (yy*tw - ty*yw) - yx * (xy*tw - ty*xw) + tx * (xy*yw - yy*xw)) * inv_det;
	m.z.w = - (xx * (yy*zw - zy*yw) - yx * (xy*zw - zy*xw) + zx * (xy*yw - yy*xw)) * inv_det;

	m.t.x = - (yx * (zy*tz - ty*zz) - zx * (yy*tz - ty*yz) + tx * (yy*zz - zy*yz)) * inv_det;
	m.t.y = + (xx * (zy*tz - ty*zz) - zx * (xy*tz - ty*xz) + tx * (xy*zz - zy*xz)) * inv_det;
	m.t.z = - (xx * (yy*tz - ty*yz) - yx * (xy*tz - ty*xz) + tx * (xy*yz - yy*xz)) * inv_det;
	m.t.w = + (xx * (yy*zz - zy*yz) - yx * (xy*zz - zy*xz) + zx * (xy*yz - yy*xz)) * inv_det;

	return m;
}

} // namespace crown
