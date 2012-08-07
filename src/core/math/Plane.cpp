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

#include "MathUtils.h"
#include "Plane.h"
#include "Types.h"

namespace Crown
{

const Plane Plane::ZERO = Plane(Vec3::ZERO, 0.0);
const Plane	Plane::XAXIS = Plane(Vec3::XAXIS, 0.0);
const Plane	Plane::YAXIS = Plane(Vec3::YAXIS, 0.0);
const Plane	Plane::ZAXIS = Plane(Vec3::ZAXIS, 0.0);

//-----------------------------------------------------------------------------
Plane::Plane()
{
}

//-----------------------------------------------------------------------------
Plane::Plane(const Plane& p) : n(p.n), d(p.d)
{
}

//-----------------------------------------------------------------------------
Plane::Plane(const Vec3& normal, real dist) : n(normal), d(dist)
{
}

//-----------------------------------------------------------------------------
Plane::~Plane()
{
}

//-----------------------------------------------------------------------------
Plane& Plane::Normalize()
{
	real len = n.GetLength();

	if (Math::Equals(len, (real)0.0))
	{
		return *this;
	}

	len = (real)1.0 / len;

	n *= len;
	d *= len;

	return *this;
}

//-----------------------------------------------------------------------------
real Plane::GetDistanceToPoint(const Vec3& p) const
{
	return n.Dot(p) + d;
}

//-----------------------------------------------------------------------------
bool Plane::ContainsPoint(const Vec3& p) const
{
	return Math::Equals(n.Dot(p) + d, (real)0.0);
}

} // namespace Crown

