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

#include "Vec3.h"
#include "Plane.h"
#include "MathUtils.h"

namespace crown
{


///	3D Triangle.
///
/// Used mainly for collision detection.
class Triangle
{
public:

	/// Does nothing for efficiency.
				Triangle();
				Triangle(const Vec3& v1, const Vec3& v2, const Vec3& v3);
				~Triangle();

	real		area() const;

	/// Returns the center of gravity (a.k.a. "centroid").
	Vec3		centroid() const;

	/// Returns the barycentric coordinates of point @a p in respect to the triangle.
	Vec3		barycentric_coords(const Vec3& p) const;

	/// Returns whether the triangle contains the @a p point.
	bool		contains_point(const Vec3& p) const;

	/// Returns the plane containing the triangle.
	Plane		to_plane() const;

private:

	// Vertices in CCW order
	Vec3		m_vertex[3];

	friend class Intersection;
};

//-----------------------------------------------------------------------------
inline Triangle::Triangle()
{
}

//-----------------------------------------------------------------------------
inline Triangle::Triangle(const Vec3& v1, const Vec3& v2, const Vec3& v3)
{
	m_vertex[0] = v1;
	m_vertex[1] = v2;
	m_vertex[2] = v3;
}

//-----------------------------------------------------------------------------
inline Triangle::~Triangle()
{
}

//-----------------------------------------------------------------------------
inline real Triangle::area() const
{
	return ((m_vertex[1] - m_vertex[0]).cross(m_vertex[2] - m_vertex[0])).length() * (real)0.5;
}

//-----------------------------------------------------------------------------
inline Vec3 Triangle::centroid() const
{
	return (m_vertex[0] + m_vertex[1] + m_vertex[2]) * math::ONE_OVER_THREE;
}

//-----------------------------------------------------------------------------
inline Vec3 Triangle::barycentric_coords(const Vec3& p) const
{
	Vec3 e1 = m_vertex[2] - m_vertex[1];
	Vec3 e2 = m_vertex[0] - m_vertex[2];
	Vec3 e3 = m_vertex[1] - m_vertex[0];

	Vec3 d1 = p - m_vertex[0];
	Vec3 d2 = p - m_vertex[1];
	Vec3 d3 = p - m_vertex[2];

	Vec3 n = e1.cross(e2) / e1.cross(e2).length();

	// Signed areas
	real at = (real)(e1.cross(e2).dot(n) * 0.5);

	real at1 = (real)(e1.cross(d3).dot(n) * 0.5);
	real at2 = (real)(e2.cross(d1).dot(n) * 0.5);
	real at3 = (real)(e3.cross(d2).dot(n) * 0.5);

	real oneOverAt = (real)(1.0 / at);

	return Vec3(at1 * oneOverAt, at2 * oneOverAt, at3 * oneOverAt);
}

//-----------------------------------------------------------------------------
inline bool Triangle::contains_point(const Vec3& p) const
{
	Vec3 bc = barycentric_coords(p);

	if (bc.x < 0.0 || bc.y < 0.0 || bc.z < 0.0)
	{
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
inline Plane Triangle::to_plane() const
{
	Vec3 e1 = m_vertex[2] - m_vertex[1];
	Vec3 e2 = m_vertex[1] - m_vertex[0];

	Vec3 n = e2.cross(e1).normalize();
	real d = -n.dot(m_vertex[0]);

	return Plane(n, d);
}

} // namespace crown
