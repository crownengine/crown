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

#include "Types.h"
#include "Sphere.h"
#include "Vector3.h"
#include "MathTypes.h"

namespace crown
{

/// Functions to manipulate Plane.
///
/// @ingroup Math
namespace plane
{
	/// Normalizes the plane @æ p and returns its result.
	Plane& normalize(Plane& p);

	/// Returns the signed distance between plane @a p and point @a point.
	float distance_to_point(const Plane& p, const Vector3& point);

} // namespace plane

namespace plane
{
	const Plane ZERO = Plane(vector3::ZERO, 0.0);
	const Plane	XAXIS = Plane(vector3::XAXIS, 0.0);
	const Plane	YAXIS = Plane(vector3::YAXIS, 0.0);
	const Plane	ZAXIS = Plane(vector3::ZAXIS, 0.0);

	//-----------------------------------------------------------------------------
	inline Plane& normalize(Plane& p)
	{
		float len = vector3::length(p.n);

		if (math::equals(len, (float) 0.0))
		{
			return p;
		}

		const float inv_len = (float) 1.0 / len;

		p.n *= inv_len;
		p.d *= inv_len;

		return p;
	}

	//-----------------------------------------------------------------------------
	inline float distance_to_point(const Plane& p, const Vector3& point)
	{
		return vector3::dot(p.n, point) + p.d;
	}
} // namespace plane

//-----------------------------------------------------------------------------
inline Plane::Plane()
{
	// Do not initialize
}

//-----------------------------------------------------------------------------
inline Plane::Plane(const Vector3& normal, float dist)
	: n(normal), d(dist)
{
}

} // namespace crown
