/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "math_types.h"
#include "vector3.h"
#include "assert.h"

namespace crown
{

/// Adds the matrix @a a to @a b and returns the result.
Matrix3x3 operator+(Matrix3x3 a, const Matrix3x3& b);

/// Subtracts the matrix @a b from @a a and returns the result.
Matrix3x3 operator-(Matrix3x3 a, const Matrix3x3& b);

/// Multiplies the matrix @a a by the scalar @a k and returns the result.
Matrix3x3 operator*(Matrix3x3 a, float k);

/// @copydoc operator*(Matrix3x3, float)
Matrix3x3 operator*(float k, Matrix3x3 a);

/// Divides the matrix @a a by the scalar @a k.
Matrix3x3 operator/(Matrix3x3 a, float k);

/// Multiplies the matrix @a a by the vector @a v and returns the result.
Vector3 operator*(const Vector3& v, const Matrix3x3& a);

/// Multiplies the matrix @a a by @a b and returns the result. (i.e. transforms first by @a a then by @a b)
Matrix3x3 operator*(Matrix3x3 a, const Matrix3x3& b);

/// Functions to manipulate Matrix3x3
///
/// @ingroup Math
namespace matrix3x3
{
	const Matrix3x3 IDENTITY = Matrix3x3(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);

	/// Transposes the matrix @a m and returns the result.
	Matrix3x3& transpose(Matrix3x3& m);

	/// Returns the transposed of the matrix @a m.
	Matrix3x3 get_transposed(Matrix3x3 m);

	/// Returns the determinant of the matrix @a m.
	float determinant(const Matrix3x3& m);

	/// Inverts the matrix @a m and returns the result.
	Matrix3x3& invert(Matrix3x3& m);

	/// Returns the inverse of the matrix @a m.
	Matrix3x3 get_inverted(Matrix3x3 m);

	/// Sets the matrix @a m to identity.
	void set_identity(Matrix3x3& m);

	/// Returns the rotation portion of the matrix @a m as a Quaternion.
	Quaternion rotation(const Matrix3x3& m);

	/// Returns the pointer to the matrix's data
	float* to_float_ptr(Matrix3x3& m);

	/// Returns the pointer to the matrix's data
	const float* to_float_ptr(const Matrix3x3& m);

	/// Returns a 4x4 matrix according to the matrix's rotation portion
	Matrix4x4 to_matrix4x4(const Matrix3x3& m);
} // namespace matrix3x3

inline Matrix3x3 operator+(Matrix3x3 a, const Matrix3x3& b)
{
	a += b;
	return a;
}

inline Matrix3x3 operator-(Matrix3x3 a, const Matrix3x3& b)
{
	a -= b;
	return a;
}

inline Matrix3x3 operator*(Matrix3x3 a, float k)
{
	a *= k;
	return a;
}

inline Matrix3x3 operator*(float k, Matrix3x3 a)
{
	a *= k;
	return a;
}

inline Matrix3x3 operator/(Matrix3x3 a, float k)
{
	a /= k;
	return a;
}

inline Vector3 operator*(const Vector3& v, const Matrix3x3& a)
{
	return Vector3(
		v.x*a.x.x + v.y*a.y.x + v.z*a.z.x,
		v.x*a.x.y + v.y*a.y.y + v.z*a.z.y,
		v.x*a.x.z + v.y*a.y.z + v.z*a.z.z
	);
}

inline Matrix3x3 operator*(Matrix3x3 a, const Matrix3x3& b)
{
	a *= b;
	return a;
}

namespace matrix3x3
{
	inline Matrix3x3& transpose(Matrix3x3& m)
	{
		float tmp;

		tmp = m.x.y;
		m.x.y = m.y.x;
		m.y.x = tmp;

		tmp = m.x.z;
		m.x.z = m.z.x;
		m.z.x = tmp;

		tmp = m.y.z;
		m.y.z = m.z.y;
		m.z.y = tmp;

		return m;
	}

