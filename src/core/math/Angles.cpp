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

#include "Angles.h"
#include "Mat3.h"
#include "Mat4.h"
#include "Types.h"
#include "MathUtils.h"
#include "Quat.h"
#include "Str.h"
#include "Vec3.h"

namespace Crown
{

const Angles Angles::ZERO = Angles(0.0, 0.0, 0.0);

//-----------------------------------------------------------------------------
Angles::Angles()
{
}

//-----------------------------------------------------------------------------
Angles::~Angles()
{
}

//-----------------------------------------------------------------------------
Angles::Angles(real h, real p, real b) : heading(h), pitch(p), bank(b)
{
}

//-----------------------------------------------------------------------------
Str Angles::to_str() const
{
	Str tmp;

	tmp = Str("[ h: ") + heading + Str(" p: ") + pitch + Str(" b: ") + bank + Str(" ]\n");

	return tmp;
}

//-----------------------------------------------------------------------------
Mat3 Angles::to_mat3() const
{
	Mat3 tmp;

	real sh = math::sin(heading);
	real ch = math::cos(heading);
	real sp = math::sin(pitch);
	real cp = math::cos(pitch);
	real sb = math::sin(bank);
	real cb = math::cos(bank);

	tmp.m[0] = ch * cb + sh * sp * sb;
	tmp.m[1] = sb * cp;
	tmp.m[2] = -sh * cb + ch * sp * sb;
	tmp.m[3] = -ch * sb + sh * sp * cb;
	tmp.m[4] = cb * cp;
	tmp.m[5] = sb * sh + ch * sp * cb;
	tmp.m[6] = sh * cp;
	tmp.m[7] = -sp;
	tmp.m[8] = ch * cp;

	return tmp;
}

//-----------------------------------------------------------------------------
Mat4 Angles::to_mat4() const
{
	Mat4 tmp;

	real sh = math::sin(heading);
	real ch = math::cos(heading);
	real sp = math::sin(pitch);
	real cp = math::cos(pitch);
	real sb = math::sin(bank);
	real cb = math::cos(bank);

	tmp.m[0] = ch * cb + sh * sp * sb;
	tmp.m[1] = sb * cp;
	tmp.m[2] = -sh * cb + ch * sp * sb;
	tmp.m[3] = 0.0;
	tmp.m[4] = -ch * sb + sh * sp * cb;
	tmp.m[5] = cb * cp;
	tmp.m[6] = sb * sh + ch * sp * cb;
	tmp.m[7] = 0.0;
	tmp.m[8] = sh * cp;
	tmp.m[9] = -sp;
	tmp.m[10] = ch * cp;
	tmp.m[11] = 0.0;
	tmp.m[12] = 0.0;
	tmp.m[13] = 0.0;
	tmp.m[14] = 0.0;
	tmp.m[15] = 1.0;

	return tmp;
}

//-----------------------------------------------------------------------------
Quat Angles::to_quat() const
{
	Quat h(heading, Vec3(0.0, 1.0, 0.0));
	Quat p(pitch, Vec3(1.0, 0.0, 0.0));
	Quat b(bank, Vec3(0.0, 0.0, 1.0));

	return b * p * h;
}

} // namespace Crown

