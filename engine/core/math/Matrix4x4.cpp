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

#include "Assert.h"
#include "Matrix3x3.h"
#include "Types.h"
#include "Matrix4x4.h"
#include "MathUtils.h"
#include "Quaternion.h"
#include "Vector3.h"
#include "Vector4.h"

namespace crown
{

const Matrix4x4 Matrix4x4::IDENTITY = Matrix4x4(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0);

//-----------------------------------------------------------------------------
Matrix4x4::Matrix4x4()
{
}

//-----------------------------------------------------------------------------
Matrix4x4::Matrix4x4(float r1c1, float r2c1, float r3c1, float r4c1, float r1c2, float r2c2, float r3c2, float r4c2,
	float r1c3, float r2c3, float r3c3, float r4c3, float r1c4, float r2c4, float r3c4, float r4c4)
{
	m[0] = r1c1;
	m[1] = r2c1;
	m[2] = r3c1;
	m[3] = r4c1;
	m[4] = r1c2;
	m[5] = r2c2;
	m[6] = r3c2;
	m[7] = r4c2;
	m[8] = r1c3;
	m[9] = r2c3;
	m[10] = r3c3;
	m[11] = r4c3;
	m[12] = r1c4;
	m[13] = r2c4;
	m[14] = r3c4;
	m[15] = r4c4;
}

//-----------------------------------------------------------------------------
Matrix4x4::Matrix4x4(const float v[16])
{
	m[0] = v[0];
	m[1] = v[1];
	m[2] = v[2];
	m[3] = v[3];
	m[4] = v[4];
	m[5] = v[5];
	m[6] = v[6];
	m[7] = v[7];
	m[8] = v[8];
	m[9] = v[9];
	m[10] = v[10];
	m[11] = v[11];
	m[12] = v[12];
	m[13] = v[13];
	m[14] = v[14];
	m[15] = v[15];
}

//-----------------------------------------------------------------------------
Matrix4x4::Matrix4x4(const Matrix4x4& a)
{
	m[0] = a.m[0];
	m[1] = a.m[1];
	m[2] = a.m[2];
	m[3] = a.m[3];
	m[4] = a.m[4];
	m[5] = a.m[5];
	m[6] = a.m[6];
	m[7] = a.m[7];
	m[8] = a.m[8];
	m[9] = a.m[9];
	m[10] = a.m[10];
	m[11] = a.m[11];
	m[12] = a.m[12];
	m[13] = a.m[13];
	m[14] = a.m[14];
	m[15] = a.m[15];
}

//-----------------------------------------------------------------------------
Matrix4x4& Matrix4x4::operator=(const Matrix4x4& a)
{
	m[0] = a.m[0];
	m[1] = a.m[1];
	m[2] = a.m[2];
	m[3] = a.m[3];
	m[4] = a.m[4];
	m[5] = a.m[5];
	m[6] = a.m[6];
	m[7] = a.m[7];
	m[8] = a.m[8];
	m[9] = a.m[9];
	m[10] = a.m[10];
	m[11] = a.m[11];
	m[12] = a.m[12];
	m[13] = a.m[13];
	m[14] = a.m[14];
	m[15] = a.m[15];

	return *this;
}

//-----------------------------------------------------------------------------
float Matrix4x4::operator[](uint32_t i) const
{
	CE_ASSERT(i < 16, "Index must be < 16");

	return m[i];
}

//-----------------------------------------------------------------------------
float& Matrix4x4::operator[](uint32_t i)
{
	CE_ASSERT(i < 16, "Index must be < 16");

	return m[i];
}

//-----------------------------------------------------------------------------
float Matrix4x4::operator()(uint32_t row, uint32_t column) const
{
	CE_ASSERT(row < 4 && column < 4, "Row and column must be < 4");

	return m[row + column * 4];
}

//-----------------------------------------------------------------------------
Matrix4x4 Matrix4x4::operator+(const Matrix4x4& a) const
{
	Matrix4x4 tmp;

	tmp.m[0] = m[0] + a.m[0];
	tmp.m[1] = m[1] + a.m[1];
	tmp.m[2] = m[2] + a.m[2];
	tmp.m[3] = m[3] + a.m[3];
	tmp.m[4] = m[4] + a.m[4];
	tmp.m[5] = m[5] + a.m[5];
	tmp.m[6] = m[6] + a.m[6];
	tmp.m[7] = m[7] + a.m[7];
	tmp.m[8] = m[8] + a.m[8];
	tmp.m[9] = m[9] + a.m[9];
	tmp.m[10] = m[10] + a.m[10];
	tmp.m[11] = m[11] + a.m[11];
	tmp.m[12] = m[12] + a.m[12];
	tmp.m[13] = m[13] + a.m[13];
	tmp.m[14] = m[14] + a.m[14];
	tmp.m[14] = m[15] + a.m[15];

	return tmp;
}

//-----------------------------------------------------------------------------
Matrix4x4& Matrix4x4::operator+=(const Matrix4x4& a)
{
	m[0] = m[0] + a.m[0];
	m[1] = m[1] + a.m[1];
	m[2] = m[2] + a.m[2];
	m[3] = m[3] + a.m[3];
	m[4] = m[4] + a.m[4];
	m[5] = m[5] + a.m[5];
	m[6] = m[6] + a.m[6];
	m[7] = m[7] + a.m[7];
	m[8] = m[8] + a.m[8];
	m[9] = m[9] + a.m[9];
	m[10] = m[10] + a.m[10];
	m[11] = m[11] + a.m[11];
	m[12] = m[12] + a.m[12];
	m[13] = m[13] + a.m[13];
	m[14] = m[14] + a.m[14];
	m[14] = m[15] + a.m[15];

	return *this;
}

//-----------------------------------------------------------------------------
Matrix4x4 Matrix4x4::operator-(const Matrix4x4& a) const
{
	Matrix4x4 tmp;

	tmp.m[0] = m[0] - a.m[0];
	tmp.m[1] = m[1] - a.m[1];
	tmp.m[2] = m[2] - a.m[2];
	tmp.m[3] = m[3] - a.m[3];
	tmp.m[4] = m[4] - a.m[4];
	tmp.m[5] = m[5] - a.m[5];
	tmp.m[6] = m[6] - a.m[6];
	tmp.m[7] = m[7] - a.m[7];
	tmp.m[8] = m[8] - a.m[8];
	tmp.m[9] = m[9] - a.m[9];
	tmp.m[10] = m[10] - a.m[10];
	tmp.m[11] = m[11] - a.m[11];
	tmp.m[12] = m[12] - a.m[12];
	tmp.m[13] = m[13] - a.m[13];
	tmp.m[14] = m[14] - a.m[14];
	tmp.m[14] = m[15] - a.m[15];

	return tmp;
}

//-----------------------------------------------------------------------------
Matrix4x4& Matrix4x4::operator-=(const Matrix4x4& a)
{
	m[0] = m[0] - a.m[0];
	m[1] = m[1] - a.m[1];
	m[2] = m[2] - a.m[2];
	m[3] = m[3] - a.m[3];
	m[4] = m[4] - a.m[4];
	m[5] = m[5] - a.m[5];
	m[6] = m[6] - a.m[6];
	m[7] = m[7] - a.m[7];
	m[8] = m[8] - a.m[8];
	m[9] = m[9] - a.m[9];
	m[10] = m[10] - a.m[10];
	m[11] = m[11] - a.m[11];
	m[12] = m[12] - a.m[12];
	m[13] = m[13] - a.m[13];
	m[14] = m[14] - a.m[14];
	m[14] = m[15] - a.m[15];

	return *this;
}

//-----------------------------------------------------------------------------
Matrix4x4 Matrix4x4::operator*(float k) const
{
	Matrix4x4 tmp;

	tmp.m[0] = m[0] * k;
	tmp.m[1] = m[1] * k;
	tmp.m[2] = m[2] * k;
	tmp.m[3] = m[3] * k;
	tmp.m[4] = m[4] * k;
	tmp.m[5] = m[5] * k;
	tmp.m[6] = m[6] * k;
	tmp.m[7] = m[7] * k;
	tmp.m[8] = m[8] * k;
	tmp.m[9] = m[9] * k;
	tmp.m[10] = m[10] * k;
	tmp.m[11] = m[11] * k;
	tmp.m[12] = m[12] * k;
	tmp.m[13] = m[13] * k;
	tmp.m[14] = m[14] * k;
	tmp.m[15] = m[15] * k;

	return tmp;
}

//-----------------------------------------------------------------------------
Matrix4x4& Matrix4x4::operator*=(float k)
{
	m[0] *= k;
	m[1] *= k;
	m[2] *= k;
	m[3] *= k;
	m[4] *= k;
	m[5] *= k;
	m[6] *= k;
	m[7] *= k;
	m[8] *= k;
	m[9] *= k;
	m[10] *= k;
	m[11] *= k;
	m[12] *= k;
	m[13] *= k;
	m[14] *= k;
	m[15] *= k;

	return *this;
}

//-----------------------------------------------------------------------------
Matrix4x4 Matrix4x4::operator/(float k) const
{
	Matrix4x4 tmp;

	k = (float)1.0 / k;

	tmp.m[0] = m[0] * k;
	tmp.m[1] = m[1] * k;
	tmp.m[2] = m[2] * k;
	tmp.m[3] = m[3] * k;
	tmp.m[4] = m[4] * k;
	tmp.m[5] = m[5] * k;
	tmp.m[6] = m[6] * k;
	tmp.m[7] = m[7] * k;
	tmp.m[8] = m[8] * k;
	tmp.m[9] = m[9] * k;
	tmp.m[10] = m[10] * k;
	tmp.m[11] = m[11] * k;
	tmp.m[12] = m[12] * k;
	tmp.m[13] = m[13] * k;
	tmp.m[14] = m[14] * k;
	tmp.m[15] = m[15] * k;

	return tmp;
}

//-----------------------------------------------------------------------------
Matrix4x4& Matrix4x4::operator/=(float k)
{
	k = (float)1.0 / k;

	m[0] *= k;
	m[1] *= k;
	m[2] *= k;
	m[3] *= k;
	m[4] *= k;
	m[5] *= k;
	m[6] *= k;
	m[7] *= k;
	m[8] *= k;
	m[9] *= k;
	m[10] *= k;
	m[11] *= k;
	m[12] *= k;
	m[13] *= k;
	m[14] *= k;
	m[15] *= k;

	return *this;
}

//-----------------------------------------------------------------------------
Vector3 Matrix4x4::operator*(const Vector3& v) const
{
	Vector3 tmp;

	tmp.x = m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12];
	tmp.y = m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13];
	tmp.z = m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14];

	return tmp;
}

