/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/math/constants.h"
#include "core/math/math.h"
#include "core/math/matrix3x3.inl"
#include "core/math/quaternion.inl"
#include "core/math/types.h"
#include "core/math/vector4.inl"

namespace crown
{
/// @addtogroup Math
/// @{

/// Returns a new matrix from individual elements.
Matrix4x4 from_elements(f32 xx, f32 xy, f32 xz, f32 xw
	, f32 yx, f32 yy, f32 yz, f32 yw
	, f32 zx, f32 zy, f32 zz, f32 zw
	, f32 tx, f32 ty, f32 tz, f32 tw
	);

/// Returns a new matrix from individual elements.
Matrix4x4 from_array(const f32 a[16]);

/// Returns a new matrix from axes @a x, @a y and @a z and translation @a t.
Matrix4x4 from_axes(const Vector3& x, const Vector3& y, const Vector3& z, const Vector3& t);

/// Returns a new matrix from rotation @a r and translation @a t.
Matrix4x4 from_quaternion_translation(const Quaternion& r, const Vector3& t);

/// Returns a new matrix from translation @a t.
Matrix4x4 from_translation(const Vector3& t);

/// Returns a new matrix from rotation matrix @a r.
Matrix4x4 from_matrix3x3(const Matrix3x3& r);

/// Adds the matrix @a a to @a b and returns the result.
inline Matrix4x4& operator+=(Matrix4x4& a, const Matrix4x4& b)
{
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	a.t += b.t;
	return a;
}

/// Subtracts the matrix @a b from @a a and returns the result.
inline Matrix4x4& operator-=(Matrix4x4& a, const Matrix4x4& b)
{
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	a.t -= b.t;
	return a;
}

/// Multiplies the matrix @a a by the scalar @a k and returns the result.
inline Matrix4x4& operator*=(Matrix4x4& a, f32 k)
{
	a.x *= k;
	a.y *= k;
	a.z *= k;
	a.t *= k;
	return a;
}

/// Multiplies the matrix @a a by @a b and returns the result. (i.e. transforms first by @a a then by @a b)
inline Matrix4x4& operator*=(Matrix4x4& a, const Matrix4x4& b)
{
	Matrix4x4 tmp;

	tmp.x.x = a.x.x*b.x.x + a.x.y*b.y.x + a.x.z*b.z.x + a.x.w*b.t.x;
	tmp.x.y = a.x.x*b.x.y + a.x.y*b.y.y + a.x.z*b.z.y + a.x.w*b.t.y;
	tmp.x.z = a.x.x*b.x.z + a.x.y*b.y.z + a.x.z*b.z.z + a.x.w*b.t.z;
	tmp.x.w = a.x.x*b.x.w + a.x.y*b.y.w + a.x.z*b.z.w + a.x.w*b.t.w;

	tmp.y.x = a.y.x*b.x.x + a.y.y*b.y.x + a.y.z*b.z.x + a.y.w*b.t.x;
	tmp.y.y = a.y.x*b.x.y + a.y.y*b.y.y + a.y.z*b.z.y + a.y.w*b.t.y;
	tmp.y.z = a.y.x*b.x.z + a.y.y*b.y.z + a.y.z*b.z.z + a.y.w*b.t.z;
	tmp.y.w = a.y.x*b.x.w + a.y.y*b.y.w + a.y.z*b.z.w + a.y.w*b.t.w;

	tmp.z.x = a.z.x*b.x.x + a.z.y*b.y.x + a.z.z*b.z.x + a.z.w*b.t.x;
	tmp.z.y = a.z.x*b.x.y + a.z.y*b.y.y + a.z.z*b.z.y + a.z.w*b.t.y;
	tmp.z.z = a.z.x*b.x.z + a.z.y*b.y.z + a.z.z*b.z.z + a.z.w*b.t.z;
	tmp.z.w = a.z.x*b.x.w + a.z.y*b.y.w + a.z.z*b.z.w + a.z.w*b.t.w;

	tmp.t.x = a.t.x*b.x.x + a.t.y*b.y.x + a.t.z*b.z.x + a.t.w*b.t.x;
	tmp.t.y = a.t.x*b.x.y + a.t.y*b.y.y + a.t.z*b.z.y + a.t.w*b.t.y;
	tmp.t.z = a.t.x*b.x.z + a.t.y*b.y.z + a.t.z*b.z.z + a.t.w*b.t.z;
	tmp.t.w = a.t.x*b.x.w + a.t.y*b.y.w + a.t.z*b.z.w + a.t.w*b.t.w;

	a = tmp;
	return a;
}

/// Adds the matrix @a a to @a b and returns the result.
inline Matrix4x4 operator+(Matrix4x4 a, const Matrix4x4& b)
{
	a += b;
	return a;
}

/// Subtracts the matrix @a b from @a a and returns the result.
inline Matrix4x4 operator-(Matrix4x4 a, const Matrix4x4& b)
{
	a -= b;
	return a;
}

/// Multiplies the matrix @a a by the scalar @a k and returns the result.
inline Matrix4x4 operator*(Matrix4x4 a, f32 k)
{
	a *= k;
	return a;
}

/// Multiplies the matrix @a a by the scalar @a k and returns the result.
inline Matrix4x4 operator*(f32 k, Matrix4x4 a)
{
	a *= k;
	return a;
}

/// Multiplies the matrix @a a by the vector @a v and returns the result.
inline Vector3 operator*(const Vector3& v, const Matrix4x4& a)
{
	Vector3 r;
	r.x = v.x*a.x.x + v.y*a.y.x + v.z*a.z.x + a.t.x;
	r.y = v.x*a.x.y + v.y*a.y.y + v.z*a.z.y + a.t.y;
	r.z = v.x*a.x.z + v.y*a.y.z + v.z*a.z.z + a.t.z;
	return r;
}

/// Multiplies the matrix @a by the vector @a v and returns the result.
inline Vector4 operator*(const Vector4& v, const Matrix4x4& a)
{
	Vector4 r;
	r.x = v.x*a.x.x + v.y*a.y.x + v.z*a.z.x + v.w*a.t.x;
	r.y = v.x*a.x.y + v.y*a.y.y + v.z*a.z.y + v.w*a.t.y;
	r.z = v.x*a.x.z + v.y*a.y.z + v.z*a.z.z + v.w*a.t.z;
	r.w = v.x*a.x.w + v.y*a.y.w + v.z*a.z.w + v.w*a.t.w;
	return r;
}

/// Multiplies the matrix @a a by @a b and returns the result. (i.e. transforms first by @a a then by @a b)
inline Matrix4x4 operator*(Matrix4x4 a, const Matrix4x4& b)
{
	a *= b;
	return a;
}

/// Returns true whether the matrices @a a and @a b are equal.
inline bool operator==(const Matrix4x4& a, const Matrix4x4& b)
{
	return a.x == b.x
		&& a.y == b.y
		&& a.z == b.z
		&& a.t == b.t
		;
}

/// Transposes the matrix @a m and returns the result.
inline Matrix4x4& transpose(Matrix4x4& m)
{
	f32 tmp;

	tmp = m.x.y;
	m.x.y = m.y.x;
	m.y.x = tmp;

	tmp = m.x.z;
	m.x.z = m.z.x;
	m.z.x = tmp;

	tmp = m.x.w;
	m.x.w = m.t.x;
	m.t.x = tmp;

	tmp = m.y.z;
	m.y.z = m.z.y;
	m.z.y = tmp;

	tmp = m.y.w;
	m.y.w = m.t.y;
	m.t.y = tmp;

	tmp = m.z.w;
	m.z.w = m.t.z;
	m.t.z = tmp;

	return m;
}

/// Returns the transposed of the matrix @a m.
inline Matrix4x4 get_transposed(Matrix4x4 m)
{
	transpose(m);
	return m;
}

/// Inverts the matrix @a m and returns the result.
Matrix4x4& invert(Matrix4x4& m);

/// Returns the inverse of the matrix @a m.
inline Matrix4x4 get_inverted(Matrix4x4 m)
{
	invert(m);
	return m;
}

/// Sets the matrix @a m to identity.
inline void set_identity(Matrix4x4& m)
{
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

	m.t.x = 0.0f;
	m.t.y = 0.0f;
	m.t.z = 0.0f;
	m.t.w = 1.0f;
}

/// Returns the x axis of the matrix @a m.
inline Vector3 x(const Matrix4x4& m)
{
	Vector3 v;
	v.x = m.x.x;
	v.y = m.x.y;
	v.z = m.x.z;
	return v;
}

/// Returns the y axis of the matrix @a m.
inline Vector3 y(const Matrix4x4& m)
{
	Vector3 v;
	v.x = m.y.x;
	v.y = m.y.y;
	v.z = m.y.z;
	return v;
}

/// Returns the z axis of the matrix @a m.
inline Vector3 z(const Matrix4x4& m)
{
	Vector3 v;
	v.x = m.z.x;
	v.y = m.z.y;
	v.z = m.z.z;
	return v;
}

/// Sets the x axis of the matrix @a m.
inline void set_x(Matrix4x4& m, const Vector3& x)
{
	m.x.x = x.x;
	m.x.y = x.y;
	m.x.z = x.z;
}

/// Sets the y axis of the matrix @a m.
inline void set_y(Matrix4x4& m, const Vector3& y)
{
	m.y.x = y.x;
	m.y.y = y.y;
	m.y.z = y.z;
}

/// Sets the z axis of the matrix @a m.
inline void set_z(Matrix4x4& m, const Vector3& z)
{
	m.z.x = z.x;
	m.z.y = z.y;
	m.z.z = z.z;
}

/// Returns the translation portion of the matrix @a m.
inline Vector3 translation(const Matrix4x4& m)
{
	Vector3 v;
	v.x = m.t.x;
	v.y = m.t.y;
	v.z = m.t.z;
	return v;
}

/// Sets the translation portion of the matrix @a m.
inline void set_translation(Matrix4x4& m, const Vector3& trans)
{
	m.t.x = trans.x;
	m.t.y = trans.y;
	m.t.z = trans.z;
}

/// Returns the rotation portion of the matrix @a m as a Matrix3x3.
inline Matrix3x3 to_matrix3x3(const Matrix4x4& m)
{
	Matrix3x3 res;
	res.x.x = m.x.x;
	res.x.y = m.x.y;
	res.x.z = m.x.z;

	res.y.x = m.y.x;
	res.y.y = m.y.y;
	res.y.z = m.y.z;

	res.z.x = m.z.x;
	res.z.y = m.z.y;
	res.z.z = m.z.z;
	return res;
}

/// Returns the rotation portion of the matrix @a m as a Quaternion.
inline Quaternion rotation(const Matrix3x3& m)
{
	const f32 lx = length(m.x);
	const f32 ly = length(m.y);
	const f32 lz = length(m.z);

	if (CE_UNLIKELY(fequal(lx, 0.0f) || fequal(ly, 0.0f) || fequal(lz, 0.0f)))
		return QUATERNION_IDENTITY;

	Matrix3x3 rot = m;
	rot.x *= 1.0f / lx;
	rot.y *= 1.0f / ly;
	rot.z *= 1.0f / lz;
	Quaternion q = quaternion(rot);
	normalize(q);
	return q;
}

/// Returns the rotation portion of the matrix @a m as a Quaternion.
inline Quaternion rotation(const Matrix4x4& m)
{
	return rotation(to_matrix3x3(m));
}

/// Sets the rotation portion of the matrix @a m.
inline void set_rotation(Matrix4x4& m, const Matrix3x3& rot)
{
	m.x.x = rot.x.x;
	m.x.y = rot.x.y;
	m.x.z = rot.x.z;

	m.y.x = rot.y.x;
	m.y.y = rot.y.y;
	m.y.z = rot.y.z;

	m.z.x = rot.z.x;
	m.z.y = rot.z.y;
	m.z.z = rot.z.z;
}

/// Sets the rotation portion of the matrix @a m.
inline void set_rotation(Matrix4x4& m, const Quaternion& rot)
{
	set_rotation(m, from_quaternion(rot));
}

/// Returns the scale of the matrix @a m.
inline Vector3 scale(const Matrix4x4& m)
{
	const f32 sx = length(to_vector3(m.x));
	const f32 sy = length(to_vector3(m.y));
	const f32 sz = length(to_vector3(m.z));
	Vector3 v;
	v.x = sx;
	v.y = sy;
	v.z = sz;
	return v;
}

/// Sets the scale of the matrix @a m.
inline void set_scale(Matrix4x4& m, const Vector3& s)
{
	Matrix3x3 rot = to_matrix3x3(m);
	set_scale(rot, s);
	set_rotation(m, rot);
}

/// Returns the pointer to the matrix's data
inline f32* to_float_ptr(Matrix4x4& m)
{
	return to_float_ptr(m.x);
}

/// Returns the pointer to the matrix's data
inline const f32* to_float_ptr(const Matrix4x4& m)
{
	return to_float_ptr(m.x);
}

/// Returns a string representing the matrix @m.
/// @note This function is for debugging purposes only and doesn't
/// output round-trip safe ASCII conversions. Do not use in production.
const char* to_string(const Matrix4x4& m, char* buf, u32 buf_len);

/// @}

} // namespace crown