	inline Matrix3x3 get_transposed(Matrix3x3 m)
	{
		transpose(m);
		return m;
	}

	inline float determinant(const Matrix3x3& m)
	{
		return	m.x.x * (m.y.y * m.z.z - m.z.y * m.y.z) -
				m.y.x * (m.x.y * m.z.z - m.z.y * m.x.z) +
				m.z.x * (m.x.y * m.y.z - m.y.y * m.x.z);
	}

	inline Matrix3x3& invert(Matrix3x3& m)
	{
		Matrix3x3 mat;

		mat.x.x = (m.y.y * m.z.z - m.z.y * m.y.z);
		mat.x.y = (m.x.y * m.z.z - m.z.y * m.x.z);
		mat.x.z = (m.x.y * m.y.z - m.y.y * m.x.z);

		const float inv_det = 1.0f / (m.x.x * mat.x.x - m.y.x * mat.x.y + m.z.x * mat.x.z);

		mat.y.x = (m.y.x * m.z.z - m.z.x * m.y.z);
		mat.y.y = (m.x.x * m.z.z - m.z.x * m.x.z);
		mat.y.z = (m.x.x * m.y.z - m.y.x * m.x.z);
		mat.z.x = (m.y.x * m.z.y - m.z.x * m.y.y);
		mat.z.y = (m.x.x * m.z.y - m.z.x * m.x.y);
		mat.z.z = (m.x.x * m.y.y - m.y.x * m.x.y);

		m.x.x = + mat.x.x * inv_det;
		m.x.y = - mat.x.y * inv_det;
		m.x.z = + mat.x.z * inv_det;
		m.y.x = - mat.y.x * inv_det;
		m.y.y = + mat.y.y * inv_det;
		m.y.z = - mat.y.z * inv_det;
		m.z.x = + mat.z.x * inv_det;
		m.z.y = - mat.z.y * inv_det;
		m.z.z = + mat.z.z * inv_det;

		return m;
	}

	inline Matrix3x3 get_inverted(Matrix3x3 m)
	{
		invert(m);
		return m;
	}

	inline void set_identity(Matrix3x3& m)
	{
		m.x = Vector3(1, 0, 0);
		m.y = Vector3(0, 1, 0);
		m.z = Vector3(0, 0, 1);
	}

	inline Matrix4x4 to_matrix4x4(const Matrix3x3& m)
	{
		return Matrix4x4(m);
	}