//-----------------------------------------------------------------------------
Vector4 Matrix4x4::operator*(const Vector4& v) const
{
	Vector4 tmp;

	tmp.x = m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12] * v.w;
	tmp.y = m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13] * v.w;
	tmp.z = m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14] * v.w;
	tmp.w = m[3] * v.x + m[7] * v.y + m[11] * v.z + m[15] * v.w;

	return tmp;
}

//-----------------------------------------------------------------------------
Matrix4x4 Matrix4x4::operator*(const Matrix4x4& a) const
{
	Matrix4x4 tmp;

	tmp.m[0] = m[0] * a.m[0] + m[4] * a.m[1] + m[8] * a.m[2] + m[12] * a.m[3];
	tmp.m[1] = m[1] * a.m[0] + m[5] * a.m[1] + m[9] * a.m[2] + m[13] * a.m[3];
	tmp.m[2] = m[2] * a.m[0] + m[6] * a.m[1] + m[10] * a.m[2] + m[14] * a.m[3];
	tmp.m[3] = m[3] * a.m[0] + m[7] * a.m[1] + m[11] * a.m[2] + m[15] * a.m[3];

	tmp.m[4] = m[0] * a.m[4] + m[4] * a.m[5] + m[8] * a.m[6] + m[12] * a.m[7];
	tmp.m[5] = m[1] * a.m[4] + m[5] * a.m[5] + m[9] * a.m[6] + m[13] * a.m[7];
	tmp.m[6] = m[2] * a.m[4] + m[6] * a.m[5] + m[10] * a.m[6] + m[14] * a.m[7];
	tmp.m[7] = m[3] * a.m[4] + m[7] * a.m[5] + m[11] * a.m[6] + m[15] * a.m[7];

	tmp.m[8] = m[0] * a.m[8] + m[4] * a.m[9] + m[8] * a.m[10] + m[12] * a.m[11];
	tmp.m[9] = m[1] * a.m[8] + m[5] * a.m[9] + m[9] * a.m[10] + m[13] * a.m[11];
	tmp.m[10] = m[2] * a.m[8] + m[6] * a.m[9] + m[10] * a.m[10] + m[14] * a.m[11];
	tmp.m[11] = m[3] * a.m[8] + m[7] * a.m[9] + m[11] * a.m[10] + m[15] * a.m[11];

	tmp.m[12] = m[0] * a.m[12] + m[4] * a.m[13] + m[8] * a.m[14] + m[12] * a.m[15];
	tmp.m[13] = m[1] * a.m[12] + m[5] * a.m[13] + m[9] * a.m[14] + m[13] * a.m[15];
	tmp.m[14] = m[2] * a.m[12] + m[6] * a.m[13] + m[10] * a.m[14] + m[14] * a.m[15];
	tmp.m[15] = m[3] * a.m[12] + m[7] * a.m[13] + m[11] * a.m[14] + m[15] * a.m[15];

	return tmp;
}

