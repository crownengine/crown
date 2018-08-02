/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/math/matrix4x4.h"

namespace crown
{
void perspective(Matrix4x4& m, f32 fovy, f32 aspect, f32 nnear, f32 ffar)
{
	const f32 height = 1.0f / ftan(fovy * 0.5f);
	const f32 width = height * 1.0f / aspect;
	const f32 aa = ffar / (ffar - nnear);
	const f32 bb = -nnear * aa;

	m.x.x = width;
	m.x.y = 0.0f;
	m.x.z = 0.0f;
	m.x.w = 0.0f;

	m.y.x = 0.0f;
	m.y.y = height;
	m.y.z = 0.0f;
	m.y.w = 0.0f;

	m.z.x = 0.0f;
	m.z.y = 0.0f;
	m.z.z = aa;
	m.z.w = 1.0f;

	m.t.x = 0.0f;
	m.t.y = 0.0f;
	m.t.z = bb;
	m.t.w = 0.0f;
}

void orthographic(Matrix4x4& m, f32 left, f32 right, f32 bottom, f32 top, f32 nnear, f32 ffar)
{
	m.x.x = 2.0f / (right - left);
	m.x.y = 0.0f;
	m.x.z = 0.0f;
	m.x.w = 0.0f;

	m.y.x = 0.0f;
	m.y.y = 2.0f / (top - bottom);
	m.y.z = 0.0f;
	m.y.w = 0.0f;

	m.z.x = 0.0f;
	m.z.y = 0.0f;
	m.z.z = 1.0f / (ffar - nnear);
	m.z.w = 0.0f;

	m.t.x = (left + right) / (left - right);
	m.t.y = (top + bottom) / (bottom - top);
	m.t.z = nnear / (nnear - ffar);
	m.t.w = 1.0f;
}

void look(Matrix4x4& m, const Vector3& pos, const Vector3& target, const Vector3& up)
{
	Vector3 zaxis = pos - target;
	normalize(zaxis);
	const Vector3 xaxis = cross(up, zaxis);
	const Vector3 yaxis = cross(zaxis, xaxis);

	m.x.x = xaxis.x;
	m.x.y = yaxis.x;
	m.x.z = zaxis.x;
	m.x.w = 0.0f;

	m.y.x = xaxis.y;
	m.y.y = yaxis.y;
	m.y.z = zaxis.y;
	m.y.w = 0.0f;

	m.z.x = xaxis.z;
	m.z.y = yaxis.z;
	m.z.z = zaxis.z;
	m.z.w = 0.0f;

	m.t.x = -dot(pos, xaxis);
	m.t.y = -dot(pos, yaxis);
	m.t.z = -dot(pos, zaxis);
	m.t.w = 1.0f;
}

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
