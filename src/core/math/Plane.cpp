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

#include "MathUtils.h"
#include "Plane.h"
#include "Types.h"

namespace crown
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
Plane::Plane(const Vec3& normal, float dist) : n(normal), d(dist)
{
}

//-----------------------------------------------------------------------------
Plane& Plane::normalize()
{
	float len = n.length();

	if (math::equals(len, (float)0.0))
	{
		return *this;
	}

	len = (float)1.0 / len;

	n *= len;
	d *= len;

	return *this;
}

//-----------------------------------------------------------------------------
float Plane::distance_to_point(const Vec3& p) const
{
	return n.dot(p) + d;
}

//-----------------------------------------------------------------------------
bool Plane::contains_point(const Vec3& p) const
{
	return math::equals(n.dot(p) + d, (float)0.0);
}

} // namespace crown