//-----------------------------------------------------------------------------
Matrix4x4& Matrix4x4::operator*=(const Matrix4x4& a)
{
	Matrix4x4 tmp;

	tmp.m[0] = m[0] * a.m[0] + m[4] * a.m[1] + m[8] * a.m[2] + m[12] * a.m[3];
	tmp.m[1] = m[1] * a.m[0] + m[5] * a.m[1] + m[9] * a.m[2] + m[13] * a.m[3];
	tmp.m[2] = m[2] * a.m[0] + m[6] * a.m[1] + m[10] * a.m[2] + m[14] * a.m[3];
	tmp.m[3] = m[3] * a.m[0] + m[7] * a.m[1] + m[11] * a.m[2] + m[15] * a.m[3];

	tmp.m[4] = m[0] * a.m[4] + m[4] * a.m[5] + m[8] * a.m[6] + m[12] * a.m[7];
	tmp.m[5] = m[1] * a.m[4] + m[5] * a.m[5] + m[9] * a.m[6] + m[13] * a.m[7];
	tmp.m[6] = m[2] * a.m[4] + m[6] * a.m[5] + m[10] * a.m[6] + m[14] * a.m[7];
	tmp.m[7] = m[3] * a.m[4] + m[7] * a.m[5] + m[11] * a.m[6] + m[15] * a.m[7];

	tmp.m[8] = m[0] * a.m[8] + m[4] * a.m[9] + m[8] * a.m[10] + m[12] * a.m[11];
	tmp.m[9] = m[1] * a.m[8] + m[5] * a.m[9] + m[9] * a.m[10] + m[13] * a.m[11];
	tmp.m[10] = m[2] * a.m[8] + m[6] * a.m[9] + m[10] * a.m[10] + m[14] * a.m[11];
	tmp.m[11] = m[3] * a.m[8] + m[7] * a.m[9] + m[11] * a.m[10] + m[15] * a.m[11];

	tmp.m[12] = m[0] * a.m[12] + m[4] * a.m[13] + m[8] * a.m[14] + m[12] * a.m[15];
	tmp.m[13] = m[1] * a.m[12] + m[5] * a.m[13] + m[9] * a.m[14] + m[13] * a.m[15];
	tmp.m[14] = m[2] * a.m[12] + m[6] * a.m[13] + m[10] * a.m[14] + m[14] * a.m[15];
	tmp.m[15] = m[3] * a.m[12] + m[7] * a.m[13] + m[11] * a.m[14] + m[15] * a.m[15];

	*this = tmp;

	return *this;
}

