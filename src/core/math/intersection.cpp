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

float ray_plane_intersection(const Vector3& from, const Vector3& dir, const Plane& p)
{
	const float num = dot(from, p.n);
	const float den = dot(dir, p.n);

	if (fequal(den, 0.0f))
		return -1.0f;

	return (-p.d - num) / den;
}

float ray_disc_intersection(const Vector3& from, const Vector3& dir, const Vector3& center, float radius, const Vector3& normal)
{
	Plane p = plane::from_point_and_normal(center, normal);
	const float t = ray_plane_intersection(from, dir, p);

	if (t == -1.0)
		return -1.0;

	const Vector3 intersection_point = from + dir * t;
	if (distance(intersection_point, center) < radius)
		return t;

	return -1.0;
}

float ray_sphere_intersection(const Vector3& from, const Vector3& dir, const Sphere& s)
{
	const Vector3 v = s.c - from;
	const float b   = dot(v, dir);
	const float det = (s.r * s.r) - dot(v, v) + (b * b);

	if (det < 0.0 || b < s.r)
		return -1.0f;

	return b - sqrtf(det);
}

// http://www.opengl-tutorial.org/miscellaneous/clicking-on-objects/picking-with-custom-ray-obb-function/
float ray_obb_intersection(const Vector3& from, const Vector3& dir, const Matrix4x4& tm, const Vector3& half_extents)
{
	float tmin = 0.0f;
	float tmax = 100000.0f;

	Vector3 obb_pos = vector3(tm.t.x, tm.t.y, tm.t.z);
	Vector3 delta = obb_pos - from;

	{
		const Vector3 xaxis = vector3(tm.x.x, tm.x.y, tm.x.z);
		float e = dot(xaxis, delta);
		float f = dot(dir, xaxis);

		if (fabs(f) > 0.001f)
		{
			float t1 = (e-half_extents.x)/f;
			float t2 = (e+half_extents.x)/f;

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
			if(-e-half_extents.x > 0.0f || -e+half_extents.x < 0.0f)
				return -1.0f;
		}
	}

	{
		const Vector3 yaxis = vector3(tm.y.x, tm.y.y, tm.y.z);
		float e = dot(yaxis, delta);
		float f = dot(dir, yaxis);

		if (fabs(f) > 0.001f){

			float t1 = (e-half_extents.y)/f;
			float t2 = (e+half_extents.y)/f;

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
			if(-e-half_extents.y > 0.0f || -e+half_extents.y < 0.0f)
				return -1.0f;
		}
	}

	{
		const Vector3 zaxis = vector3(tm.z.x, tm.z.y, tm.z.z);
		float e = dot(zaxis, delta);
		float f = dot(dir, zaxis);

		if (fabs(f) > 0.001f){

			float t1 = (e-half_extents.z)/f;
			float t2 = (e+half_extents.z)/f;

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
			if(-e-half_extents.z > 0.0f || -e+half_extents.z < 0.0f)
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
	const float den = -dot(cross(n1, n2), n3);

	if (fequal(den, 0.0f))
		return false;

	const float inv_den = 1.0f / den;

	Vector3 res = p1.d * cross(n2, n3) + p2.d * cross(n3, n1) + p3.d * cross(n1, n2);
	ip = res * inv_den;

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
