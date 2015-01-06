/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "intersection.h"
#include "aabb.h"
#include "plane.h"
#include "sphere.h"
#include "vector3.h"

namespace crown
{

/// Returns the distance along ray (from, dir) to intersection point with plane @a p.
/// -1.0f if no collision.
float ray_plane_intersection(const Vector3& from, const Vector3& dir, const Plane& p)
{
	float nd = vector3::dot(dir, p.n);
	float orpn = vector3::dot(from, p.n);
	float dist = -1.0f;

	if (nd < 0.0f)
		dist = (-p.d - orpn) / nd;

	return dist > 0.0f ? dist : -1.0f;
}

/// Returns the distance along ray (from, dir) to intersection point with sphere @a s.
/// -1.0f if no collision.
float ray_sphere_intersection(const Vector3& from, const Vector3& dir, const Sphere& s)
{
	Vector3 v = s.c - from;
	float b = vector3::dot(v, dir);
	float det = (s.r * s.r) - vector3::dot(v, v) + (b * b);

	if (det < 0.0 || b < s.r)
	{
		return -1.0f;
	}

	return b - sqrt(det);
}

// http://www.opengl-tutorial.org/miscellaneous/clicking-on-objects/picking-with-custom-ray-obb-function/
float ray_oobb_intersection(const Vector3& from, const Vector3& dir, const OBB& obb)
{
	using namespace vector3;

	float tmin = 0.0f;
	float tmax = 100000.0f;

	Vector3 obb_pos(obb.tm.t.x, obb.tm.t.y, obb.tm.t.z);
	Vector3 delta = obb_pos - from;

	{
		const Vector3 xaxis(obb.tm.x.x, obb.tm.x.y, obb.tm.x.z);
		float e = dot(xaxis, delta);
		float f = dot(dir, xaxis);

		if (fabs(f) > 0.001f)
		{
			float t1 = (e+obb.aabb.min.x)/f;
			float t2 = (e+obb.aabb.max.x)/f;

			if (t1>t2){
				float w=t1;t1=t2;t2=w;
			}

			if (t2 < tmax)
				tmax = t2;
			if (t1 > tmin)
				tmin = t1;

			if (tmax < tmin)
				return -1.0f;

		}
		else
		{
			if(-e+obb.aabb.min.x > 0.0f || -e+obb.aabb.max.x < 0.0f)
				return -1.0f;
		}
	}

	{
		const Vector3 yaxis(obb.tm.y.x, obb.tm.y.y, obb.tm.y.z);
		float e = dot(yaxis, delta);
		float f = dot(dir, yaxis);

		if (fabs(f) > 0.001f){

			float t1 = (e+obb.aabb.min.y)/f;
			float t2 = (e+obb.aabb.max.y)/f;

			if (t1>t2){float w=t1;t1=t2;t2=w;}

			if (t2 < tmax)
				tmax = t2;
			if (t1 > tmin)
				tmin = t1;

			if (tmin > tmax)
				return -1.0f;
		}
		else
		{
			if(-e+obb.aabb.min.y > 0.0f || -e+obb.aabb.max.y < 0.0f)
				return -1.0f;
		}
	}

	{
		const Vector3 zaxis(obb.tm.z.x, obb.tm.z.y, obb.tm.z.z);
		float e = dot(zaxis, delta);
		float f = dot(dir, zaxis);

		if (fabs(f) > 0.001f){

			float t1 = (e+obb.aabb.min.z)/f;
			float t2 = (e+obb.aabb.max.z)/f;

			if (t1>t2){float w=t1;t1=t2;t2=w;}

			if (t2 < tmax)
				tmax = t2;
			if (t1 > tmin)
				tmin = t1;

			if (tmin > tmax)
				return -1.0f;

		}
		else
		{
			if(-e+obb.aabb.min.z > 0.0f || -e+obb.aabb.max.z < 0.0f)
				return -1.0f;
		}
	}

	return tmin;
}

bool plane_3_intersection(const Plane& p1, const Plane& p2, const Plane& p3, Vector3& ip)
{
	const Vector3& n1 = p1.n;
	const Vector3& n2 = p2.n;
	const Vector3& n3 = p3.n;

	float den = -vector3::dot(vector3::cross(n1, n2), n3);

	if (equals(den, (float)0.0))
	{
		return false;
	}

	Vector3 res = p1.d * vector3::cross(n2, n3) + p2.d * vector3::cross(n3, n1) + p3.d * vector3::cross(n1, n2);
	ip = res / den;

	return true;
}

bool frustum_sphere_intersection(const Frustum& f, const Sphere& s)
{
	if (plane::distance_to_point(f.left, s.c) < -s.r ||
		plane::distance_to_point(f.right, s.c) < -s.r)
	{
		return false;
	}

	if (plane::distance_to_point(f.bottom, s.c) < -s.r ||
		plane::distance_to_point(f.top, s.c) < -s.r)
	{
		return false;
	}

	if (plane::distance_to_point(f.near, s.c) < -s.r ||
		plane::distance_to_point(f.far, s.c) < -s.r)
	{
		return false;
	}

	return true;
}

bool frustum_box_intersection(const Frustum& f, const AABB& b)
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

} // namespace crown