Matrix4x4 operator*(float k, const Matrix4x4& a)
{
	return a * k;
}

//-----------------------------------------------------------------------------
void Matrix4x4::build_rotation_x(float radians)
{
	m[0] = 1.0;
	m[1] = 0.0;
	m[2] = 0.0;
	m[3] = 0.0;
	m[4] = 0.0;
	m[5] = math::cos(radians);
	m[6] = math::sin(radians);
	m[7] = 0.0;
	m[8] = 0.0;
	m[9] = -math::sin(radians);
	m[10] = math::cos(radians);
	m[11] = 0.0;
	m[12] = 0.0;
	m[13] = 0.0;
	m[14] = 0.0;
	m[15] = 1.0;
}

//-----------------------------------------------------------------------------
void Matrix4x4::build_rotation_y(float radians)
{
	m[0] = math::cos(radians);
	m[1] = 0.0;
	m[2] = -math::sin(radians);
	m[3] = 0.0;
	m[4] = 0.0;
	m[5] = 1.0;
	m[6] = 0.0;
	m[7] = 0.0;
	m[8] = math::sin(radians);
	m[9] = 0.0;
	m[10] = math::cos(radians);
	m[11] = 0.0;
	m[12] = 0.0;
	m[13] = 0.0;
	m[14] = 0.0;
	m[15] = 1.0;
}

//-----------------------------------------------------------------------------
void Matrix4x4::build_rotation_z(float radians)
{
	m[0] = math::cos(radians);
	m[1] = math::sin(radians);
	m[2] = 0.0;
	m[3] = 0.0;
	m[4] = -math::sin(radians);
	m[5] = math::cos(radians);
	m[6] = 0.0;
	m[7] = 0.0;
	m[8] = 0.0;
	m[9] = 0.0;
	m[10] = 1.0;
	m[11] = 0.0;
	m[12] = 0.0;
	m[13] = 0.0;
	m[14] = 0.0;
	m[15] = 1.0;
}

//-----------------------------------------------------------------------------
void Matrix4x4::build_rotation(const Vector3& n, float radians)
{
	float a = (float)1.0 - math::cos(radians);
	float sin_a = math::sin(radians);
	float cos_a = math::cos(radians);

	m[0] = n.x * n.x * a + cos_a;
	m[1] = n.x * n.y * a + n.z * sin_a;
	m[2] = n.x * n.z * a - n.y * sin_a;
	m[3] = 0.0;
	m[4] = n.x * n.y * a - n.z * sin_a;
	m[5] = n.y * n.y * a + cos_a;
	m[6] = n.y * n.z * a + n.x * sin_a;
	m[7] = 0.0;
	m[8] = n.x * n.z * a + n.y * sin_a;
	m[9] = n.y * n.z * a - n.x * sin_a;
	m[10] = n.z * n.z * a + cos_a;
	m[11] = 0.0;
	m[12] = 0.0;
	m[13] = 0.0;
	m[14] = 0.0;
	m[15] = 1.0;
}

//-----------------------------------------------------------------------------
void Matrix4x4::build_projection_perspective_rh(float fovy, float aspect, float near, float far)
{
	double top, right;

	top = math::tan((float)((double)fovy / 360.0 * math::PI)) * (double)near;
	right = top * aspect;

	m[0] = (float)(near / right);
	m[1] = 0.0;
	m[2] = 0.0;
	m[3] = 0.0;
	m[4] = 0.0;
	m[5] = (float)(near / top);
	m[6] = 0.0;
	m[7] = 0.0;
	m[8] = 0.0;
	m[9] = 0.0;
	m[10] = (float)((far + near) / (near - far));
	m[11] = -1.0;
	m[12] = 0.0;
	m[13] = 0.0;
	m[14] = (float)((2.0 * far * near) / (near - far));
	m[15] = 0.0;
}

