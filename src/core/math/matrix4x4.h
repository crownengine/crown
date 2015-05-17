/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "math_types.h"
#include "matrix3x3.h"
#include "quaternion.h"
#include "vector4.h"
#include "types.h"

namespace crown
{

/// Adds the matrix @a a to @a b and returns the result.
Matrix4x4 operator+(Matrix4x4 a, const Matrix4x4& b);

/// Subtracts the matrix @a b from @a a and returns the result.
Matrix4x4 operator-(Matrix4x4 a, const Matrix4x4& b);

/// Multiplies the matrix @a a by the scalar @a k and returns the result.
Matrix4x4 operator*(Matrix4x4 a, float k);

/// @copydoc operator*(Matrix4x4, float)
Matrix4x4 operator*(float k, Matrix4x4 a);

/// Divides the matrix @a a by the scalar @a k and returns the result.
Matrix4x4 operator/(Matrix4x4 a, float k);

/// Multiplies the matrix @a a by the vector @a v and returns the result.
Vector3 operator*(const Vector3& v, const Matrix4x4& a);

/// Multiplies the matrix @a by the vector @a v and returns the result.
Vector4 operator*(const Vector4& v, const Matrix4x4& a);

/// Multiplies the matrix @a a by @a b and returns the result. (i.e. transforms first by @a a then by @a b)
Matrix4x4 operator*(Matrix4x4 a, const Matrix4x4& b);

/// Functions to manipulate Matrix4x4.
///
/// @ingroup Math
namespace matrix4x4
{
	const Matrix4x4 IDENTITY = Matrix4x4(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0);

	/// Sets the matrix @a m to perspective.
	void set_perspective(Matrix4x4& m, float fovy, float aspect, float near, float far);

	/// Sets the matrix @a m to orthographic.
	void set_orthographic(Matrix4x4& m, float left, float right, float bottom, float top, float near, float far);

	/// Sets the matrix @a m to look.
	void set_look(Matrix4x4& m, const Vector3& pos, const Vector3& target, const Vector3& up);

	/// Transposes the matrix @a m and returns the result.
	Matrix4x4& transpose(Matrix4x4& m);

	/// Returns the transposed of the matrix @a m.
	Matrix4x4 get_transposed(Matrix4x4 m);

	/// Returns the determinant of the matrix @a m.
	float determinant(const Matrix4x4& m);

	/// Inverts the matrix @a m and returns the result.
	Matrix4x4& invert(Matrix4x4& m);

	/// Returns the inverse of the matrix @a m.
	Matrix4x4 get_inverted(Matrix4x4 m);

	/// Sets the matrix @a m to identity.
	void set_identity(Matrix4x4& m);

	/// Returns the x asis of the matrix @a m.
	Vector3 x(const Matrix4x4& m);

	/// Returns the y axis of the matrix @a m.
	Vector3 y(const Matrix4x4& m);

	/// Returns the z axis of the matrix @a m.
	Vector3 z(const Matrix4x4& m);

	/// Sets the x axis of the matrix @a m.
	void set_x(Matrix4x4& m, const Vector3& x);

	/// Sets the y axis of the matrix @a m.
	void set_y(Matrix4x4& m, const Vector3& y);

	/// Sets the z axis of the matrix @a m.
	void set_z(Matrix4x4& m, const Vector3& z);

	/// Returns the translation portion of the matrix @a m.
	Vector3 translation(const Matrix4x4& m);

	/// Sets the translation portion of the matrix @a m.
	void set_translation(Matrix4x4& m, const Vector3& trans);

	/// Returns the rotation portion of the matrix @a m as a Quaternion.
	Quaternion rotation(const Matrix4x4& m);

	/// Sets the rotation portion of the matrix @a m.
	void set_rotation(Matrix4x4& m, const Quaternion& rot);

	/// Sets the rotation portion of the matrix @a m.
	void set_rotation(Matrix4x4& m, const Matrix3x3& rot);

