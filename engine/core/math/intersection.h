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

#include "plane.h"
#include "sphere.h"
#include "frustum.h"
#include "math_utils.h"
#include "types.h"
#include "aabb.h"

namespace crown
{
namespace Intersection
{
	/// Returns the distance along ray (from, dir) to intersection point with plane @a p.
	/// -1.0f if no collision.
	inline float ray_plane_intersection(const Vector3& from, const Vector3& dir, const Plane& p)
	{
		float nd = vector3::dot(dir, p.n);
		float orpn = vector3::dot(from, p.n);
		float dist = -1.0f;

		if (nd < 0.0f)
			dist = (-p.d - orpn) / nd;

		return dist > 0.0f ? dist : -1.0f;
	}
	
	//-----------------------------------------------------------------------------
	inline bool test_ray_sphere(const Vector3& from, const Vector3& dir, const Sphere& s, float& distance, Vector3& intersectionPoint)
	{
		Vector3 v = s.center() - from;
		float b = vector3::dot(v, dir);
		float det = (s.radius() * s.radius()) - vector3::dot(v, v) + (b * b);

		if (det < 0.0 || b < s.radius())
		{
			return false;
		}

		distance = b - math::sqrt(det);
		intersectionPoint = from + dir * distance;

		return true;
	}

	//-----------------------------------------------------------------------------
	inline bool test_plane_3(const Plane& p1, const Plane& p2, const Plane& p3, Vector3& ip)
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
	inline bool test_static_sphere_plane(const Sphere& s, const Plane& p)
	{
		if (math::abs(plane::distance_to_point(p, s.center())) < s.radius())
		{
			return true;
		}

		return false;
	}

	//-----------------------------------------------------------------------------
	inline bool test_static_sphere_sphere(const Sphere& a, const Sphere& b)
	{
		float dist = vector3::squared_length(b.center() - a.center());
		return (dist < (b.radius() + a.radius()) * (b.radius() + a.radius()));
	}

