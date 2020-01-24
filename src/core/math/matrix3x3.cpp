/*
 * Copyright (c) 2012-2020 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/math/matrix3x3.inl"

namespace crown
{
Matrix3x3 from_elements(f32 xx, f32 xy, f32 xz, f32 yx, f32 yy, f32 yz, f32 zx, f32 zy, f32 zz)
{
	Matrix3x3 m;
	m.x.x = xx;
	m.x.y = xy;
	m.x.z = xz;

	m.y.x = yx;
	m.y.y = yy;
	m.y.z = yz;

	m.z.x = zx;
	m.z.y = zy;
	m.z.z = zz;
	return m;
}

Matrix3x3 from_axes(const Vector3& x, const Vector3& y, const Vector3& z)
{
	Matrix3x3 m;
	m.x = x;
	m.y = y;
	m.z = z;
	return m;
}

Matrix3x3 from_quaternion(const Quaternion& r)
{
	Matrix3x3 m;
	m.x.x = 1.0f - 2.0f * r.y * r.y - 2.0f * r.z * r.z;
	m.x.y = 2.0f * r.x * r.y + 2.0f * r.w * r.z;
	m.x.z = 2.0f * r.x * r.z - 2.0f * r.w * r.y;

	m.y.x = 2.0f * r.x * r.y - 2.0f * r.w * r.z;
	m.y.y = 1.0f - 2.0f * r.x * r.x - 2.0f * r.z * r.z;
	m.y.z = 2.0f * r.y * r.z + 2.0f * r.w * r.x;

	m.z.x = 2.0f * r.x * r.z + 2.0f * r.w * r.y;
	m.z.y = 2.0f * r.y * r.z - 2.0f * r.w * r.x;
	m.z.z = 1.0f - 2.0f * r.x * r.x - 2.0f * r.y * r.y;
	return m;
}

Matrix3x3& invert(Matrix3x3& m)
{
	const f32 xx = m.x.x;
	const f32 xy = m.x.y;
	const f32 xz = m.x.z;
	const f32 yx = m.y.x;
	const f32 yy = m.y.y;
	const f32 yz = m.y.z;
	const f32 zx = m.z.x;
	const f32 zy = m.z.y;
	const f32 zz = m.z.z;

	f32 det = 0.0f;
	det += m.x.x * (m.y.y * m.z.z - m.z.y * m.y.z);
	det -= m.y.x * (m.x.y * m.z.z - m.z.y * m.x.z);
	det += m.z.x * (m.x.y * m.y.z - m.y.y * m.x.z);

	const f32 inv_det = 1.0f / det;

	m.x.x = + (yy*zz - zy*yz) * inv_det;
	m.x.y = - (xy*zz - zy*xz) * inv_det;
	m.x.z = + (xy*yz - yy*xz) * inv_det;

	m.y.x = - (yx*zz - zx*yz) * inv_det;
	m.y.y = + (xx*zz - zx*xz) * inv_det;
	m.y.z = - (xx*yz - yx*xz) * inv_det;

	m.z.x = + (yx*zy - zx*yy) * inv_det;
	m.z.y = - (xx*zy - zx*xy) * inv_det;
	m.z.z = + (xx*yy - yx*xy) * inv_det;

	return m;
}

} // namespace crown