//-----------------------------------------------------------------------------
void Matrix4x4::build_projection_perspective_lh(float fovy, float aspect, float near, float far)
{
	double top, right;

	top = math::tan((float)((double)fovy / 360.0 * math::PI)) * (double)near;
	right = top * aspect;

	m[0] = (float)(near / right);
	m[1] = 0.0;
	m[2] = 0.0;
	m[3] = 0.0;
	m[4] = 0.0;
	m[5] = (float)(near / top);
	m[6] = 0.0;
	m[7] = 0.0;
	m[8] = 0.0;
	m[9] = 0.0;
	m[10] = (float)(far / (far - near));
	m[11] = 1.0;
	m[12] = 0.0;
	m[13] = 0.0;
	m[14] = (float)((far * near) / (near - far));
	m[15] = 0.0;
}

//-----------------------------------------------------------------------------
void Matrix4x4::build_projection_ortho_rh(float width, float height, float near, float far)
{
	m[0] = (float)2.0 / width;
	m[1] = 0.0;
	m[2] = 0.0;
	m[3] = 0.0;
	m[4] = 0.0;
	m[5] = (float)2.0 / height;
	m[6] = 0.0;
	m[7] = 0.0;
	m[8] = 0.0;
	m[9] = 0.0;
	m[10] = (float)2 / (near - far);
	m[11] = 0.0;
	m[12] = 0.0;
	m[13] = 0.0;
	m[14] = (far + near) / (near - far);
	m[15] = 1.0;
}

//-----------------------------------------------------------------------------
void Matrix4x4::build_projection_ortho_lh(float width, float height, float near, float far)
{
	m[0] = (float)2.0 / width;
	m[1] = 0.0;
	m[2] = 0.0;
	m[3] = 0.0;
	m[4] = 0.0;
	m[5] = (float)2.0 / height;
	m[6] = 0.0;
	m[7] = 0.0;
	m[8] = 0.0;
	m[9] = 0.0;
	m[10] = (float)2 / (far - near);
	m[11] = 0.0;
	m[12] = 0.0;
	m[13] = 0.0;
	m[14] = near / (near - far);
	m[15] = (float)1.0;
}

//-----------------------------------------------------------------------------
void Matrix4x4::build_projection_ortho_2d_rh(float width, float height, float near, float far)
{
	m[0] = (float)2.0 / width;
	m[1] = 0.0;
	m[2] = 0.0;
	m[3] = 0.0;
	m[4] = 0.0;
	m[5] = (float)-2.0 / height;
	m[6] = 0.0;
	m[7] = 0.0;
	m[8] = 0.0;
	m[9] = 0.0;
	m[10] = (float)2.0 / (near - far);
	m[11] = 0.0;
	m[12] = (float)-1.0 + (m[0] * (float)0.375);		// Add 0.375 to acheive
	m[13] =  (float)1.0 + (m[5] * (float)0.375);		// pixel-perfect 2d drawing
	m[14] = near / (near - far);
	m[15] = (float)1.0;
}

//-----------------------------------------------------------------------------
Matrix4x4& Matrix4x4::transpose()
{
	float tmp;

	tmp = m[1];
	m[1] = m[4];
	m[4] = tmp;

	tmp = m[2];
	m[2] = m[8];
	m[8] = tmp;

	tmp = m[3];
	m[3] = m[12];
	m[12] = tmp;

	tmp = m[6];
	m[6] = m[9];
	m[9] = tmp;

	tmp = m[7];
	m[7] = m[13];
	m[13] = tmp;

	tmp = m[11];
	m[11] = m[14];
	m[14] = tmp;

	return *this;
}

//-----------------------------------------------------------------------------
Matrix4x4 Matrix4x4::get_transposed() const
{
	Matrix4x4 tmp;

	tmp.m[0] = m[0];
	tmp.m[1] = m[4];
	tmp.m[2] = m[8];
	tmp.m[3] = m[12];
	tmp.m[4] = m[1];
	tmp.m[5] = m[5];
	tmp.m[6] = m[9];
	tmp.m[7] = m[13];
	tmp.m[8] = m[2];
	tmp.m[9] = m[6];
	tmp.m[10] = m[10];
	tmp.m[11] = m[14];
	tmp.m[12] = m[3];
	tmp.m[13] = m[7];
	tmp.m[14] = m[11];
	tmp.m[15] = m[15];

	return tmp;
}

