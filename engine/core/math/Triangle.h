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

#include "Vector3.h"
#include "Plane.h"
#include "MathUtils.h"

namespace crown
{


///	3D Triangle.
///
/// Used mainly for collision detection.
struct Triangle
{
public:

	/// Does nothing for efficiency.
				Triangle();
				Triangle(const Vector3& v1, const Vector3& v2, const Vector3& v3);
				~Triangle();

	float		area() const;

	/// Returns the center of gravity (a.k.a. "centroid").
	Vector3		centroid() const;

	/// Returns the barycentric coordinates of point @a p in respect to the triangle.
	Vector3		barycentric_coords(const Vector3& p) const;

	/// Returns whether the triangle contains the @a p point.
	bool		contains_point(const Vector3& p) const;

	/// Returns the plane containing the triangle.
	Plane		to_plane() const;

private:

	// Vertices in CCW order
	Vector3		m_vertex[3];

	friend class Intersection;
};

//-----------------------------------------------------------------------------
inline Triangle::Triangle()
{
}

//-----------------------------------------------------------------------------
inline Triangle::Triangle(const Vector3& v1, const Vector3& v2, const Vector3& v3)
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
inline float Triangle::area() const
{
	Vector3 cr = vector3::cross(m_vertex[1] - m_vertex[0], m_vertex[2] - m_vertex[0]);
	return vector3::length(cr * 0.5);
}

//-----------------------------------------------------------------------------
inline Vector3 Triangle::centroid() const
{
	return (m_vertex[0] + m_vertex[1] + m_vertex[2]) * math::ONE_OVER_THREE;
}

//-----------------------------------------------------------------------------
inline Vector3 Triangle::barycentric_coords(const Vector3& p) const
{
	Vector3 e1 = m_vertex[2] - m_vertex[1];
	Vector3 e2 = m_vertex[0] - m_vertex[2];
	Vector3 e3 = m_vertex[1] - m_vertex[0];

	Vector3 d1 = p - m_vertex[0];
	Vector3 d2 = p - m_vertex[1];
	Vector3 d3 = p - m_vertex[2];

	Vector3 n = vector3::cross(e1, e2) / vector3::length(vector3::cross(e1, e2));

	// Signed areas
	float at = (float)(vector3::dot(vector3::cross(e1, e2), n) * 0.5);

	float at1 = (float)(vector3::dot(vector3::cross(e1, d3), n) * 0.5);
	float at2 = (float)(vector3::dot(vector3::cross(e2, d1), n) * 0.5);
	float at3 = (float)(vector3::dot(vector3::cross(e3, d2), n) * 0.5);

	float oneOverAt = (float)(1.0 / at);

	return Vector3(at1 * oneOverAt, at2 * oneOverAt, at3 * oneOverAt);
}

//-----------------------------------------------------------------------------
inline bool Triangle::contains_point(const Vector3& p) const
{
	Vector3 bc = barycentric_coords(p);

	if (bc.x < 0.0 || bc.y < 0.0 || bc.z < 0.0)
	{
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
inline Plane Triangle::to_plane() const
{
	Vector3 e1 = m_vertex[2] - m_vertex[1];
	Vector3 e2 = m_vertex[1] - m_vertex[0];

	Vector3 e2xe1 = vector3::cross(e2, e1);
	Vector3 n = vector3::normalize(e2xe1);
	float d = -vector3::dot(n, m_vertex[0]);

	return Plane(n, d);
}

} // namespace crown
