/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
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
	const Plane p = plane::from_point_and_normal(center, normal);
	const float t = ray_plane_intersection(from, dir, p);

	if (t == -1.0f)
		return -1.0f;

	const Vector3 intersection_point = from + dir * t;
	if (distance_squared(intersection_point, center) < radius*radius)
		return t;

	return -1.0f;
}

float ray_sphere_intersection(const Vector3& from, const Vector3& dir, const Sphere& s)
{
	const Vector3 v = s.c - from;
	const float b   = dot(v, dir);
	const float rr  = s.r * s.r;
	const float bb  = b * b;
	const float det = rr - dot(v, v) + bb;

	if (det < 0.0f || b < s.r)
		return -1.0f;

	return b - sqrtf(det);
}

// http://www.opengl-tutorial.org/miscellaneous/clicking-on-objects/picking-with-custom-ray-obb-function/
float ray_obb_intersection(const Vector3& from, const Vector3& dir, const Matrix4x4& tm, const Vector3& half_extents)
{
	float tmin = 0.0f;
	float tmax = 100000.0f;

	const Vector3 obb_pos = vector3(tm.t.x, tm.t.y, tm.t.z);
	const Vector3 delta = obb_pos - from;

	{
		const Vector3 xaxis = vector3(tm.x.x, tm.x.y, tm.x.z);
		const float e = dot(xaxis, delta);
		const float f = dot(dir, xaxis);

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
		const float e = dot(yaxis, delta);
		const float f = dot(dir, yaxis);

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
		const float e = dot(zaxis, delta);
		const float f = dot(dir, zaxis);

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

bool plane_3_intersection(const Plane& a, const Plane& b, const Plane& c, Vector3& ip)
{
	const Vector3 na = a.n;
	const Vector3 nb = b.n;
	const Vector3 nc = c.n;
	const float den  = -dot(cross(na, nb), nc);

	if (fequal(den, 0.0f))
		return false;

	const float inv_den = 1.0f / den;

	const Vector3 nbnc = a.d * cross(nb, nc);
	const Vector3 ncna = b.d * cross(nc, na);
	const Vector3 nanb = c.d * cross(na, nb);

	ip = (nbnc + ncna + nanb) * inv_den;

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
	const Vector3 v0 = aabb::vertex(b, 0);
	const Vector3 v1 = aabb::vertex(b, 1);
	const Vector3 v2 = aabb::vertex(b, 2);
	const Vector3 v3 = aabb::vertex(b, 3);
	const Vector3 v4 = aabb::vertex(b, 4);
	const Vector3 v5 = aabb::vertex(b, 5);
	const Vector3 v6 = aabb::vertex(b, 6);
	const Vector3 v7 = aabb::vertex(b, 7);

	uint8_t out = 0;
	out += (plane::distance_to_point(f.left, v0) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.left, v1) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.left, v2) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.left, v3) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.left, v4) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.left, v5) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.left, v6) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.left, v7) < 0.0f) ? 1 : 0;
	if (out == 8) return false;

	out = 0;
	out += (plane::distance_to_point(f.right, v0) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.right, v1) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.right, v2) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.right, v3) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.right, v4) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.right, v5) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.right, v6) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.right, v7) < 0.0f) ? 1 : 0;
	if (out == 8) return false;

	out = 0;
	out += (plane::distance_to_point(f.bottom, v0) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.bottom, v1) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.bottom, v2) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.bottom, v3) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.bottom, v4) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.bottom, v5) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.bottom, v6) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.bottom, v7) < 0.0f) ? 1 : 0;
	if (out == 8) return false;

	out = 0;
	out += (plane::distance_to_point(f.top, v0) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.top, v1) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.top, v2) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.top, v3) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.top, v4) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.top, v5) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.top, v6) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.top, v7) < 0.0f) ? 1 : 0;
	if (out == 8) return false;

	out = 0;
	out += (plane::distance_to_point(f.near, v0) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.near, v1) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.near, v2) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.near, v3) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.near, v4) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.near, v5) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.near, v6) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.near, v7) < 0.0f) ? 1 : 0;
	if (out == 8) return false;

	out = 0;
	out += (plane::distance_to_point(f.far, v0) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.far, v1) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.far, v2) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.far, v3) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.far, v4) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.far, v5) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.far, v6) < 0.0f) ? 1 : 0;
	out += (plane::distance_to_point(f.far, v7) < 0.0f) ? 1 : 0;
	if (out == 8) return false;

	// If we are here, it is because either the box intersects or it is contained in the frustum
	return true;
}

} // namespace crown