//-----------------------------------------------------------------------------
void Matrix4x4::build_look_at_rh(const Vector3& pos, const Vector3& target, const Vector3& up)
{
	Vector3 zAxis =  pos - target;
	zAxis.normalize();

	Vector3 xAxis = up.cross(zAxis);
	Vector3 yAxis = zAxis.cross(xAxis);

	m[0] = xAxis.x;
	m[1] = yAxis.x;
	m[2] = zAxis.x;
	m[3] = 0.0;
	m[4] = xAxis.y;
	m[5] = yAxis.y;
	m[6] = zAxis.y;
	m[7] = 0.0;
	m[8] = xAxis.z;
	m[9] = yAxis.z;
	m[10] = zAxis.z;
	m[11] = 0.0;
	m[12] = -pos.dot(xAxis);
	m[13] = -pos.dot(yAxis);
	m[14] = -pos.dot(zAxis);
	m[15] = 1.0;
}

//-----------------------------------------------------------------------------
void Matrix4x4::build_look_at_lh(const Vector3& pos, const Vector3& target, const Vector3& up)
{
	Vector3 zAxis =  target - pos;
	zAxis.normalize();

	Vector3 xAxis = up.cross(zAxis);
	Vector3 yAxis = zAxis.cross(xAxis);

	m[0] = xAxis.x;
	m[1] = yAxis.x;
	m[2] = zAxis.x;
	m[3] = 0.0;
	m[4] = xAxis.y;
	m[5] = yAxis.y;
	m[6] = zAxis.y;
	m[7] = 0.0;
	m[8] = xAxis.z;
	m[9] = yAxis.z;
	m[10] = zAxis.z;
	m[11] = 0.0;
	m[12] = -pos.dot(xAxis);
	m[13] = -pos.dot(yAxis);
	m[14] = -pos.dot(zAxis);
	m[15] = 1.0;
}

//-----------------------------------------------------------------------------
void Matrix4x4::build_viewpoint_billboard(const Vector3& pos, const Vector3& target, const Vector3& up)
{
	Vector3 zAxis = target - pos;
	zAxis.normalize();

	Vector3 xAxis = up.cross(zAxis).normalize();
	Vector3 yAxis = zAxis.cross(xAxis).normalize();

	m[0] = xAxis.x;
	m[1] = xAxis.y;
	m[2] = xAxis.z;
	m[3] = 0.0;
	m[4] = yAxis.x;
	m[5] = yAxis.y;
	m[6] = yAxis.z;
	m[7] = 0.0;
	m[8] = zAxis.x;
	m[9] = zAxis.y;
	m[10] = zAxis.z;
	m[11] = 0.0;
	m[12] = pos.x;
	m[13] = pos.y;
	m[14] = pos.z;
	m[15] = 1.0;
}

//-----------------------------------------------------------------------------
void Matrix4x4::build_axis_billboard(const Vector3& pos, const Vector3& target, const Vector3& axis)
{
	Vector3 zAxis = target - pos;

	Vector3 xAxis = axis.cross(zAxis).normalize();
	zAxis = axis.cross(xAxis).normalize();
	const Vector3& yAxis = axis;

	m[0] = xAxis.x;
	m[1] = xAxis.y;
	m[2] = xAxis.z;
	m[3] = 0.0;
	m[4] = yAxis.x;
	m[5] = yAxis.y;
	m[6] = yAxis.z;
	m[7] = 0.0;
	m[8] = zAxis.x;
	m[9] = zAxis.y;
	m[10] = zAxis.z;
	m[11] = 0.0;
	m[12] = pos.x;
	m[13] = pos.y;
	m[14] = pos.z;
	m[15] = 1.0;
}

//-----------------------------------------------------------------------------
float Matrix4x4::get_determinant() const
{
	float det;

	float m02m07_m06m03 = m[2] * m[7] - m[6] * m[3];
	float m02m11_m10m03 = m[2] * m[11] - m[10] * m[3];
	float m02m15_m14m03 = m[2] * m[15] - m[14] * m[3];
	float m06m11_m10m07 = m[6] * m[11] - m[10] * m[7];
	float m06m15_m14m07 = m[6] * m[15] - m[14] * m[7];
	float m10m15_m14m11 = m[10] * m[15] - m[14] * m[11];

	det = 	+ m[0] * (m[5] * m10m15_m14m11 - m[9] * m06m15_m14m07 + m[13] * m06m11_m10m07)
			- m[4] * (m[1] * m10m15_m14m11 - m[9] * m02m15_m14m03 + m[13] * m02m11_m10m03)
			+ m[8] * (m[1] * m06m15_m14m07 - m[5] * m02m15_m14m03 + m[13] * m02m07_m06m03)
			- m[12] * (m[1] * m06m11_m10m07 - m[5] * m02m11_m10m03 + m[9] * m02m07_m06m03);

	return det;
}

