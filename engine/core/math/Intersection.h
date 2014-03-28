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

#include "Ray.h"
#include "Plane.h"
#include "Sphere.h"
#include "Frustum.h"
#include "MathUtils.h"
#include "Types.h"
#include "Triangle.h"
#include "Circle.h"
#include "Rect.h"

namespace crown
{

/// Intersection test utils.
/// Table of Intersection tests (3d)
/// +----------+----------+----------+----------+----------+----------+----------+
/// |          | Ray      | Plane    | Sphere   | Box      | Frustum  | Triangle |
/// +----------+----------+----------+----------+----------+----------+----------+
/// | Ray      | No       | Yes      | Yes      | No       | No       | Yes      |
/// +----------+----------+----------+----------+----------+----------+----------+
/// | Plane    | -        | Yes (1)  | Yes (+)  | No       | No       | No       |
/// +----------+----------+----------+----------+----------+----------+----------+
/// | Sphere   | -        | -        | Yes (+)  | No       | Yes      | Yes (+)  |
/// +----------+----------+----------+----------+----------+----------+----------+
/// | Box      | -        | -        | -        | Yes (+)  | Yes      | No       |
/// +----------+----------+----------+----------+----------+----------+----------+
/// | Frustum  | -        | -        | -        | -        | No       | No       |
/// +----------+----------+----------+----------+----------+----------+----------+
/// | Triangle | -        | -        | -        | -        | -        | No       |
/// +----------+----------+----------+----------+----------+----------+----------+
/// Notes:
/// (1): Intersection of three planes
/// (-): Static intersection only
/// (+): Static/Dynamic intersection
/// Table of Intersection tests (2d)
/// +---------------+----------+-------------+-------------+----------+----------+
/// |               | Circle   | Rect        | O Rect      | Segment  | Ray 2d   |
/// +---------------+----------+-------------+-------------+----------+----------+
/// | Circle        | Yes (p-) | No          | No          | No       | No       |
/// +---------------+----------+-------------+-------------+----------+----------+
/// | Rect          | -        | Yes         | No          | No       | No       | <- Axis Aligned Rect
/// +---------------+----------+-------------+-------------+----------+----------+
/// | O Rect        | -        | -           | No          | No       | No       | <- Oriented Rect
/// +---------------+----------+-------------+-------------+----------+----------+
/// | Segment       | -        | -           | -           | No       | No       |
/// +---------------+----------+-------------+-------------+----------+----------+
/// | Ray 2d        | -        | -           | -           | -        | No       |
/// +---------------+----------+-------------+-------------+----------+----------+
/// Notes:
/// (p): Penetration vector
/// (-): Static intersection only
/// (+): Static/Dynamic intersection
class Intersection
{
public:

	static bool test_ray_plane(const Ray& r, const Plane& p, float& distance, Vector3& inttersectionPoint_t);
	static bool test_ray_sphere(const Ray& r, const Sphere& s, float& distance, Vector3& intersectionPoint);
	static bool test_ray_box(const Ray& r, const Box& b, float& distance, Vector3& intersectionPoint);
	static bool test_ray_triangle(const Ray& r, const Triangle& t, float& distance, Vector3& intersectionPoint);

	static bool test_plane_3(const Plane& p1, const Plane& p2, const Plane& p3, Vector3& ip);

	static bool test_static_sphere_plane(const Sphere& s, const Plane& p);
	static bool test_static_sphere_sphere(const Sphere& a, const Sphere& b);
	static bool test_dynamic_sphere_plane(const Sphere& s, const Vector3& d, const Plane& p, float& it, Vector3& intersectionPoint);
	static bool test_dynamic_sphere_triangle(const Sphere& s, const Vector3& d, const Triangle& tri, float& it, Vector3& intersectionPoint);
	static bool test_dynamic_sphere_sphere(const Sphere& s1, const Vector3& d1, const Sphere& s2, const Vector3& d2, float& it, Vector3& intersectionPoint);

	static bool test_static_box_box(const Box& b1, const Box& b2);
	static bool test_dynamic_box_box(const Box& b1, const Vector3& v1, const Box& b2, const Vector3& v2, float& it);

	static bool test_frustum_sphere(const Frustum& f, const Sphere& s);
	static bool test_frustum_box(const Frustum& f, const Box& box);

