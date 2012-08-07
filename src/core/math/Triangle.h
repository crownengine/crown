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

#pragma once

#include "Vec3.h"
#include "Plane.h"
#include "MathUtils.h"

namespace Crown
{

/**
	3D Triangle.

	Used mainly for collision detection.
*/
class Triangle
{

public:

	Vec3		v1, v2, v3;				//!< Vertices, CCW order

				Triangle();				//!< Constructor
				Triangle(const Vec3& nv1, const Vec3& nv2, const Vec3& nv3);	//!< Constructor
				~Triangle();			//!< Destructor

	real		GetArea() const;		//!< Returns the area
	Vec3		GetCentroid() const;	//!< Returns the center of gravity (a.k.a. centroid.)
	Vec3		GetBarycentricCoords(const Vec3& p) const;	//!< Returns the barycentric coordinates of point "p"

	bool		ContainsPoint(const Vec3& p) const;		//!< Returns whether the triangle contains the "p" point

	Plane		ToPlane() const;		//!< Returns the plane containing the triangle
};

//-----------------------------------------------------------------------------
inline Triangle::Triangle() : v1(Vec3::ZERO), v2(Vec3::ZERO), v3(Vec3::ZERO)
{
}

//-----------------------------------------------------------------------------
inline Triangle::Triangle(const Vec3& nv1, const Vec3& nv2, const Vec3& nv3) : v1(nv1), v2(nv2), v3(nv3)
{
}

//-----------------------------------------------------------------------------
inline Triangle::~Triangle()
{
}

//-----------------------------------------------------------------------------
inline real Triangle::GetArea() const
{
	return ((v2 - v1).Cross(v3 - v1)).GetLength() * (real)0.5;
}

//-----------------------------------------------------------------------------
inline Vec3 Triangle::GetCentroid() const
{
	return (v1 + v2 + v3) * Math::ONE_OVER_THREE;
}

//-----------------------------------------------------------------------------
inline Vec3 Triangle::GetBarycentricCoords(const Vec3& p) const
{
//	Vec3 e1 = v1 - v3;
//	Vec3 e2 = v2 - v1;
//	Vec3 e3 = v3 - v2;

	Vec3 e1 = v3 - v2;
	Vec3 e2 = v1 - v3;
	Vec3 e3 = v2 - v1;

	Vec3 d1 = p - v1;
	Vec3 d2 = p - v2;
	Vec3 d3 = p - v3;

	Vec3 n = e1.Cross(e2) / e1.Cross(e2).GetLength();

	// Signed areas
	real at = (real)(e1.Cross(e2).Dot(n) * 0.5);

	real at1 = (real)(e1.Cross(d3).Dot(n) * 0.5);
	real at2 = (real)(e2.Cross(d1).Dot(n) * 0.5);
	real at3 = (real)(e3.Cross(d2).Dot(n) * 0.5);

	real oneOverAt = (real)(1.0 / at);

	return Vec3(at1 * oneOverAt, at2 * oneOverAt, at3 * oneOverAt);
}

//-----------------------------------------------------------------------------
inline bool Triangle::ContainsPoint(const Vec3& p) const
{
	Vec3 bc = GetBarycentricCoords(p);

	if (bc.x < 0.0 || bc.y < 0.0 || bc.z < 0.0)
	{
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
inline Plane Triangle::ToPlane() const
{
	Vec3 e1 = v3 - v2;
	Vec3 e2 = v2 - v1;

	Vec3 n = e2.Cross(e1).Normalize();
	real d = -n.Dot(v1);

	return Plane(n, d);
}

} // namespace Crown

