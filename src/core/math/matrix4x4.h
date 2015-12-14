/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "math_types.h"
#include "vector4.h"
#include "quaternion.h"
#include "matrix3x3.h"

namespace crown
{
/// @addtogroup Math
/// @{

inline Matrix4x4 matrix4x4(float r1c1, float r2c1, float r3c1, float r4c1
	, float r1c2, float r2c2, float r3c2, float r4c2
	, float r1c3, float r2c3, float r3c3, float r4c3
	, float r1c4, float r2c4, float r3c4, float r4c4
	)
{
	Matrix4x4 m;
	m.x.x = r1c1;
	m.x.y = r1c2;
	m.x.z = r1c3;
	m.x.w = r1c4;

	m.y.x = r2c1;
	m.y.y = r2c2;
	m.y.z = r2c3;
	m.y.w = r2c4;

	m.z.x = r3c1;
	m.z.y = r3c2;
	m.z.z = r3c3;
	m.z.w = r3c4;

	m.t.x = r4c1;
	m.t.y = r4c2;
	m.t.z = r4c3;
	m.t.w = r4c4;
	return m;
}

inline Matrix4x4 matrix4x4(const float a[16])
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

inline Matrix4x4 matrix4x4(const Vector3& x, const Vector3& y, const Vector3& z, const Vector3& t)
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

inline Matrix4x4 matrix4x4(const Quaternion& r, const Vector3& p)
{
	Matrix4x4 m;
	m.x.x = 1.0f - 2.0f * r.y * r.y - 2.0f * r.z * r.z;
	m.x.y = 2.0f * r.x * r.y + 2.0f * r.w * r.z;
	m.x.z = 2.0f * r.x * r.z - 2.0f * r.w * r.y;
	m.x.w = 0.0f;

	m.y.x = 2.0f * r.x * r.y - 2.0f * r.w * r.z;
	m.y.y = 1.0f - 2.0f * r.x * r.x - 2.0f * r.z * r.z;
	m.y.z = 2.0f * r.y * r.z + 2.0f * r.w * r.x;
	m.y.w = 0.0f;

	m.z.x = 2.0f * r.x * r.z + 2.0f * r.w * r.y;
	m.z.y = 2.0f * r.y * r.z - 2.0f * r.w * r.x;
	m.z.z = 1.0f - 2.0f * r.x * r.x - 2.0f * r.y * r.y;
	m.z.w = 0.0f;

	m.t.x = p.x;
	m.t.y = p.y;
	m.t.z = p.z;
	m.t.w = 1.0f;
	return m;
}

inline Matrix4x4 matrix4x4(const Matrix3x3& rot)
{
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

	m.t.x = 0.0f;
	m.t.y = 0.0f;
	m.t.z = 0.0f;
	m.t.w = 1.0f;
	return m;
}

inline Matrix4x4& operator+=(Matrix4x4& a, const Matrix4x4& b)
{
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	a.t += b.t;
	return a;
}

inline Matrix4x4& operator-=(Matrix4x4& a, const Matrix4x4& b)
{
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	a.t -= b.t;
	return a;
}

inline Matrix4x4& operator*=(Matrix4x4& a, float k)
{
	a.x *= k;
	a.y *= k;
	a.z *= k;
	a.t *= k;
	return a;
}

inline Matrix4x4& operator/=(Matrix4x4& a, float k)
{
	const float inv_k = 1.0f / k;
	a.x *= inv_k;
	a.y *= inv_k;
	a.z *= inv_k;
	a.t *= inv_k;
	return a;
}

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
inline Matrix4x4 operator*(Matrix4x4 a, float k)
{
	a *= k;
	return a;
}

/// Multiplies the matrix @a a by the scalar @a k and returns the result.
inline Matrix4x4 operator*(float k, Matrix4x4 a)
{
	a *= k;
	return a;
}

/// Divides the matrix @a a by the scalar @a k and returns the result.
inline Matrix4x4 operator/(Matrix4x4 a, float k)
{
	a /= k;
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

/// Sets the matrix @a m to perspective.
inline void set_perspective(Matrix4x4& m, float fovy, float aspect, float near, float far)
{
	const float height = 1.0f / tanf(to_rad(fovy) * 0.5f);
	const float width = height * 1.0f / aspect;
	const float aa = far / (far - near);
	const float bb = -near * aa;

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

/// Sets the matrix @a m to orthographic.
inline void set_orthographic(Matrix4x4& m, float left, float right, float bottom, float top, float near, float far)
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
	m.z.z = 1.0f / (far - near);
	m.z.w = 0.0f;

	m.t.x = (left + right) / (left - right);
	m.t.y = (top + bottom) / (bottom - top);
	m.t.z = near / (near - far);
	m.t.w = 1.0f;
}

/// Transposes the matrix @a m and returns the result.
inline Matrix4x4& transpose(Matrix4x4& m)
{
	float tmp;

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

/// Sets the matrix @a m to look.
inline void set_look(Matrix4x4& m, const Vector3& pos, const Vector3& target, const Vector3& up)
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

/// Returns the determinant of the matrix @a m.
inline float determinant(const Matrix4x4& m)
{
	const float m02m07_m06m03 = m.x.z * m.y.w - m.y.z * m.x.w;
	const float m02m11_m10m03 = m.x.z * m.z.w - m.z.z * m.x.w;
	const float m02m15_m14m03 = m.x.z * m.t.w - m.t.z * m.x.w;
	const float m06m11_m10m07 = m.y.z * m.z.w - m.z.z * m.y.w;
	const float m06m15_m14m07 = m.y.z * m.t.w - m.t.z * m.y.w;
	const float m10m15_m14m11 = m.z.z * m.t.w - m.t.z * m.z.w;

	return 	+ m.x.x * (m.y.y * m10m15_m14m11 - m.z.y * m06m15_m14m07 + m.t.y * m06m11_m10m07)
			- m.y.x * (m.x.y * m10m15_m14m11 - m.z.y * m02m15_m14m03 + m.t.y * m02m11_m10m03)
			+ m.z.x * (m.x.y * m06m15_m14m07 - m.y.y * m02m15_m14m03 + m.t.y * m02m07_m06m03)
			- m.t.x * (m.x.y * m06m11_m10m07 - m.y.y * m02m11_m10m03 + m.z.y * m02m07_m06m03);
}

/// Inverts the matrix @a m and returns the result.
inline Matrix4x4& invert(Matrix4x4& m)
{
	Matrix4x4 mat;

	const float m01m06_m05m02 = m.x.y * m.y.z - m.y.y * m.x.z;
	const float m01m07_m05m03 = m.x.y * m.y.w - m.y.y * m.x.w;
	const float m01m10_m09m02 = m.x.y * m.z.z - m.z.y * m.x.z;
	const float m01m11_m09m03 = m.x.y * m.z.w - m.z.y * m.x.w;
	const float m01m14_m13m02 = m.x.y * m.t.z - m.t.y * m.x.z;
	const float m01m15_m13m03 = m.x.y * m.t.w - m.t.y * m.x.w;
	const float m02m07_m06m03 = m.x.z * m.y.w - m.y.z * m.x.w;
	const float m02m11_m10m03 = m.x.z * m.z.w - m.z.z * m.x.w;
	const float m02m15_m14m03 = m.x.z * m.t.w - m.t.z * m.x.w;
	const float m05m10_m09m06 = m.y.y * m.z.z - m.z.y * m.y.z;
	const float m05m11_m09m07 = m.y.y * m.z.w - m.z.y * m.y.w;
	const float m05m14_m13m06 = m.y.y * m.t.z - m.t.y * m.y.z;
	const float m05m15_m13m07 = m.y.y * m.t.w - m.t.y * m.y.w;
	const float m06m11_m10m07 = m.y.z * m.z.w - m.z.z * m.y.w;
	const float m06m15_m14m07 = m.y.z * m.t.w - m.t.z * m.y.w;
	const float m09m14_m13m10 = m.z.y * m.t.z - m.t.y * m.z.z;
	const float m09m15_m13m11 = m.z.y * m.t.w - m.t.y * m.z.w;
	const float m10m15_m14m11 = m.z.z * m.t.w - m.t.z * m.z.w;

	mat.x.x = (+ m.y.y * m10m15_m14m11 - m.z.y * m06m15_m14m07 + m.t.y * m06m11_m10m07);
	mat.x.y = (+ m.x.y * m10m15_m14m11 - m.z.y * m02m15_m14m03 + m.t.y * m02m11_m10m03);
	mat.x.z = (+ m.x.y * m06m15_m14m07 - m.y.y * m02m15_m14m03 + m.t.y * m02m07_m06m03);
	mat.x.w = (+ m.x.y * m06m11_m10m07 - m.y.y * m02m11_m10m03 + m.z.y * m02m07_m06m03);

	const float inv_det = 1.0f / (m.x.x * mat.x.x - m.y.x * mat.x.y + m.z.x * mat.x.z - m.t.x * mat.x.w);

	mat.y.x = (+ m.y.x * m10m15_m14m11 - m.z.x * m06m15_m14m07 + m.t.x * m06m11_m10m07);
	mat.y.y = (+ m.x.x * m10m15_m14m11 - m.z.x * m02m15_m14m03 + m.t.x * m02m11_m10m03);
	mat.y.z = (+ m.x.x * m06m15_m14m07 - m.y.x * m02m15_m14m03 + m.t.x * m02m07_m06m03);
	mat.y.w = (+ m.x.x * m06m11_m10m07 - m.y.x * m02m11_m10m03 + m.z.x * m02m07_m06m03);
	mat.z.x = (+ m.y.x * m09m15_m13m11 - m.z.x * m05m15_m13m07 + m.t.x * m05m11_m09m07);
	mat.z.y = (+ m.x.x * m09m15_m13m11 - m.z.x * m01m15_m13m03 + m.t.x * m01m11_m09m03);
	mat.z.z = (+ m.x.x * m05m15_m13m07 - m.y.x * m01m15_m13m03 + m.t.x * m01m07_m05m03);
	mat.z.w = (+ m.x.x * m05m11_m09m07 - m.y.x * m01m11_m09m03 + m.z.x * m01m07_m05m03);
	mat.t.x = (+ m.y.x * m09m14_m13m10 - m.z.x * m05m14_m13m06 + m.t.x * m05m10_m09m06);
	mat.t.y = (+ m.x.x * m09m14_m13m10 - m.z.x * m01m14_m13m02 + m.t.x * m01m10_m09m02);
	mat.t.z = (+ m.x.x * m05m14_m13m06 - m.y.x * m01m14_m13m02 + m.t.x * m01m06_m05m02);
	mat.t.w = (+ m.x.x * m05m10_m09m06 - m.y.x * m01m10_m09m02 + m.z.x * m01m06_m05m02);

	m.x.x = + mat.x.x * inv_det;
	m.x.y = - mat.x.y * inv_det;
	m.x.z = + mat.x.z * inv_det;
	m.x.w = - mat.x.w * inv_det;
	m.y.x = - mat.y.x * inv_det;
	m.y.y = + mat.y.y * inv_det;
	m.y.z = - mat.y.z * inv_det;
	m.y.w = + mat.y.w * inv_det;
	m.z.x = + mat.z.x * inv_det;
	m.z.y = - mat.z.y * inv_det;
	m.z.z = + mat.z.z * inv_det;
	m.z.w = - mat.z.w * inv_det;
	m.t.x = - mat.t.x * inv_det;
	m.t.y = + mat.t.y * inv_det;
	m.t.z = - mat.t.z * inv_det;
	m.t.w = + mat.t.w * inv_det;

	return m;
}

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

/// Returns the x asis of the matrix @a m.
inline Vector3 x(const Matrix4x4& m)
{
	Vector3 v;
	v.x = m.x.x;
	v.y = m.x.y;
	v.z = m.x.z;
	return v;
}

/// Returns the y asis of the matrix @a m.
inline Vector3 y(const Matrix4x4& m)
{
	Vector3 v;
	v.x = m.y.x;
	v.y = m.y.y;
	v.z = m.y.z;
	return v;
}

/// Returns the z asis of the matrix @a m.
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
inline Quaternion rotation(const Matrix4x4& m)
{
	return quaternion(to_matrix3x3(m));
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
	set_rotation(m, matrix3x3(rot));
}

/// Returns the scale of the matrix @a m.
inline Vector3 scale(const Matrix4x4& m)
{
	const float sx = length(to_vector3(m.x));
	const float sy = length(to_vector3(m.y));
	const float sz = length(to_vector3(m.z));
	Vector3 v;
	v.x = sx;
	v.y = sy;
	v.z = sz;
	return v;
}

/// Sets the scale of the matrix @æ m.
inline void set_scale(Matrix4x4& m, const Vector3& s)
{
	Matrix3x3 rot = to_matrix3x3(m);
	set_scale(rot, s);
	set_rotation(m, rot);
}

/// Returns the pointer to the matrix's data
inline float* to_float_ptr(Matrix4x4& m)
{
	return to_float_ptr(m.x);
}

/// Returns the pointer to the matrix's data
inline const float* to_float_ptr(const Matrix4x4& m)
{
	return to_float_ptr(m.x);
}

/// @}
} // namespace crown