	static bool test_circle_circle(const Circle& c1, const Circle& c2, Vector2& penetration);
	static bool test_dynamic_circle_circle(const Circle& c1, const Vector2& d1, const Circle& c2, const Vector2& d2, float& it);
	static bool test_rect_rect(const Rect& r1, const Rect& r2, Vector2& penetration);
	static bool test_circle_rect(const Circle& c1, const Rect& r2, Vector2& penetration);

private:

	// Disable construction
	Intersection();
};

//-----------------------------------------------------------------------------
inline bool Intersection::test_ray_plane(const Ray& r, const Plane& p, float& distance, Vector3& intersectionPoint)
{
	float nd = r.direction().dot(p.n);
	float orpn = r.origin().dot(p.n);

	if (nd < 0.0)
	{
		float dist = (-p.d - orpn) / nd;
		if (dist > 0.0)
		{
			distance = dist;
			intersectionPoint = r.origin() + r.direction() * distance;
			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
inline bool Intersection::test_ray_sphere(const Ray& r, const Sphere& s, float& distance, Vector3& intersectionPoint)
{
	Vector3 v = s.center() - r.origin();
	float b = v.dot(r.direction());
	float det = (s.radius() * s.radius()) - v.dot(v) + (b * b);

	if (det < 0.0 || b < s.radius())
	{
		return false;
	}

	distance = b - math::sqrt(det);
	intersectionPoint = r.origin() + r.direction() * distance;

	return true;
}

//-----------------------------------------------------------------------------
inline bool Intersection::test_ray_box(const Ray& r, const Box& b, float& /*distance*/, Vector3& /*intersectionPoint*/)
{
	if (r.origin().x < b.min().x)
	{
		if (r.direction().x < 0.0)
		{
			return false;
		}
	}

	if (r.origin().x > b.max().x)
	{
		if (r.direction().x > 0.0)
		{
			return false;
		}
	}

	if (r.origin().y < b.min().y)
	{
		if (r.direction().y < 0.0)
		{
			return false;
		}
	}

	if (r.origin().y > b.max().y)
	{
		if (r.direction().y > 0.0)
		{
			return false;
		}
	}

	if (r.origin().z < b.min().z)
	{
		if (r.direction().z < 0.0)
		{
			return false;
		}
	}

	if (r.origin().z > b.max().z)
	{
		if (r.direction().z > 0.0)
		{
			return false;
		}
	}

	// Possibly int32_tersects
	return true;

	// TODO
}

//-----------------------------------------------------------------------------
inline bool Intersection::test_ray_triangle(const Ray& r, const Triangle& t, float& distance, Vector3& intersectionPoint)
{
	if (Intersection::test_ray_plane(r, t.to_plane(), distance, intersectionPoint))
	{
		if (t.contains_point(intersectionPoint))
		{
			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
inline bool Intersection::test_plane_3(const Plane& p1, const Plane& p2, const Plane& p3, Vector3& ip)
{
	const Vector3& n1 = p1.n;
	const Vector3& n2 = p2.n;
	const Vector3& n3 = p3.n;

	float den = -n1.cross(n2).dot(n3);

	if (math::equals(den, (float)0.0))
	{
		return false;
	}

	Vector3 res = p1.d * n2.cross(n3) + p2.d * n3.cross(n1) + p3.d * n1.cross(n2);
	ip = res / den;

	return true;
}

//-----------------------------------------------------------------------------
inline bool Intersection::test_static_sphere_plane(const Sphere& s, const Plane& p)
{
	if (math::abs(p.distance_to_point(s.center())) < s.radius())
	{
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
inline bool Intersection::test_static_sphere_sphere(const Sphere& a, const Sphere& b)
{
	float dist = (b.center() - a.center()).squared_length();
	return (dist < (b.radius() + a.radius()) * (b.radius() + a.radius()));
}

//-----------------------------------------------------------------------------
inline bool Intersection::test_dynamic_sphere_plane(const Sphere& s, const Vector3& d, const Plane& p, float& it, Vector3& intersectionPoint)
{
	const Vector3& sphereCenter = s.center();
	const float sphereRadius = s.radius();

	float t0;	// Time at which the sphere int32_tersects the plane remaining at the front side of the plane
	float t1;	// Time at which the sphere int32_tersects the plane remaining at the back side of the plane

	float sphereToPlaneDistance = p.distance_to_point(sphereCenter);
	float planeNormalDotVelocity = p.n.dot(d);

	if (planeNormalDotVelocity > 0.0)
	{
		return false;
	}

	// If the sphere is travelling parallel to the plane
	if (planeNormalDotVelocity == 0.0)
	{
		// If the sphere is embedded in the plane
		if (math::abs(sphereToPlaneDistance) < sphereRadius)
		{
			t0 = 0.0;
			t1 = 1.0;

			it = t0;
			intersectionPoint = s.center() - p.n * s.radius();
			return true;
		}

		return false;
	}

	t0 = (sphereRadius - sphereToPlaneDistance) / planeNormalDotVelocity;
	t1 = (-sphereRadius - sphereToPlaneDistance) / planeNormalDotVelocity;

	// If _both_ t0 and t1 are outside [0,1] then collision can never happen
	if (t0 >= 0.0 && t0 <= 1.0)
	{
		it = math::min(t0, t1);
		intersectionPoint = s.center() - p.n * s.radius() + (d * it);
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
inline bool Intersection::test_dynamic_sphere_triangle(const Sphere& s, const Vector3& d, const Triangle& tri, float& it, Vector3& intersectionPoint)
{
	Plane triPlane = tri.to_plane();

	// Test against the plane containing the triangle
	float spherePlaneIt;
	if (!test_dynamic_sphere_plane(s, d, triPlane, spherePlaneIt, intersectionPoint))
	{
		return false;
	}

	// Check if the int32_tersection point32_t lies inside the triangle
	if (tri.contains_point(intersectionPoint))
	{
		it = spherePlaneIt;
		// intersectionPoint is already returned by the above call to test_dynamic_sphere_plane
		return true;
	}

	it = spherePlaneIt;

	// Sweep test
	// Check for collision against the vertices
	bool collisionFound = false;
	float a, b, c;
	float x1, x2;

	// v1
	a = d.dot(d);
	b = 2.0 * (d.dot(s.center() - tri.m_vertex[0]));
	c = (tri.m_vertex[0] - s.center()).dot(tri.m_vertex[0] - s.center()) - (s.radius() * s.radius());

	if (math::solve_quadratic_equation(a, b, c, x1, x2))
	{
		it = math::min(x1, it);
		intersectionPoint = tri.m_vertex[0];
		collisionFound = true;
	}

	// v2
	b = 2.0 * (d.dot(s.center() - tri.m_vertex[1]));
	c = (tri.m_vertex[1] - s.center()).dot(tri.m_vertex[1] - s.center()) - (s.radius() * s.radius());

	if (math::solve_quadratic_equation(a, b, c, x1, x2))
	{
		it = math::min(x1, it);
		intersectionPoint = tri.m_vertex[1];
		collisionFound = true;
	}

	// v3
	b = 2.0 * (d.dot(s.center() - tri.m_vertex[2]));
	c = (tri.m_vertex[2] - s.center()).dot(tri.m_vertex[2] - s.center()) - (s.radius() * s.radius());

	if (math::solve_quadratic_equation(a, b, c, x1, x2))
	{
		it = math::min(x1, it);
		intersectionPoint = tri.m_vertex[2];
		collisionFound = true;
	}

	// Check for collisions against the edges
	Vector3 edge;
	Vector3 centerToVertex;
	float edgeDotVelocity;
	float edgeDotCenterToVertex;
	float edgeSquaredLength;
	float velocitySquaredLength;

	velocitySquaredLength = d.squared_length();

	// e1
	edge = tri.m_vertex[1] - tri.m_vertex[0];
	centerToVertex = tri.m_vertex[0] - s.center();
	edgeDotVelocity = edge.dot(d);
	edgeDotCenterToVertex = edge.dot(centerToVertex);
	edgeSquaredLength = edge.squared_length();


	a = edgeSquaredLength * -velocitySquaredLength + edgeDotVelocity * edgeDotVelocity;
	b = edgeSquaredLength * (2.0 * d.dot(centerToVertex)) - (2.0 * edgeDotVelocity * edgeDotCenterToVertex);

	c = edgeSquaredLength * ((s.radius() * s.radius()) - centerToVertex.squared_length()) + (edgeDotCenterToVertex * edgeDotCenterToVertex);

	if (math::solve_quadratic_equation(a, b, c, x1, x2))
	{
		float f0 = (edgeDotVelocity * x1 - edgeDotCenterToVertex) / edgeSquaredLength;

		if (f0 >= 0.0 && f0 <= 1.0)
		{
			it = math::min(x1, it);
			intersectionPoint = tri.m_vertex[0] + f0 * edge;
			collisionFound = true;
		}
	}

	// e2
	edge = tri.m_vertex[2] - tri.m_vertex[1];
	centerToVertex = tri.m_vertex[1] - s.center();
	edgeDotVelocity = edge.dot(d);
	edgeDotCenterToVertex = edge.dot(centerToVertex);
	edgeSquaredLength = edge.squared_length();


	a = edgeSquaredLength * -velocitySquaredLength + edgeDotVelocity * edgeDotVelocity;
	b = edgeSquaredLength * (2.0 * d.dot(centerToVertex)) - (2.0 * edgeDotVelocity * edgeDotCenterToVertex);

	c = edgeSquaredLength * ((s.radius() * s.radius()) - centerToVertex.squared_length()) + (edgeDotCenterToVertex * edgeDotCenterToVertex);

	if (math::solve_quadratic_equation(a, b, c, x1, x2))
	{
		float f0 = (edgeDotVelocity * x1 - edgeDotCenterToVertex) / edgeSquaredLength;

		if (f0 >= 0.0 && f0 <= 1.0)
		{
			it = math::min(x1, it);
			intersectionPoint = tri.m_vertex[1] + f0 * edge;
			collisionFound = true;
		}
	}

	// e3
	edge = tri.m_vertex[0] - tri.m_vertex[2];
	centerToVertex = tri.m_vertex[2] - s.center();
	edgeDotVelocity = edge.dot(d);
	edgeDotCenterToVertex = edge.dot(centerToVertex);
	edgeSquaredLength = edge.squared_length();


	a = edgeSquaredLength * -velocitySquaredLength + edgeDotVelocity * edgeDotVelocity;
	b = edgeSquaredLength * (2.0 * d.dot(centerToVertex)) - (2.0 * edgeDotVelocity * edgeDotCenterToVertex);

	c = edgeSquaredLength * ((s.radius() * s.radius()) - centerToVertex.squared_length()) + (edgeDotCenterToVertex * edgeDotCenterToVertex);

	if (math::solve_quadratic_equation(a, b, c, x1, x2))
	{
		float f0 = (edgeDotVelocity * x1 - edgeDotCenterToVertex) / edgeSquaredLength;

		if (f0 >= 0.0 && f0 <= 1.0)
		{
			it = math::min(x1, it);
			intersectionPoint = tri.m_vertex[2] + f0 * edge;
			collisionFound = true;
		}
	}

	return collisionFound;
}

//-----------------------------------------------------------------------------
inline bool Intersection::test_dynamic_sphere_sphere(const Sphere& s1, const Vector3& d1, const Sphere& s2, const Vector3& d2, float& it, Vector3& /*intersectionPoint*/)
{
	// s1 == static sphere
	// s2 == moving sphere
	Vector3 d = d2 - d1;
	d.normalize();

	const Vector3& cs = s1.center();
	const Vector3& cm = s2.center();

	Vector3 e = cs - cm;
	float r = s1.radius() + s2.radius();

	// If ||e|| < r, int32_tersection occurs at t = 0
	if (e.length() < r)
	{
		it = 0.0;
		return true;
	}

	// it == Intersection Time
	float ed = e.dot(d);
	float squared = (ed * ed) + (r * r) - e.dot(e);

	// If the value inside the square root is neg, then no int32_tersection
	if (squared < 0.0)
	{
		return false;
	}

	float t = ed - math::sqrt(squared);
	float l = (d2 - d1).length();

	// If t < 0 || t > l, then non int32_tersection in the considered period of time
	if (t < 0.0 || t > l)
	{
		return false;
	}

	it = t / l;
	return true;
}

//-----------------------------------------------------------------------------
inline bool Intersection::test_static_box_box(const Box& b1, const Box& b2)
{
	if (b1.min().x > b2.max().x || b1.max().x < b2.min().x)
	{
		return false;
	}

	if (b1.min().y > b2.max().y || b1.max().y < b2.min().y)
	{
		return false;
	}

	if (b1.min().z > b2.max().z || b1.max().z < b2.min().z)
	{
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
inline bool Intersection::test_dynamic_box_box(const Box& b1, const Vector3& v1, const Box& b2, const Vector3& v2, float& it)
{
	// b1 == static box
	// b2 == moving box
	Vector3 d = v2 - v1;

	// Start time of int32_tersection aint64_t each axis
	Vector3 tEnterXYZ(0.0, 0.0, 0.0);
	// Stop time of int32_tersection aint64_t each axis
	Vector3 tLeaveXYZ(1.0, 1.0, 1.0);

	// If the resulting displacement equals zero, then fallback to static int32_tersection test
	if (math::equals(d.x, (float)0.0))
	{
		if (b1.min().x > b2.max().x || b1.max().x < b2.min().x)
		{
			return false;
		}
	}

	if (math::equals(d.y, (float)0.0))
	{
		if (b1.min().y > b2.max().y || b1.max().y < b2.min().y)
		{
			return false;
		}
	}

	if (math::equals(d.z, (float)0.0))
	{
		if (b1.min().z > b2.max().z || b1.max().z < b2.min().z)
		{
			return false;
		}
	}

	// Otherwise, compute the enter/leave times aint64_t each axis
	float oneOverD = (float)(1.0 / d.x);
	tEnterXYZ.x = (b1.min().x - b2.max().x) * oneOverD;
	tLeaveXYZ.x = (b1.max().x - b2.min().x) * oneOverD;

	oneOverD = (float)(1.0 / d.y);
	tEnterXYZ.y = (b1.min().y - b2.max().y) * oneOverD;
	tLeaveXYZ.y = (b1.max().y - b2.min().y) * oneOverD;

	oneOverD = (float)(1.0 / d.z);
	tEnterXYZ.z = (b1.min().z - b2.max().z) * oneOverD;
	tLeaveXYZ.z = (b1.max().z - b2.min().z) * oneOverD;

	// We must ensure that enter time < leave time
	if (tLeaveXYZ.x < tEnterXYZ.x)
	{
		math::swap(tLeaveXYZ.x, tEnterXYZ.x);
	}

	if (tLeaveXYZ.y < tEnterXYZ.y)
	{
		math::swap(tLeaveXYZ.y, tEnterXYZ.y);
	}

	if (tLeaveXYZ.z < tEnterXYZ.z)
	{
		math::swap(tLeaveXYZ.z, tEnterXYZ.z);
	}

	float tEnter = math::max(tEnterXYZ.x, math::max(tEnterXYZ.y, tEnterXYZ.z));
	float tLeave = math::min(tLeaveXYZ.x, math::min(tLeaveXYZ.y, tLeaveXYZ.z));

	// If tEnter > 1, then there is no int32_tersection in the period
	// of time cosidered
	if (tEnter > 1.0)
	{
		return false;
	}

	it = tEnter;

	return tEnter < tLeave;
}

//-----------------------------------------------------------------------------
inline bool Intersection::test_frustum_sphere(const Frustum& f, const Sphere& s)
{
	if (f.m_planes[0].distance_to_point(s.center()) < -s.radius() || f.m_planes[1].distance_to_point(s.center()) < -s.radius())
	{
		return false;
	}

	if (f.m_planes[2].distance_to_point(s.center()) < -s.radius() || f.m_planes[3].distance_to_point(s.center()) < -s.radius())
	{
		return false;
	}

	if (f.m_planes[4].distance_to_point(s.center()) < -s.radius() || f.m_planes[5].distance_to_point(s.center()) < -s.radius())
	{
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
inline bool Intersection::test_frustum_box(const Frustum& f, const Box& b)
{
	uint32_t vertexOutCount;

	for (uint32_t i = 0; i < 6; i++)
	{
		vertexOutCount = 0;

		for (uint32_t j = 0; j < 8; j++)
		{
			if (f.m_planes[i].distance_to_point(b.vertex(j)) < 0.0)
			{
				vertexOutCount++;
			}
		}

		// If all vertices are outside one face, then the box doesn't int32_tersect the frustum
		if (vertexOutCount == 8)
		{
			return false;
		}
	}

	// If we are here, is because either the box int32_tersects or it is contained in the frustum
	return true;
}

//-----------------------------------------------------------------------------
inline bool Intersection::test_circle_circle(const Circle& c1, const Circle& c2, Vector2& penetration)
{
	Vector2 distance = c1.center() - c2.center();
	float distanceLen2 = vector2::squared_length(distance);
	float radiusSum = c1.radius() + c2.radius();
	if (distanceLen2 > radiusSum*radiusSum)
	{
		return false;
	}

	if (distanceLen2 < 0.001)
	{
		penetration = Vector2(c1.radius(), 0.0);
	}
	else
	{
		distanceLen2 = math::sqrt(distanceLen2);
		penetration = distance * ((radiusSum - distanceLen2) / distanceLen2);
	}
	return true;
}

//-----------------------------------------------------------------------------
inline bool Intersection::test_dynamic_circle_circle(const Circle& c1, const Vector2& d1, const Circle& c2, const Vector2& d2, float& it)
{
	// c1 == static circle
	// c2 == moving circle
	Vector2 d = d2 - d1;
	vector2::normalize(d);

	const Vector2& cs = c1.center();
	const Vector2& cm = c2.center();

	Vector2 e = cs - cm;
	float r = c1.radius() + c2.radius();

	// If ||e|| < r, int32_tersection occurs at t = 0
	if (vector2::length(e) < r)
	{
		it = 0.0;
		return true;
	}

	// it == Intersection Time
	float ed = vector2::dot(e, d);
	float squared = (ed * ed) + (r * r) - vector2::dot(e, e);

	// If the value inside the square root is neg, then no int32_tersection
	if (squared < 0.0)
	{
		return false;
	}

	float t = ed - math::sqrt(squared);
	float l = vector2::length(d2 - d1);

	// If t < 0 || t > l, then non int32_tersection in the considered period of time
	if (t < 0.0 || t > l)
	{
		return false;
	}

	it = t / l;
	return true;
}

//-----------------------------------------------------------------------------
inline bool Intersection::test_rect_rect(const Rect& r1, const Rect& r2, Vector2& penetration)
{
	//x
	float min1MinusMax2 = r1.min().x - r2.max().x;
	float min2MinusMax1 = r2.min().x - r1.max().x;

	if (min1MinusMax2 > min2MinusMax1)
	{
		if (min1MinusMax2 > 0)
		{
			return false;
		}
		penetration.x = -min1MinusMax2;
	}
	else
	{
		if (min2MinusMax1 > 0)
		{
			return false;
		}
		penetration.x = min2MinusMax1;
	}

	//y
	min1MinusMax2 = r1.min().y - r2.max().y;
	min2MinusMax1 = r2.min().y - r1.max().y;

	if (min1MinusMax2 > min2MinusMax1)
	{
		if (min1MinusMax2 > 0)
		{
			return false;
		}
		penetration.y = -min1MinusMax2;
	}
	else
	{
		if (min2MinusMax1 > 0)
		{
			return false;
		}
		penetration.y = min2MinusMax1;
	}

	if (math::abs(penetration.x) < math::abs(penetration.y))
	{
		penetration.y = 0.0;
	}
	else
	{
		penetration.x = 0.0;
	}

	return true;
}

//-----------------------------------------------------------------------------
inline bool Intersection::test_circle_rect(const Circle& c1, const Rect& r2, Vector2& penetration)
{
	bool circleIsAtRight;
	if (c1.center().x > (r2.min().x + r2.max().x) / 2)
	{
		penetration.x = (c1.center().x - c1.radius()) - r2.max().x;
		circleIsAtRight = true;
	}
	else
	{
		penetration.x = r2.min().x - (c1.center().x + c1.radius());
		circleIsAtRight = false;
	}

	bool circleIsAtTop;
	if (c1.center().y > (r2.min().y + r2.max().y) / 2)
	{
		penetration.y = (c1.center().y - c1.radius()) - r2.max().y;
		circleIsAtTop = true;
	}
	else
	{
		penetration.y = r2.min().y - (c1.center().y + c1.radius());
		circleIsAtTop = false;
	}

	if (penetration.x < -c1.radius() || penetration.y < -c1.radius())
	{
		if (penetration.y > 0 || penetration.x > 0)
		{
			return false;
		}
		if (penetration.x > penetration.y)
		{
			penetration.y = 0;
		}
		else
		{
			penetration.x = 0;
		}
	}
	//else if (penetration.y < -c1.radius())
	//{
	//	if (penetration.x > 0)
	//	{
	//		return false;
	//	}
	//	penetration.y = 0;
	//}
	else
	{
		penetration += Vector2(c1.radius(), c1.radius());
		float len = math::sqrt(vector2::squared_length(penetration));
		if (len > c1.radius())
		{
			return false;
		}
		//The - is to point32_t outwards
		penetration *= - (c1.radius() - len) / len;
	}

	if (circleIsAtRight)
	{
		penetration.x *= -1;
	}

	if (circleIsAtTop)
	{
		penetration.y *= -1;
	}
	
	return true;
}

} // namespace crown

