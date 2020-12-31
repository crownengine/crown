/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/math/matrix4x4.inl"

namespace crown
{
Matrix4x4 from_elements(f32 xx, f32 xy, f32 xz, f32 xw
	, f32 yx, f32 yy, f32 yz, f32 yw
	, f32 zx, f32 zy, f32 zz, f32 zw
	, f32 tx, f32 ty, f32 tz, f32 tw
	)
{
	Matrix4x4 m;
	m.x.x = xx;
	m.x.y = xy;
	m.x.z = xz;
	m.x.w = xw;

	m.y.x = yx;
	m.y.y = yy;
	m.y.z = yz;
	m.y.w = yw;

	m.z.x = zx;
	m.z.y = zy;
	m.z.z = zz;
	m.z.w = zw;

	m.t.x = tx;
	m.t.y = ty;
	m.t.z = tz;
	m.t.w = tw;
	return m;
}

Matrix4x4 from_array(const f32 a[16])
{
	Matrix4x4 m;
	m.x.x = a[0];
	m.x.y = a[1];
	m.x.z = a[2];
	m.x.w = a[3];

	m.y.x = a[4];
	m.y.y = a[5];
	m.y.z = a[6];
	m.y.w = a[7];

	m.z.x = a[8];
	m.z.y = a[9];
	m.z.z = a[10];
	m.z.w = a[11];

	m.t.x = a[12];
	m.t.y = a[13];
	m.t.z = a[14];
	m.t.w = a[15];
	return m;
}

Matrix4x4 from_axes(const Vector3& x, const Vector3& y, const Vector3& z, const Vector3& t)
{
	Matrix4x4 m;
	m.x.x = x.x;
	m.x.y = x.y;
	m.x.z = x.z;
	m.x.w = 0.0f;

	m.y.x = y.x;
	m.y.y = y.y;
	m.y.z = y.z;
	m.y.w = 0.0f;

	m.z.x = z.x;
	m.z.y = z.y;
	m.z.z = z.z;
	m.z.w = 0.0f;

	m.t.x = t.x;
	m.t.y = t.y;
	m.t.z = t.z;
	m.t.w = 1.0f;
	return m;
}

Matrix4x4 from_quaternion_translation(const Quaternion& r, const Vector3& t)
{
	const Matrix3x3 rot = from_quaternion(r);

	Matrix4x4 m;
	m.x.x = rot.x.x;
	m.x.y = rot.x.y;
	m.x.z = rot.x.z;
	m.x.w = 0.0f;

	m.y.x = rot.y.x;
	m.y.y = rot.y.y;
	m.y.z = rot.y.z;
	m.y.w = 0.0f;

	m.z.x = rot.z.x;
	m.z.y = rot.z.y;
	m.z.z = rot.z.z;
	m.z.w = 0.0f;

	m.t.x = t.x;
	m.t.y = t.y;
	m.t.z = t.z;
	m.t.w = 1.0f;
	return m;
}

Matrix4x4 from_translation(const Vector3& t)
{
	Matrix4x4 m;
	m.x.x = 1.0f;
	m.x.y = 0.0f;
	m.x.z = 0.0f;
	m.x.w = 0.0f;

	m.y.x = 0.0f;
	m.y.y = 1.0f;
	m.y.z = 0.0f;
	m.y.w = 0.0f;

	m.z.x = 0.0f;
	m.z.y = 0.0f;
	m.z.z = 1.0f;
	m.z.w = 0.0f;

	m.t.x = t.x;
	m.t.y = t.y;
	m.t.z = t.z;
	m.t.w = 1.0f;
	return m;
}

Matrix4x4 from_matrix3x3(const Matrix3x3& r)
{
	Matrix4x4 m;
	m.x.x = r.x.x;
	m.x.y = r.x.y;
	m.x.z = r.x.z;
	m.x.w = 0.0f;

	m.y.x = r.y.x;
	m.y.y = r.y.y;
	m.y.z = r.y.z;
	m.y.w = 0.0f;

	m.z.x = r.z.x;
	m.z.y = r.z.y;
	m.z.z = r.z.z;
	m.z.w = 0.0f;

	m.t.x = 0.0f;
	m.t.y = 0.0f;
	m.t.z = 0.0f;
	m.t.w = 1.0f;
	return m;
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