	inline Quaternion rotation(const Matrix3x3& m)
	{
		const float ww = m.x.x + m.y.y + m.z.z;
		const float xx = m.x.x - m.y.y - m.z.z;
		const float yy = m.y.y - m.x.x - m.z.z;
		const float zz = m.z.z - m.x.x - m.y.y;
		float max = ww;
		uint32_t index = 0;

		if (xx > max)
		{
			max = xx;
			index = 1;
		}

		if (yy > max)
		{
			max = yy;
			index = 2;
		}

		if (zz > max)
		{
			max = zz;
			index = 3;
		}

		const float biggest = math::sqrt(max + 1.0f) * 0.5f;
		const float mult = 0.25f / biggest;

		Quaternion tmp;
		switch (index)
		{
			case 0:
			{
				printf("case=0\n");
				tmp.w = biggest;
				tmp.x = (m.y.z - m.z.y) * mult;
				tmp.y = (m.z.x - m.x.z) * mult;
				tmp.z = (m.x.y - m.y.x) * mult;
				break;
			}
			case 1:
			{
				printf("case=1\n");
				tmp.x = biggest;
				tmp.w = (m.y.z - m.z.y) * mult;
				tmp.y = (m.x.y + m.y.x) * mult;
				tmp.z = (m.z.x + m.x.z) * mult;
				break;
			}
			case 2:
			{
				printf("case=2\n");
				tmp.y = biggest;
				tmp.w = (m.z.x - m.x.z) * mult;
				tmp.x = (m.x.y + m.y.x) * mult;
				tmp.z = (m.y.z + m.z.y) * mult;
				break;
			}
			case 3:
			{
				printf("case=3\n");
				tmp.z = biggest;
				tmp.w = (m.x.y - m.y.x) * mult;
				tmp.x = (m.z.x + m.x.z) * mult;
				tmp.y = (m.y.z + m.z.y) * mult;
				break;
			}
			default:
			{
				CE_FATAL("You should not be here");
				break;
			}
		}

		return tmp;
	}
} // namespace matrix3x3

inline Matrix3x3::Matrix3x3()
{
	// Do not initialize
}

inline Matrix3x3::Matrix3x3(const Vector3& x, const Vector3& y, const Vector3& z)
	: x(x)
	, y(y)
	, z(z)
{
}

inline Matrix3x3::Matrix3x3(const Quaternion& r)
	: x(1.0f - 2.0f * r.y * r.y - 2.0f * r.z * r.z, 2.0f * r.x * r.y + 2.0f * r.w * r.z, 2.0f * r.x * r.z - 2.0f * r.w * r.y)
	, y(2.0f * r.x * r.y - 2.0f * r.w * r.z, 1.0f - 2.0f * r.x * r.x - 2.0f * r.z * r.z, 2.0f * r.y * r.z + 2.0f * r.w * r.x)
	, z(2.0f * r.x * r.z + 2.0f * r.w * r.y, 2.0f * r.y * r.z - 2.0f * r.w * r.x, 1.0f - 2.0f * r.x * r.x - 2.0f * r.y * r.y)
{
}

inline Matrix3x3::Matrix3x3(float r1c1, float r2c1, float r3c1, float r1c2, float r2c2, float r3c2,
						float r1c3, float r2c3, float r3c3)
	: x(r1c1, r2c1, r3c1)
	, y(r1c2, r2c2, r3c2)
	, z(r1c3, r2c3, r3c3)
{
}

inline Matrix3x3::Matrix3x3(const float v[9])
	: x(v[0], v[1], v[2])
	, y(v[3], v[4], v[5])
	, z(v[6], v[7], v[8])
{
}

inline float& Matrix3x3::operator[](uint32_t i)
{
	CE_ASSERT(i < 9, "Index out of bounds");

	return vector3::to_float_ptr(x)[i];
}

inline const float& Matrix3x3::operator[](uint32_t i) const
{
	CE_ASSERT(i < 9, "Index out of bounds");

	return vector3::to_float_ptr(x)[i];
}

inline Matrix3x3& Matrix3x3::operator+=(const Matrix3x3& a)
{
	x += a.x;
	y += a.y;
	z += a.z;

	return *this;
}

inline Matrix3x3& Matrix3x3::operator-=(const Matrix3x3& a)
{
	x -= a.x;
	y -= a.y;
	z -= a.z;

	return *this;
}

inline Matrix3x3& Matrix3x3::operator*=(float k)
{
	x *= k;
	y *= k;
	z *= k;

	return *this;
}

inline Matrix3x3& Matrix3x3::operator/=(float k)
{
	const float inv_k = 1.0f / k;

	x *= inv_k;
	y *= inv_k;
	z *= inv_k;

	return *this;
}

inline Matrix3x3& Matrix3x3::operator*=(const Matrix3x3& a)
{
	Matrix3x3 tmp(
		x.x*a.x.x + x.y*a.y.x + x.z*a.z.x,
		x.x*a.x.y + x.y*a.y.y + x.z*a.z.y,
		x.x*a.x.z + x.y*a.y.z + x.z*a.z.z,

		y.x*a.x.x + y.y*a.y.x + y.z*a.z.x,
		y.x*a.x.y + y.y*a.y.y + y.z*a.z.y,
		y.x*a.x.z + y.y*a.y.z + y.z*a.z.z,

		z.x*a.x.x + z.y*a.y.x + z.z*a.z.x,
		z.x*a.x.y + z.y*a.y.y + z.z*a.z.y,
		z.x*a.x.z + z.y*a.y.z + z.z*a.z.z
	);

	*this = tmp;
	return *this;
}

} // namespace crown
