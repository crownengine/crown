/*
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

#include <cassert>
#include "Angles.h"
#include "Mat3.h"
#include "Types.h"
#include "Mat4.h"
#include "MathUtils.h"
#include "Quat.h"
#include "Str.h"
#include "Vec3.h"

namespace crown
{

const Mat3 Mat3::IDENTITY = Mat3(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);

//-----------------------------------------------------------------------------
Mat3::Mat3()
{
}

//-----------------------------------------------------------------------------
Mat3::Mat3(real r1c1, real r2c1, real r3c1, real r1c2, real r2c2, real r3c2,
	real r1c3, real r2c3, real r3c3)
{
	m[0] = r1c1;
	m[1] = r2c1;
	m[2] = r3c1;
	m[3] = r1c2;
	m[4] = r2c2;
	m[5] = r3c2;
	m[6] = r1c3;
	m[7] = r2c3;
	m[8] = r3c3;
}

//-----------------------------------------------------------------------------
Mat3::Mat3(const real v[9])
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
}

//-----------------------------------------------------------------------------
Mat3::Mat3(const Mat3& a)
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
}

//-----------------------------------------------------------------------------
Mat3::~Mat3()
{
}

//-----------------------------------------------------------------------------
Mat3& Mat3::operator=(const Mat3& a)
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

	return *this;
}

//-----------------------------------------------------------------------------
real Mat3::operator[](uint32_t i) const
{
	assert(i < 9);

	return m[i];
}

//-----------------------------------------------------------------------------
real& Mat3::operator[](uint32_t i)
{
	assert(i < 9);

	return m[i];
}

//-----------------------------------------------------------------------------
real Mat3::operator()(uint32_t row, uint32_t column) const
{
	assert(row < 3 && column < 3);

	return m[row + column * 3];
}

//-----------------------------------------------------------------------------
Mat3 Mat3::operator+(const Mat3& a) const
{
	Mat3 tmp;

	tmp.m[0] = m[0] + a.m[0];
	tmp.m[1] = m[1] + a.m[1];
	tmp.m[2] = m[2] + a.m[2];
	tmp.m[3] = m[3] + a.m[3];
	tmp.m[4] = m[4] + a.m[4];
	tmp.m[5] = m[5] + a.m[5];
	tmp.m[6] = m[6] + a.m[6];
	tmp.m[7] = m[7] + a.m[7];
	tmp.m[8] = m[8] + a.m[8];

	return tmp;
}

//-----------------------------------------------------------------------------
Mat3& Mat3::operator+=(const Mat3& a)
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

	return *this;
}

//-----------------------------------------------------------------------------
Mat3 Mat3::operator-(const Mat3& a) const
{
	Mat3 tmp;

	tmp.m[0] = m[0] - a.m[0];
	tmp.m[1] = m[1] - a.m[1];
	tmp.m[2] = m[2] - a.m[2];
	tmp.m[3] = m[3] - a.m[3];
	tmp.m[4] = m[4] - a.m[4];
	tmp.m[5] = m[5] - a.m[5];
	tmp.m[6] = m[6] - a.m[6];
	tmp.m[7] = m[7] - a.m[7];
	tmp.m[8] = m[8] - a.m[8];

	return tmp;
}

//-----------------------------------------------------------------------------
Mat3& Mat3::operator-=(const Mat3& a)
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

	return *this;
}

//-----------------------------------------------------------------------------
Mat3 Mat3::operator*(real k) const
{
	Mat3 tmp;

	tmp.m[0] = m[0] * k;
	tmp.m[1] = m[1] * k;
	tmp.m[2] = m[2] * k;
	tmp.m[3] = m[3] * k;
	tmp.m[4] = m[4] * k;
	tmp.m[5] = m[5] * k;
	tmp.m[6] = m[6] * k;
	tmp.m[7] = m[7] * k;
	tmp.m[8] = m[8] * k;

	return tmp;
}

//-----------------------------------------------------------------------------
Mat3& Mat3::operator*=(real k)
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

	return *this;
}

//-----------------------------------------------------------------------------
Mat3 Mat3::operator/(real k) const
{
	Mat3 tmp;

	k = (real)1.0 / k;

	tmp.m[0] = m[0] * k;
	tmp.m[1] = m[1] * k;
	tmp.m[2] = m[2] * k;
	tmp.m[3] = m[3] * k;
	tmp.m[4] = m[4] * k;
	tmp.m[5] = m[5] * k;
	tmp.m[6] = m[6] * k;
	tmp.m[7] = m[7] * k;
	tmp.m[8] = m[8] * k;

	return tmp;
}

//-----------------------------------------------------------------------------
Mat3& Mat3::operator/=(real k)
{
	k = (real)1.0 / k;

	m[0] *= k;
	m[1] *= k;
	m[2] *= k;
	m[3] *= k;
	m[4] *= k;
	m[5] *= k;
	m[6] *= k;
	m[7] *= k;
	m[8] *= k;

	return *this;
}

//-----------------------------------------------------------------------------
Vec3 Mat3::operator*(const Vec3& v) const
{
	Vec3 tmp;

	tmp.x = m[0] * v.x + m[3] * v.y + m[6] * v.z;
	tmp.y = m[1] * v.x + m[4] * v.y + m[7] * v.z;
	tmp.z = m[2] * v.x + m[5] * v.y + m[8] * v.z;

	return tmp;
}

//-----------------------------------------------------------------------------
Mat3 Mat3::operator*(const Mat3& a) const
{
	Mat3 tmp;

	tmp.m[0] = m[0] * a.m[0] + m[3] * a.m[1] + m[6] * a.m[2];
	tmp.m[1] = m[1] * a.m[0] + m[4] * a.m[1] + m[7] * a.m[2];
	tmp.m[2] = m[2] * a.m[0] + m[5] * a.m[1] + m[8] * a.m[2];

	tmp.m[3] = m[0] * a.m[3] + m[3] * a.m[4] + m[6] * a.m[5];
	tmp.m[4] = m[1] * a.m[3] + m[4] * a.m[4] + m[7] * a.m[5];
	tmp.m[5] = m[2] * a.m[3] + m[5] * a.m[4] + m[8] * a.m[5];

	tmp.m[6] = m[0] * a.m[6] + m[3] * a.m[7] + m[6] * a.m[8];
	tmp.m[7] = m[1] * a.m[6] + m[4] * a.m[7] + m[7] * a.m[8];
	tmp.m[8] = m[2] * a.m[6] + m[5] * a.m[7] + m[8] * a.m[8];

	return tmp;
}

//-----------------------------------------------------------------------------
Mat3& Mat3::operator*=(const Mat3& a)
{
	Mat3 tmp;

	tmp.m[0] = m[0] * a.m[0] + m[3] * a.m[1] + m[6] * a.m[2];
	tmp.m[1] = m[1] * a.m[0] + m[4] * a.m[1] + m[7] * a.m[2];
	tmp.m[2] = m[2] * a.m[0] + m[5] * a.m[1] + m[8] * a.m[2];

	tmp.m[3] = m[0] * a.m[3] + m[3] * a.m[4] + m[6] * a.m[5];
	tmp.m[4] = m[1] * a.m[3] + m[4] * a.m[4] + m[7] * a.m[5];
	tmp.m[5] = m[2] * a.m[3] + m[5] * a.m[4] + m[8] * a.m[5];

	tmp.m[6] = m[0] * a.m[6] + m[3] * a.m[7] + m[6] * a.m[8];
	tmp.m[7] = m[1] * a.m[6] + m[4] * a.m[7] + m[7] * a.m[8];
	tmp.m[8] = m[2] * a.m[6] + m[5] * a.m[7] + m[8] * a.m[8];

	*this = tmp;

	return *this;
}

//-----------------------------------------------------------------------------
Mat3 operator*(real k, const Mat3& a)
{
	return a * k;
}

//-----------------------------------------------------------------------------
void Mat3::build_rotation_x(real radians)
{
	m[0] = 1.0;
	m[1] = 0.0;
	m[2] = 0.0;
	m[3] = 0.0;
	m[4] = math::cos(radians);
	m[5] = math::sin(radians);
	m[6] = 0.0;
	m[7] = -math::sin(radians);
	m[8] = math::cos(radians);
}

//-----------------------------------------------------------------------------
void Mat3::build_rotation_y(real radians)
{
	m[0] = math::cos(radians);
	m[1] = 0.0;
	m[2] = -math::sin(radians);
	m[3] = 0.0;
	m[4] = 1.0;
	m[5] = 0.0;
	m[6] = math::sin(radians);
	m[7] = 0.0;
	m[8] = math::cos(radians);
}

//-----------------------------------------------------------------------------
void Mat3::build_rotation_z(real radians)
{
	m[0] = math::cos(radians);
	m[1] = math::sin(radians);
	m[2] = 0.0;
	m[3] = -math::sin(radians);
	m[4] = math::cos(radians);
	m[5] = 0.0;
	m[6] = 0.0;
	m[7] = 0.0;
	m[8] = 1.0;
}

//-----------------------------------------------------------------------------
void Mat3::build_rotation(const Vec3& n, real radians)
{
	real a = (real)1.0 - math::cos(radians);
	real sin_a = math::sin(radians);
	real cos_a = math::cos(radians);

	m[0] = n.x * n.x * a + cos_a;
	m[1] = n.x * n.y * a + n.z * sin_a;
	m[2] = n.x * n.z * a - n.y * sin_a;
	m[3] = n.x * n.y * a - n.z * sin_a;
	m[4] = n.y * n.y * a + cos_a;
	m[5] = n.y * n.z * a + n.x * sin_a;
	m[6] = n.x * n.z * a + n.y * sin_a;
	m[7] = n.y * n.z * a - n.x * sin_a;
	m[8] = n.z * n.z * a + cos_a;
}

//-----------------------------------------------------------------------------
Mat3& Mat3::transpose()
{
	real tmp;

	tmp = m[1];
	m[1] = m[3];
	m[3] = tmp;

	tmp = m[2];
	m[2] = m[6];
	m[6] = tmp;

	tmp = m[5];
	m[5] = m[7];
	m[7] = tmp;

	return *this;
}

//-----------------------------------------------------------------------------
Mat3 Mat3::get_transposed() const
{
	Mat3 tmp;

	tmp.m[0] = m[0];
	tmp.m[1] = m[3];
	tmp.m[2] = m[6];
	tmp.m[3] = m[1];
	tmp.m[4] = m[4];
	tmp.m[5] = m[7];
	tmp.m[6] = m[2];
	tmp.m[7] = m[5];
	tmp.m[8] = m[8];

	return tmp;
}

//-----------------------------------------------------------------------------
real Mat3::get_determinant() const
{
	real det;

	det =	m[0] * (m[4] * m[8] - m[7] * m[5]) -
			m[3] * (m[1] * m[8] - m[7] * m[2]) +
			m[6] * (m[1] * m[5] - m[4] * m[2]);

	return det;
}

//-----------------------------------------------------------------------------
Mat3& Mat3::invert()
{
	Mat3 mat;
	real det;

	mat.m[0] = (m[4] * m[8] - m[7] * m[5]);
	mat.m[1] = (m[1] * m[8] - m[7] * m[2]);
	mat.m[2] = (m[1] * m[5] - m[4] * m[2]);

	det = m[0] * mat.m[0] - m[3] * mat.m[1] + m[6] * mat.m[2];
	det = (real)1.0 / det;

	mat.m[3] = (m[3] * m[8] - m[6] * m[5]);
	mat.m[4] = (m[0] * m[8] - m[6] * m[2]);
	mat.m[5] = (m[0] * m[5] - m[3] * m[2]);
	mat.m[6] = (m[3] * m[7] - m[6] * m[4]);
	mat.m[7] = (m[0] * m[7] - m[6] * m[1]);
	mat.m[8] = (m[0] * m[4] - m[3] * m[1]);

	m[0] = + mat.m[0] * det;
	m[1] = - mat.m[1] * det;
	m[2] = + mat.m[2] * det;
	m[3] = - mat.m[3] * det;
	m[4] = + mat.m[4] * det;
	m[5] = - mat.m[5] * det;
	m[6] = + mat.m[6] * det;
	m[7] = - mat.m[7] * det;
	m[8] = + mat.m[8] * det;

	return *this;
}

//-----------------------------------------------------------------------------
inline Mat3 Mat3::get_inverted() const
{
	Mat3 tmp(*this);

	return tmp.invert();
}

//-----------------------------------------------------------------------------
void Mat3::load_identity()
{
	m[0] = m[4] = m[8] = 1.0;
	m[1] = m[2] = m[3] = m[5] = m[6] = m[7] = 0.0;
}

//-----------------------------------------------------------------------------
Vec3 Mat3::get_scale() const
{
	Vec3 tmp;

	tmp.x = m[0];
	tmp.y = m[4];
	tmp.z = m[8];

	return tmp;
}

//-----------------------------------------------------------------------------
void Mat3::set_scale(const Vec3& scale)
{
	m[0] = scale.x;
	m[4] = scale.y;
	m[8] = scale.z;
}

//-----------------------------------------------------------------------------
real* Mat3::to_float_ptr()
{
	return &m[0];
}

//-----------------------------------------------------------------------------
const real* Mat3::to_float_ptr() const
{
	return &m[0];
}

//-----------------------------------------------------------------------------
Angles Mat3::to_angles() const
{
	Angles tmp;
	real sp = -m[7];

	if (sp <= -1.0)
	{
		tmp.pitch = -math::HALF_PI;
	}
	else if (sp >= 1.0)
	{
		tmp.pitch = math::HALF_PI;
	}
	else
	{
		tmp.pitch = math::asin(-m[8]);
	}

	if (sp > 0.9999)
	{
		tmp.bank = 0.0;
		tmp.heading = math::atan2(-m[2], m[0]);
	}
	else
	{
		tmp.heading = math::atan2(m[6], m[8]);
		tmp.bank = math::atan2(m[1], m[4]);
	}

	return tmp;
}

//-----------------------------------------------------------------------------
Mat4 Mat3::to_mat4() const
{
	Mat4 tmp;

	tmp.m[0] = m[0];
	tmp.m[1] = m[1];
	tmp.m[2] = m[2];
	tmp.m[3] = 0.0;
	tmp.m[4] = m[3];
	tmp.m[5] = m[4];
	tmp.m[6] = m[5];
	tmp.m[7] = 0.0;
	tmp.m[8] = m[6];
	tmp.m[9] = m[7];
	tmp.m[10] = m[8];
	tmp.m[11] = 0.0;
	tmp.m[12] = 0.0;
	tmp.m[13] = 0.0;
	tmp.m[14] = 0.0;
	tmp.m[15] = 1.0;

	return tmp;
}

//-----------------------------------------------------------------------------
Quat Mat3::to_quat() const
{
	Quat tmp;

	real fourWSquaredMinusOne = m[0] + m[4] + m[8];
	real fourXSquaredMinusOne = m[0] - m[4] - m[8];
	real fourYSquaredMinusOne = -m[0] + m[4] - m[8];
	real fourZSquaredMinusOne = -m[0] - m[4] + m[8];
	real fourMaxSquaredMinusOne = fourWSquaredMinusOne;
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

	real biggest = math::sqrt(fourMaxSquaredMinusOne + (real)1.0) * (real)0.5;
	real mult = (real)0.25 / biggest;

	switch (index)
	{
		case 0:
			tmp.w = biggest;
			tmp.v.x = (-m[7] + m[5]) * mult;
			tmp.v.y = (-m[2] + m[6]) * mult;
			tmp.v.z = (-m[3] + m[1]) * mult;
			break;
		case 1:
			tmp.v.x = biggest;
			tmp.w = (-m[7] + m[5]) * mult;
			tmp.v.y = (-m[3] - m[1]) * mult;
			tmp.v.z = (-m[2] - m[6]) * mult;
			break;
		case 2:
			tmp.v.y = biggest;
			tmp.w = (-m[2] + m[6]) * mult;
			tmp.v.x = (-m[3] - m[1]) * mult;
			tmp.v.z = (-m[7] - m[5]) * mult;
			break;
		case 3:
			tmp.v.z = biggest;
			tmp.w = (-m[3] + m[1]) * mult;
			tmp.v.x = (-m[2] - m[6]) * mult;
			tmp.v.y = (-m[7] - m[5]) * mult;
			break;
	}

	return tmp;
}

//-----------------------------------------------------------------------------
Str Mat3::to_str() const
{
	Str tmp;

	tmp = Str("[ ") + m[0] + Str(" ") + m[3] + Str(" ") + m[6] + Str(" ]\n");
	tmp += Str("| ") + m[1] + Str(" ") + m[4] + Str(" ") + m[7] + Str(" |\n");
	tmp += Str("[ ") + m[2] + Str(" ") + m[5] + Str(" ") + m[8] + Str(" ]\n");

	return tmp;
}

} // namespace crown

