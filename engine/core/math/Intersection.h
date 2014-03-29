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
#include "AABB.h"

namespace crown
{
class Intersection
{
public:

	static bool test_ray_plane(const Ray& r, const Plane& p, float& distance, Vector3& inttersectionPoint_t);
	static bool test_ray_sphere(const Ray& r, const Sphere& s, float& distance, Vector3& intersectionPoint);
	static bool test_ray_box(const Ray& r, const AABB& b, float& distance, Vector3& intersectionPoint);

	static bool test_plane_3(const Plane& p1, const Plane& p2, const Plane& p3, Vector3& ip);

	static bool test_static_sphere_plane(const Sphere& s, const Plane& p);
	static bool test_static_sphere_sphere(const Sphere& a, const Sphere& b);
	static bool test_dynamic_sphere_plane(const Sphere& s, const Vector3& d, const Plane& p, float& it, Vector3& intersectionPoint);
	static bool test_dynamic_sphere_sphere(const Sphere& s1, const Vector3& d1, const Sphere& s2, const Vector3& d2, float& it, Vector3& intersectionPoint);

	static bool test_static_box_box(const AABB& b1, const AABB& b2);
	static bool test_dynamic_box_box(const AABB& b1, const Vector3& v1, const AABB& b2, const Vector3& v2, float& it);

	static bool test_frustum_sphere(const Frustum& f, const Sphere& s);
	static bool test_frustum_box(const Frustum& f, const AABB& box);

private:

	// Disable construction
	Intersection();
};

//-----------------------------------------------------------------------------
inline bool Intersection::test_ray_plane(const Ray& r, const Plane& p, float& distance, Vector3& intersectionPoint)
{
	float nd = vector3::dot(r.direction(), p.n);
	float orpn = vector3::dot(r.origin(), p.n);

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
	float b = vector3::dot(v, r.direction());
	float det = (s.radius() * s.radius()) - vector3::dot(v, v) + (b * b);

	if (det < 0.0 || b < s.radius())
	{
		return false;
	}

	distance = b - math::sqrt(det);
	intersectionPoint = r.origin() + r.direction() * distance;

	return true;
}

//-----------------------------------------------------------------------------
inline bool Intersection::test_ray_box(const Ray& r, const AABB& b, float& /*distance*/, Vector3& /*intersectionPoint*/)
{
	if (r.origin().x < b.min.x)
	{
		if (r.direction().x < 0.0)
		{
			return false;
		}
	}

	if (r.origin().x > b.max.x)
	{
		if (r.direction().x > 0.0)
		{
			return false;
		}
	}

	if (r.origin().y < b.min.y)
	{
		if (r.direction().y < 0.0)
		{
			return false;
		}
	}

	if (r.origin().y > b.max.y)
	{
		if (r.direction().y > 0.0)
		{
			return false;
		}
	}

	if (r.origin().z < b.min.z)
	{
		if (r.direction().z < 0.0)
		{
			return false;
		}
	}

	if (r.origin().z > b.max.z)
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
inline bool Intersection::test_plane_3(const Plane& p1, const Plane& p2, const Plane& p3, Vector3& ip)
{
	const Vector3& n1 = p1.n;
	const Vector3& n2 = p2.n;
	const Vector3& n3 = p3.n;

	float den = -vector3::dot(vector3::cross(n1, n2), n3);

	if (math::equals(den, (float)0.0))
	{
		return false;
	}

	Vector3 res = p1.d * vector3::cross(n2, n3) + p2.d * vector3::cross(n3, n1) + p3.d * vector3::cross(n1, n2);
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
	float dist = vector3::squared_length(b.center() - a.center());
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
	float planeNormalDotVelocity = vector3::dot(p.n, d);

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
inline bool Intersection::test_dynamic_sphere_sphere(const Sphere& s1, const Vector3& d1, const Sphere& s2, const Vector3& d2, float& it, Vector3& /*intersectionPoint*/)
{
	// s1 == static sphere
	// s2 == moving sphere
	Vector3 d = d2 - d1;
	vector3::normalize(d);

	const Vector3& cs = s1.center();
	const Vector3& cm = s2.center();

	Vector3 e = cs - cm;
	float r = s1.radius() + s2.radius();

	// If ||e|| < r, int32_tersection occurs at t = 0
	if (vector3::length(e) < r)
	{
		it = 0.0;
		return true;
	}

	// it == Intersection Time
	float ed = vector3::dot(e, d);
	float squared = (ed * ed) + (r * r) - vector3::dot(e, e);

	// If the value inside the square root is neg, then no int32_tersection
	if (squared < 0.0)
	{
		return false;
	}

	float t = ed - math::sqrt(squared);
	float l = vector3::length(d2 - d1);

	// If t < 0 || t > l, then non int32_tersection in the considered period of time
	if (t < 0.0 || t > l)
	{
		return false;
	}

	it = t / l;
	return true;
}

//-----------------------------------------------------------------------------
inline bool Intersection::test_static_box_box(const AABB& b1, const AABB& b2)
{
	if (b1.min.x > b2.max.x || b1.max.x < b2.min.x)
	{
		return false;
	}

	if (b1.min.y > b2.max.y || b1.max.y < b2.min.y)
	{
		return false;
	}

	if (b1.min.z > b2.max.z || b1.max.z < b2.min.z)
	{
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
inline bool Intersection::test_dynamic_box_box(const AABB& b1, const Vector3& v1, const AABB& b2, const Vector3& v2, float& it)
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
		if (b1.min.x > b2.max.x || b1.max.x < b2.min.x)
		{
			return false;
		}
	}

	if (math::equals(d.y, (float)0.0))
	{
		if (b1.min.y > b2.max.y || b1.max.y < b2.min.y)
		{
			return false;
		}
	}

	if (math::equals(d.z, (float)0.0))
	{
		if (b1.min.z > b2.max.z || b1.max.z < b2.min.z)
		{
			return false;
		}
	}

	// Otherwise, compute the enter/leave times aint64_t each axis
	float oneOverD = (float)(1.0 / d.x);
	tEnterXYZ.x = (b1.min.x - b2.max.x) * oneOverD;
	tLeaveXYZ.x = (b1.max.x - b2.min.x) * oneOverD;

	oneOverD = (float)(1.0 / d.y);
	tEnterXYZ.y = (b1.min.y - b2.max.y) * oneOverD;
	tLeaveXYZ.y = (b1.max.y - b2.min.y) * oneOverD;

	oneOverD = (float)(1.0 / d.z);
	tEnterXYZ.z = (b1.min.z - b2.max.z) * oneOverD;
	tLeaveXYZ.z = (b1.max.z - b2.min.z) * oneOverD;

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
inline bool Intersection::test_frustum_box(const Frustum& f, const AABB& b)
{
	uint32_t vertexOutCount;

	for (uint32_t i = 0; i < 6; i++)
	{
		vertexOutCount = 0;

		for (uint32_t j = 0; j < 8; j++)
		{
			if (f.m_planes[i].distance_to_point(aabb::vertex(b, j)) < 0.0)
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

} // namespace crown