//-----------------------------------------------------------------------------
Matrix4x4& Matrix4x4::invert()
{
	Matrix4x4 mat;
	float det;

	float m01m06_m05m02 = m[1] * m[6] - m[5] * m[2];
	float m01m07_m05m03 = m[1] * m[7] - m[5] * m[3];
	float m01m10_m09m02 = m[1] * m[10] - m[9] * m[2];
	float m01m11_m09m03 = m[1] * m[11] - m[9] * m[3];
	float m01m14_m13m02 = m[1] * m[14] - m[13] * m[2];
	float m01m15_m13m03 = m[1] * m[15] - m[13] * m[3];
	float m02m07_m06m03 = m[2] * m[7] - m[6] * m[3];
	float m02m11_m10m03 = m[2] * m[11] - m[10] * m[3];
	float m02m15_m14m03 = m[2] * m[15] - m[14] * m[3];
	float m05m10_m09m06 = m[5] * m[10] - m[9] * m[6];
	float m05m11_m09m07 = m[5] * m[11] - m[9] * m[7];
	float m05m14_m13m06 = m[5] * m[14] - m[13] * m[6];
	float m05m15_m13m07 = m[5] * m[15] - m[13] * m[7];
	float m06m11_m10m07 = m[6] * m[11] - m[10] * m[7];
	float m06m15_m14m07 = m[6] * m[15] - m[14] * m[7];
	float m09m14_m13m10 = m[9] * m[14] - m[13] * m[10];
	float m09m15_m13m11 = m[9] * m[15] - m[13] * m[11];
	float m10m15_m14m11 = m[10] * m[15] - m[14] * m[11];

	mat.m[0] = (+ m[5] * m10m15_m14m11 - m[9] * m06m15_m14m07 + m[13] * m06m11_m10m07);
	mat.m[1] = (+ m[1] * m10m15_m14m11 - m[9] * m02m15_m14m03 + m[13] * m02m11_m10m03);
	mat.m[2] = (+ m[1] * m06m15_m14m07 - m[5] * m02m15_m14m03 + m[13] * m02m07_m06m03);
	mat.m[3] = (+ m[1] * m06m11_m10m07 - m[5] * m02m11_m10m03 + m[9] * m02m07_m06m03);

	det = m[0] * mat.m[0] - m[4] * mat.m[1] + m[8] * mat.m[2] - m[12] * mat.m[3];
	det = (float)1.0 / det;

	mat.m[4] = (+ m[4] * m10m15_m14m11 - m[8] * m06m15_m14m07 + m[12] * m06m11_m10m07);
	mat.m[5] = (+ m[0] * m10m15_m14m11 - m[8] * m02m15_m14m03 + m[12] * m02m11_m10m03);
	mat.m[6] = (+ m[0] * m06m15_m14m07 - m[4] * m02m15_m14m03 + m[12] * m02m07_m06m03);
	mat.m[7] = (+ m[0] * m06m11_m10m07 - m[4] * m02m11_m10m03 + m[8] * m02m07_m06m03);
	mat.m[8] = (+ m[4] * m09m15_m13m11 - m[8] * m05m15_m13m07 + m[12] * m05m11_m09m07);
	mat.m[9] = (+ m[0] * m09m15_m13m11 - m[8] * m01m15_m13m03 + m[12] * m01m11_m09m03);
	mat.m[10] = (+ m[0] * m05m15_m13m07 - m[4] * m01m15_m13m03 + m[12] * m01m07_m05m03);
	mat.m[11] = (+ m[0] * m05m11_m09m07 - m[4] * m01m11_m09m03 + m[8] * m01m07_m05m03);
	mat.m[12] = (+ m[4] * m09m14_m13m10 - m[8] * m05m14_m13m06 + m[12] * m05m10_m09m06);
	mat.m[13] = (+ m[0] * m09m14_m13m10 - m[8] * m01m14_m13m02 + m[12] * m01m10_m09m02);
	mat.m[14] = (+ m[0] * m05m14_m13m06 - m[4] * m01m14_m13m02 + m[12] * m01m06_m05m02);
	mat.m[15] = (+ m[0] * m05m10_m09m06 - m[4] * m01m10_m09m02 + m[8] * m01m06_m05m02);

	m[0] = + mat.m[0] * det;
	m[1] = - mat.m[1] * det;
	m[2] = + mat.m[2] * det;
	m[3] = - mat.m[3] * det;
	m[4] = - mat.m[4] * det;
	m[5] = + mat.m[5] * det;
	m[6] = - mat.m[6] * det;
	m[7] = + mat.m[7] * det;
	m[8] = + mat.m[8] * det;
	m[9] = - mat.m[9] * det;
	m[10] = + mat.m[10] * det;
	m[11] = - mat.m[11] * det;
	m[12] = - mat.m[12] * det;
	m[13] = + mat.m[13] * det;
	m[14] = - mat.m[14] * det;
	m[15] = + mat.m[15] * det;

	return *this;
}

//-----------------------------------------------------------------------------
inline Matrix4x4 Matrix4x4::get_inverted() const
{
	Matrix4x4 tmp(*this);

	return tmp.invert();
}

//-----------------------------------------------------------------------------
void Matrix4x4::load_identity()
{
	m[0] = m[5] = m[10] = m[15] = 1.0;
	m[1] = m[2] = m[3] = m[4] = m[6] = m[7] = m[8] = m[9] = m[11] = m[12] = m[13] = m[14] = 0.0;
}

