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

#include "Mat3.h"
#include "Mat4.h"
#include "Types.h"
#include "MathUtils.h"
#include "Quat.h"
#include "Vec3.h"

namespace crown
{

//-----------------------------------------------------------------------------
Quat::Quat()
{
}

//-----------------------------------------------------------------------------
Quat::Quat(const Vec3& axis, float angle)
{
	v = axis * math::sin(angle * 0.5);
	w = math::cos(angle * 0.5);
}

//-----------------------------------------------------------------------------
void Quat::negate()
{
	w = -w;
	v.negate();
}

//-----------------------------------------------------------------------------
void Quat::load_identity()
{
	w = 1.0;
	v.x = 0.0;
	v.y = 0.0;
	v.z = 0.0;
}

//-----------------------------------------------------------------------------
float Quat::length() const
{
	return math::sqrt(w * w + v.x * v.x + v.y * v.y + v.z * v.z);
}

//-----------------------------------------------------------------------------
void Quat::conjugate()
{
	v = -v;
}

//-----------------------------------------------------------------------------
Quat Quat::get_conjugate() const
{
	return Quat(-v, w);
}

//-----------------------------------------------------------------------------
Quat Quat::get_inverse() const
{
	return get_conjugate() * (1.0 / length());
}

//-----------------------------------------------------------------------------
Mat3 Quat::to_mat3() const
{
	const float& x = v.x;
	const float& y = v.y;
	const float& z = v.z;

	Mat3 tmp;

	tmp.m[0] = (float)(1.0 - 2.0*y*y - 2.0*z*z);
	tmp.m[1] = (float)(2.0*x*y + 2.0*w*z);
	tmp.m[2] = (float)(2.0*x*z - 2.0*w*y);
	tmp.m[3] = (float)(2.0*x*y - 2.0*w*z);
	tmp.m[4] = (float)(1.0 - 2.0*x*x - 2.0*z*z);
	tmp.m[5] = (float)(2.0*y*z + 2.0*w*x);
	tmp.m[6] = (float)(2.0*x*z + 2.0*w*y);
	tmp.m[7] = (float)(2.0*y*z - 2.0*w*x);
	tmp.m[8] = (float)(1.0 - 2.0*x*x - 2.0*y*y);

	return tmp;
}

//-----------------------------------------------------------------------------
Mat4 Quat::to_mat4() const
{
	const float& x = v.x;
	const float& y = v.y;
	const float& z = v.z;

	Mat4 tmp;

	tmp.m[0] = (float)(1.0 - 2.0*y*y - 2.0*z*z);
	tmp.m[1] = (float)(2.0*x*y + 2.0*w*z);
	tmp.m[2] = (float)(2.0*x*z - 2.0*w*y);
	tmp.m[3] = 0;
	tmp.m[4] = (float)(2.0*x*y - 2.0*w*z);
	tmp.m[5] = (float)(1.0 - 2.0*x*x - 2.0*z*z);
	tmp.m[6] = (float)(2.0*y*z + 2.0*w*x);
	tmp.m[7] = 0.0;
	tmp.m[8] = (float)(2.0*x*z + 2.0*w*y);
	tmp.m[9] = (float)(2.0*y*z - 2.0*w*x);
	tmp.m[10] = (float)(1.0 - 2.0*x*x - 2.0*y*y);
	tmp.m[11] = 0.0;
	tmp.m[12] = 0.0;
	tmp.m[13] = 0.0;
	tmp.m[14] = 0.0;
	tmp.m[15] = 1.0;

	return tmp;
}

//-----------------------------------------------------------------------------
Quat Quat::operator*(const Quat& b) const
{
	Quat tmp;

	tmp.w = w * b.w - v.dot(b.v);
	tmp.v = w * b.v + b.w * v + b.v.cross(v);

	return tmp;
}

//-----------------------------------------------------------------------------
Quat Quat::operator*(const float& k) const
{
	Quat tmp;

	tmp.w = w * k;
	tmp.v = v * k;

	return tmp;
}

//-----------------------------------------------------------------------------
Quat Quat::power(float exp)
{
	Quat tmp;

	if (math::abs(w) < 0.9999)
	{
		float alpha = math::acos(w); // alpha = theta/2
		float newAlpha = alpha * exp;
		tmp.w = math::cos(newAlpha);
		float mult = math::sin(newAlpha) / math::sin(alpha);
		tmp.v.x = v.x * mult;
		tmp.v.y = v.y * mult;
		tmp.v.z = v.z * mult;
		return tmp;
	}

	tmp.w = w;
	tmp.v = v;

	return tmp;
}

/*
The geometric interpretation of the Quat dot product is similar to the interpretation of
the vector dot product; the larger the absolute value of the Quat dot product axb, the more
"similar" the angular displacements represented by a and b.
*/
//-----------------------------------------------------------------------------
float dot(const Quat& a, const Quat& b)
{
	return a.w * b.w + a.v.dot(b.v);
}

// Spherical Linear interpolation
//-----------------------------------------------------------------------------
Quat slerp(const Quat& start, const Quat& end, float t)
{
	Quat delta = end * start.get_inverse();
	delta = delta.power(t);

	return delta * start;
}

} // namespace crown