	//-----------------------------------------------------------------------------
	inline bool test_dynamic_sphere_plane(const Sphere& s, const Vector3& d, const Plane& p, float& it, Vector3& intersectionPoint)
	{
		const Vector3& sphereCenter = s.center();
		const float sphereRadius = s.radius();

		float t0;	// Time at which the sphere int32_tersects the plane remaining at the front side of the plane
		float t1;	// Time at which the sphere int32_tersects the plane remaining at the back side of the plane

		float sphereToPlaneDistance = plane::distance_to_point(p, sphereCenter);
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
	inline bool test_dynamic_sphere_sphere(const Sphere& s1, const Vector3& d1, const Sphere& s2, const Vector3& d2, float& it, Vector3& /*intersectionPoint*/)
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
	inline bool test_static_box_box(const AABB& b1, const AABB& b2)
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
	inline bool test_frustum_sphere(const Frustum& f, const Sphere& s)
	{
		if (plane::distance_to_point(f.left, s.center()) < -s.radius() ||
			plane::distance_to_point(f.right, s.center()) < -s.radius())
		{
			return false;
		}

		if (plane::distance_to_point(f.bottom, s.center()) < -s.radius() ||
			plane::distance_to_point(f.top, s.center()) < -s.radius())
		{
			return false;
		}

		if (plane::distance_to_point(f.near, s.center()) < -s.radius() ||
			plane::distance_to_point(f.far, s.center()) < -s.radius())
		{
			return false;
		}

		return true;
	}

	//-----------------------------------------------------------------------------
	inline bool test_frustum_box(const Frustum& f, const AABB& b)
	{
		uint8_t out;

		out = 0;
		if (plane::distance_to_point(f.left, aabb::vertex(b, 0)) < 0.0) out++;
		if (plane::distance_to_point(f.left, aabb::vertex(b, 1)) < 0.0) out++;
		if (plane::distance_to_point(f.left, aabb::vertex(b, 2)) < 0.0) out++;
		if (plane::distance_to_point(f.left, aabb::vertex(b, 3)) < 0.0) out++;
		if (plane::distance_to_point(f.left, aabb::vertex(b, 4)) < 0.0) out++;
		if (plane::distance_to_point(f.left, aabb::vertex(b, 5)) < 0.0) out++;
		if (plane::distance_to_point(f.left, aabb::vertex(b, 6)) < 0.0) out++;
		if (plane::distance_to_point(f.left, aabb::vertex(b, 7)) < 0.0) out++;

		// If all vertices are outside one face, then the box doesn't intersect the frustum
		if (out == 8) return false;

		out = 0;
		if (plane::distance_to_point(f.right, aabb::vertex(b, 0)) < 0.0) out++;
		if (plane::distance_to_point(f.right, aabb::vertex(b, 1)) < 0.0) out++;
		if (plane::distance_to_point(f.right, aabb::vertex(b, 2)) < 0.0) out++;
		if (plane::distance_to_point(f.right, aabb::vertex(b, 3)) < 0.0) out++;
		if (plane::distance_to_point(f.right, aabb::vertex(b, 4)) < 0.0) out++;
		if (plane::distance_to_point(f.right, aabb::vertex(b, 5)) < 0.0) out++;
		if (plane::distance_to_point(f.right, aabb::vertex(b, 6)) < 0.0) out++;
		if (plane::distance_to_point(f.right, aabb::vertex(b, 7)) < 0.0) out++;
		if (out == 8) return false;

		out = 0;
		if (plane::distance_to_point(f.bottom, aabb::vertex(b, 0)) < 0.0) out++;
		if (plane::distance_to_point(f.bottom, aabb::vertex(b, 1)) < 0.0) out++;
		if (plane::distance_to_point(f.bottom, aabb::vertex(b, 2)) < 0.0) out++;
		if (plane::distance_to_point(f.bottom, aabb::vertex(b, 3)) < 0.0) out++;
		if (plane::distance_to_point(f.bottom, aabb::vertex(b, 4)) < 0.0) out++;
		if (plane::distance_to_point(f.bottom, aabb::vertex(b, 5)) < 0.0) out++;
		if (plane::distance_to_point(f.bottom, aabb::vertex(b, 6)) < 0.0) out++;
		if (plane::distance_to_point(f.bottom, aabb::vertex(b, 7)) < 0.0) out++;
		if (out == 8) return false;

		out = 0;
		if (plane::distance_to_point(f.top, aabb::vertex(b, 0)) < 0.0) out++;
		if (plane::distance_to_point(f.top, aabb::vertex(b, 1)) < 0.0) out++;
		if (plane::distance_to_point(f.top, aabb::vertex(b, 2)) < 0.0) out++;
		if (plane::distance_to_point(f.top, aabb::vertex(b, 3)) < 0.0) out++;
		if (plane::distance_to_point(f.top, aabb::vertex(b, 4)) < 0.0) out++;
		if (plane::distance_to_point(f.top, aabb::vertex(b, 5)) < 0.0) out++;
		if (plane::distance_to_point(f.top, aabb::vertex(b, 6)) < 0.0) out++;
		if (plane::distance_to_point(f.top, aabb::vertex(b, 7)) < 0.0) out++;
		if (out == 8) return false;

		out = 0;
		if (plane::distance_to_point(f.near, aabb::vertex(b, 0)) < 0.0) out++;
		if (plane::distance_to_point(f.near, aabb::vertex(b, 1)) < 0.0) out++;
		if (plane::distance_to_point(f.near, aabb::vertex(b, 2)) < 0.0) out++;
		if (plane::distance_to_point(f.near, aabb::vertex(b, 3)) < 0.0) out++;
		if (plane::distance_to_point(f.near, aabb::vertex(b, 4)) < 0.0) out++;
		if (plane::distance_to_point(f.near, aabb::vertex(b, 5)) < 0.0) out++;
		if (plane::distance_to_point(f.near, aabb::vertex(b, 6)) < 0.0) out++;
		if (plane::distance_to_point(f.near, aabb::vertex(b, 7)) < 0.0) out++;
		if (out == 8) return false;

		out = 0;
		if (plane::distance_to_point(f.far, aabb::vertex(b, 0)) < 0.0) out++;
		if (plane::distance_to_point(f.far, aabb::vertex(b, 1)) < 0.0) out++;
		if (plane::distance_to_point(f.far, aabb::vertex(b, 2)) < 0.0) out++;
		if (plane::distance_to_point(f.far, aabb::vertex(b, 3)) < 0.0) out++;
		if (plane::distance_to_point(f.far, aabb::vertex(b, 4)) < 0.0) out++;
		if (plane::distance_to_point(f.far, aabb::vertex(b, 5)) < 0.0) out++;
		if (plane::distance_to_point(f.far, aabb::vertex(b, 6)) < 0.0) out++;
		if (plane::distance_to_point(f.far, aabb::vertex(b, 7)) < 0.0) out++;
		if (out == 8) return false;

		// If we are here, it is because either the box intersects or it is contained in the frustum
		return true;
	}
} // namespace Intersection
} // namespace crown