//-----------------------------------------------------------------------------
Vector3 Matrix4x4::x() const
{
	return Vector3(m[0], m[1], m[2]);
}

//-----------------------------------------------------------------------------
Vector3 Matrix4x4::y() const
{
	return Vector3(m[4], m[5], m[6]);
}

//-----------------------------------------------------------------------------
Vector3 Matrix4x4::z() const
{
	return Vector3(m[8], m[9], m[10]);
}

//-----------------------------------------------------------------------------
void Matrix4x4::set_x(const Vector3& x)
{
	m[0] = x.x;
	m[1] = x.y;
	m[2] = x.z;
}

//-----------------------------------------------------------------------------
void Matrix4x4::set_y(const Vector3& y)
{
	m[4] = y.x;
	m[5] = y.y;
	m[6] = y.z;
}

//-----------------------------------------------------------------------------
void Matrix4x4::set_z(const Vector3& z)
{
	m[8] = z.x;
	m[9] = z.y;
	m[10] = z.z;
}

//-----------------------------------------------------------------------------
Vector3 Matrix4x4::translation() const
{
	Vector3 tmp;

	tmp.x = m[12];
	tmp.y = m[13];
	tmp.z = m[14];

	return tmp;
}

//-----------------------------------------------------------------------------
void Matrix4x4::set_translation(const Vector3& trans)
{
	m[12] = trans.x;
	m[13] = trans.y;
	m[14] = trans.z;
}

//-----------------------------------------------------------------------------
Vector3 Matrix4x4::get_scale() const
{
	Vector3 tmp;

	tmp.x = m[0];
	tmp.y = m[5];
	tmp.z = m[10];

	return tmp;
}

//-----------------------------------------------------------------------------
void Matrix4x4::set_scale(const Vector3& scale)
{
	m[0] = scale.x;
	m[5] = scale.y;
	m[10] = scale.z;
}

//-----------------------------------------------------------------------------
float* Matrix4x4::to_float_ptr()
{
	return &m[0];
}

//-----------------------------------------------------------------------------
const float* Matrix4x4::to_float_ptr() const
{
	return &m[0];
}

//-----------------------------------------------------------------------------
Matrix3x3 Matrix4x4::to_mat3() const
{
	Matrix3x3 tmp;

	tmp.m[0] = m[0];
	tmp.m[1] = m[1];
	tmp.m[2] = m[2];
	tmp.m[3] = m[4];
	tmp.m[4] = m[5];
	tmp.m[5] = m[6];
	tmp.m[6] = m[8];
	tmp.m[7] = m[9];
	tmp.m[8] = m[10];

	return tmp;
}

//-----------------------------------------------------------------------------
Quaternion Matrix4x4::to_quat() const
{
	Quaternion tmp;
	float fourWSquaredMinusOne = m[0] + m[5] + m[10];
	float fourXSquaredMinusOne = m[0] - m[5] - m[10];
	float fourYSquaredMinusOne = -m[0] + m[5] - m[10];
	float fourZSquaredMinusOne = -m[0] - m[5] + m[10];
	float fourMaxSquaredMinusOne = fourWSquaredMinusOne;
	uint32_t index = 0;

	if (fourXSquaredMinusOne > fourMaxSquaredMinusOne)
	{
		fourMaxSquaredMinusOne = fourXSquaredMinusOne;
		index = 1;
	}

	if (fourYSquaredMinusOne > fourMaxSquaredMinusOne)
	{
		fourMaxSquaredMinusOne = fourYSquaredMinusOne;
		index = 2;
	}

	if (fourZSquaredMinusOne > fourMaxSquaredMinusOne)
	{
		fourMaxSquaredMinusOne = fourZSquaredMinusOne;
		index = 3;
	}

	float biggest = math::sqrt(fourMaxSquaredMinusOne + (float)1.0) * (float)0.5;
	float mult = (float)0.25 / biggest;

	switch (index)
	{
		case 0:
			tmp.w = biggest;
			tmp.v.x = (-m[9] + m[6]) * mult;
			tmp.v.y = (-m[2] + m[8]) * mult;
			tmp.v.z = (-m[4] + m[1]) * mult;
			break;
		case 1:
			tmp.v.x = biggest;
			tmp.w = (-m[9] + m[6]) * mult;
			tmp.v.y = (-m[4] - m[1]) * mult;
			tmp.v.z = (-m[2] - m[8]) * mult;
			break;
		case 2:
			tmp.v.y = biggest;
			tmp.w = (-m[2] + m[8]) * mult;
			tmp.v.x = (-m[4] - m[1]) * mult;
			tmp.v.z = (-m[9] - m[6]) * mult;
			break;
		case 3:
			tmp.v.z = biggest;
			tmp.w = (-m[4] + m[1]) * mult;
			tmp.v.x = (-m[2] - m[8]) * mult;
			tmp.v.y = (-m[9] - m[6]) * mult;
			break;
	}

	return tmp;
}

} // namespace crown