	/// Returns the scale of the matrix @a m.
	Vector3 scale(const Matrix4x4& m);

	/// Sets the scale of the matrix @æ m.
	void set_scale(Matrix4x4& m, const Vector3& s);

	/// Returns the pointer to the matrix's data
	float* to_float_ptr(Matrix4x4& m);

	/// Returns the pointer to the first elemento of the matrix @a m.
	const float* to_float_ptr(const Matrix4x4& m);

	/// Returns the rotation portion of the matrix @a m as a Matrix3x3.
	Matrix3x3 to_matrix3x3(const Matrix4x4& m);
} // namespace matrix4x4

inline Matrix4x4 operator+(Matrix4x4 a, const Matrix4x4& b)
{
	a += b;
	return a;
}

inline Matrix4x4 operator-(Matrix4x4 a, const Matrix4x4& b)
{
	a -= b;
	return a;
}

inline Matrix4x4 operator*(Matrix4x4 a, float k)
{
	a *= k;
	return a;
}

inline Matrix4x4 operator*(float k, Matrix4x4 a)
{
	a *= k;
	return a;
}

inline Matrix4x4 operator/(Matrix4x4 a, float k)
{
	a /= k;
	return a;
}

inline Vector3 operator*(const Vector3& v, const Matrix4x4& a)
{
	return Vector3(
		v.x*a.x.x + v.y*a.y.x + v.z*a.z.x + a.t.x,
		v.x*a.x.y + v.y*a.y.y + v.z*a.z.y + a.t.y,
		v.x*a.x.z + v.y*a.y.z + v.z*a.z.z + a.t.z
	);
}

inline Vector4 operator*(const Vector4& v, const Matrix4x4& a)
{
	return Vector4(
		v.x*a.x.x + v.y*a.y.x + v.z*a.z.x + v.w*a.t.x,
		v.x*a.x.y + v.y*a.y.y + v.z*a.z.y + v.w*a.t.y,
		v.x*a.x.z + v.y*a.y.z + v.z*a.z.z + v.w*a.t.z,
		v.x*a.x.w + v.y*a.y.w + v.z*a.z.w + v.w*a.t.w
	);
}

inline Matrix4x4 operator*(Matrix4x4 a, const Matrix4x4& b)
{
	a *= b;
	return a;
}

namespace matrix4x4
{
	inline void set_perspective(Matrix4x4& m, float fovy, float aspect, float near, float far)
	{
		const float height = 1.0f / tan(fovy * ((float) PI / 180.0f) * 0.5f);
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

	inline Matrix4x4 get_transposed(Matrix4x4 m)
	{
		transpose(m);
		return m;
	}

	inline void set_look(Matrix4x4& m, const Vector3& pos, const Vector3& target, const Vector3& up)
	{
		Vector3 zAxis = pos - target;
		vector3::normalize(zAxis);
		const Vector3 xAxis = vector3::cross(up, zAxis);
		const Vector3 yAxis = vector3::cross(zAxis, xAxis);

		m.x.x= xAxis.x;
		m.x.y= yAxis.x;
		m.x.z= zAxis.x;
		m.x.w= 0;

		m.y.x= xAxis.y;
		m.y.y= yAxis.y;
		m.y.z= zAxis.y;
		m.y.w= 0;

		m.z.x= xAxis.z;
		m.z.y= yAxis.z;
		m.z.z= zAxis.z;
		m.z.w= 0;

		m.t.x= -vector3::dot(pos, xAxis);
		m.t.y= -vector3::dot(pos, yAxis);
		m.t.z= -vector3::dot(pos, zAxis);
		m.t.w= 1;
	}

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

	inline Matrix4x4 get_inverted(Matrix4x4 m)
	{
		invert(m);
		return m;
	}

	inline void set_identity(Matrix4x4& m)
	{
		m.x = Vector4(1, 0, 0, 0);
		m.y = Vector4(0, 1, 0, 0);
		m.z = Vector4(0, 0, 1, 0);
		m.t = Vector4(0, 0, 0, 1);
	}

	inline Vector3 x(const Matrix4x4& m)
	{
		return Vector3(m.x.x, m.x.y, m.x.z);
	}

	inline Vector3 y(const Matrix4x4& m)
	{
		return Vector3(m.y.x, m.y.y, m.y.z);
	}

	inline Vector3 z(const Matrix4x4& m)
	{
		return Vector3(m.z.x, m.z.y, m.z.z);
	}

	inline void set_x(Matrix4x4& m, const Vector3& x)
	{
		m.x.x = x.x;
		m.x.y = x.y;
		m.x.z = x.z;
	}

	inline void set_y(Matrix4x4& m, const Vector3& y)
	{
		m.y.x = y.x;
		m.y.y = y.y;
		m.y.z = y.z;
	}

	inline void set_z(Matrix4x4& m, const Vector3& z)
	{
		m.z.x = z.x;
		m.z.y = z.y;
		m.z.z = z.z;
	}

	inline Vector3 translation(const Matrix4x4& m)
	{
		return Vector3(m.t.x, m.t.y, m.t.z);
	}

	inline void set_translation(Matrix4x4& m, const Vector3& trans)
	{
		m.t.x = trans.x;
		m.t.y = trans.y;
		m.t.z = trans.z;
	}

	inline Quaternion rotation(const Matrix4x4& m)
	{
		return matrix3x3::rotation(to_matrix3x3(m));
	}

	inline void set_rotation(Matrix4x4& m, const Quaternion& rot)
	{
		set_rotation(m, Matrix3x3(rot));
	}

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

	inline Vector3 scale(const Matrix4x4& m)
	{
		using namespace vector4;
		using namespace vector3;
		const float sx = length(to_vector3(m.x));
		const float sy = length(to_vector3(m.y));
		const float sz = length(to_vector3(m.z));
		return Vector3(sx, sy, sz);
	}

	inline void set_scale(Matrix4x4& m, const Vector3& s)
	{
		Matrix3x3 rot = to_matrix3x3(m);
		matrix3x3::set_scale(rot, s);
		set_rotation(m, rot);
	}

	inline float* to_float_ptr(Matrix4x4& m)
	{
		return vector4::to_float_ptr(m.x);
	}

	inline const float* to_float_ptr(const Matrix4x4& m)
	{
		return vector4::to_float_ptr(m.x);
	}

	inline Matrix3x3 to_matrix3x3(const Matrix4x4& m)
	{
		return Matrix3x3(x(m), y(m), z(m));
	}
} // namespace matrix4x4

inline Matrix4x4::Matrix4x4()
{
	// Do not initialize
}

inline Matrix4x4::Matrix4x4(const Vector3& x, const Vector3& y, const Vector3& z, const Vector3& t)
	: x(x, 0), y(y, 0), z(z, 0), t(t, 1)
{
}

inline Matrix4x4::Matrix4x4(float r1c1, float r2c1, float r3c1, float r4c1,
							float r1c2, float r2c2, float r3c2, float r4c2,
							float r1c3, float r2c3, float r3c3, float r4c3,
							float r1c4, float r2c4, float r3c4, float r4c4)
	: x(r1c1, r2c1, r3c1, r4c1)
	, y(r1c2, r2c2, r3c2, r4c2)
	, z(r1c3, r2c3, r3c3, r4c3)
	, t(r1c4, r2c4, r3c4, r4c4)
{
}

inline Matrix4x4::Matrix4x4(const Quaternion& r, const Vector3& p)
	: x(1.0f - 2.0f * r.y * r.y - 2.0f * r.z * r.z, 2.0f * r.x * r.y + 2.0f * r.w * r.z, 2.0f * r.x * r.z - 2.0f * r.w * r.y, 0)
	, y(2.0f * r.x * r.y - 2.0f * r.w * r.z, 1.0f - 2.0f * r.x * r.x - 2.0f * r.z * r.z, 2.0f * r.y * r.z + 2.0f * r.w * r.x, 0)
	, z(2.0f * r.x * r.z + 2.0f * r.w * r.y, 2.0f * r.y * r.z - 2.0f * r.w * r.x, 1.0f - 2.0f * r.x * r.x - 2.0f * r.y * r.y, 0)
	, t(p, 1)
{
}

inline Matrix4x4::Matrix4x4(const Matrix3x3& m)
	: x(m.x, 0)
	, y(m.y, 0)
	, z(m.z, 0)
	, t(0, 0, 0, 1)
{
}

inline Matrix4x4::Matrix4x4(const float v[16])
	: x(v[0], v[1], v[2], v[3])
	, y(v[4], v[5], v[6], v[7])
	, z(v[8], v[9], v[10], v[11])
	, t(v[12], v[13], v[14], v[15])
{
}

inline float& Matrix4x4::operator[](uint32_t i)
{
	CE_ASSERT(i < 16, "Index out of bounds");

	return vector4::to_float_ptr(x)[i];
}

inline const float& Matrix4x4::operator[](uint32_t i) const
{
	CE_ASSERT(i < 16, "Index out of bounds");

	return vector4::to_float_ptr(x)[i];
}

inline Matrix4x4& Matrix4x4::operator+=(const Matrix4x4& a)
{
	x += a.x;
	y += a.y;
	z += a.z;
	t += a.t;

	return *this;
}

inline Matrix4x4& Matrix4x4::operator-=(const Matrix4x4& a)
{
	x -= a.x;
	y -= a.y;
	z -= a.z;
	t -= a.t;

	return *this;
}

inline Matrix4x4& Matrix4x4::operator*=(float k)
{
	x *= k;
	y *= k;
	z *= k;
	t *= k;

	return *this;
}

inline Matrix4x4& Matrix4x4::operator/=(float k)
{
	const float inv_k = 1.0f / k;

	x *= inv_k;
	y *= inv_k;
	z *= inv_k;
	t *= inv_k;

	return *this;
}

inline Matrix4x4& Matrix4x4::operator*=(const Matrix4x4& a)
{
	Matrix4x4 tmp(
		x.x*a.x.x + x.y*a.y.x + x.z*a.z.x + x.w*a.t.x,
		x.x*a.x.y + x.y*a.y.y + x.z*a.z.y + x.w*a.t.y,
		x.x*a.x.z + x.y*a.y.z + x.z*a.z.z + x.w*a.t.z,
		x.x*a.x.w + x.y*a.y.w + x.z*a.z.w + x.w*a.t.w,

		y.x*a.x.x + y.y*a.y.x + y.z*a.z.x + y.w*a.t.x,
		y.x*a.x.y + y.y*a.y.y + y.z*a.z.y + y.w*a.t.y,
		y.x*a.x.z + y.y*a.y.z + y.z*a.z.z + y.w*a.t.z,
		y.x*a.x.w + y.y*a.y.w + y.z*a.z.w + y.w*a.t.w,

		z.x*a.x.x + z.y*a.y.x + z.z*a.z.x + z.w*a.t.x,
		z.x*a.x.y + z.y*a.y.y + z.z*a.z.y + z.w*a.t.y,
		z.x*a.x.z + z.y*a.y.z + z.z*a.z.z + z.w*a.t.z,
		z.x*a.x.w + z.y*a.y.w + z.z*a.z.w + z.w*a.t.w,

		t.x*a.x.x + t.y*a.y.x + t.z*a.z.x + t.w*a.t.x,
		t.x*a.x.y + t.y*a.y.y + t.z*a.z.y + t.w*a.t.y,
		t.x*a.x.z + t.y*a.y.z + t.z*a.z.z + t.w*a.t.z,
		t.x*a.x.w + t.y*a.y.w + t.z*a.z.w + t.w*a.t.w
	);

	*this = tmp;
	return *this;
}

} // namespace crown
