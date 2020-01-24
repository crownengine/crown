/*
 * Copyright (c) 2012-2020 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/math/aabb.inl"
#include "core/math/constants.h"
#include "core/math/intersection.h"
#include "core/math/plane3.inl"
#include "core/math/sphere.inl"
#include "core/math/vector3.inl"

namespace crown
{
f32 ray_plane_intersection(const Vector3& from, const Vector3& dir, const Plane3& p)
{
	const f32 num = dot(from, p.n);
	const f32 den = dot(dir, p.n);

	if (fequal(den, 0.0f))
		return -1.0f;

	return (-p.d - num) / den;
}

f32 ray_disc_intersection(const Vector3& from, const Vector3& dir, const Vector3& center, f32 radius, const Vector3& normal)
{
	const Plane3 p = plane3::from_point_and_normal(center, normal);
	const f32 t = ray_plane_intersection(from, dir, p);

	if (t == -1.0f)
		return -1.0f;

	const Vector3 intersection_point = from + dir * t;
	if (distance_squared(intersection_point, center) < radius*radius)
		return t;

	return -1.0f;
}

f32 ray_sphere_intersection(const Vector3& from, const Vector3& dir, const Sphere& s)
{
	const Vector3 v = s.c - from;
	const f32 b   = dot(v, dir);
	const f32 rr  = s.r * s.r;
	const f32 bb  = b * b;
	const f32 det = rr - dot(v, v) + bb;

	if (det < 0.0f || b < s.r)
		return -1.0f;

	return b - fsqrt(det);
}

// http://www.opengl-tutorial.org/miscellaneous/clicking-on-objects/picking-with-custom-ray-obb-function/
f32 ray_obb_intersection(const Vector3& from, const Vector3& dir, const Matrix4x4& tm, const Vector3& half_extents)
{
	f32 tmin = 0.0f;
	f32 tmax = 999999999.9f;

	const Vector3 obb_pos = vector3(tm.t.x, tm.t.y, tm.t.z);
	const Vector3 delta = obb_pos - from;

	{
		const Vector3 xaxis = vector3(tm.x.x, tm.x.y, tm.x.z);
		const f32 e = dot(xaxis, delta);
		const f32 f = dot(dir, xaxis);

		if (fabs(f) > 0.001f)
		{
			f32 t1 = (e-half_extents.x)/f;
			f32 t2 = (e+half_extents.x)/f;

			if (t1 > t2) { f32 w=t1;t1=t2;t2=w; }

			if (t2 < tmax)
				tmax = t2;
			if (t1 > tmin)
				tmin = t1;

			if (tmax < tmin)
				return -1.0f;

		}
		else
		{
			if (-e-half_extents.x > 0.0f || -e+half_extents.x < 0.0f)
				return -1.0f;
		}
	}

	{
		const Vector3 yaxis = vector3(tm.y.x, tm.y.y, tm.y.z);
		const f32 e = dot(yaxis, delta);
		const f32 f = dot(dir, yaxis);

		if (fabs(f) > 0.001f)
		{
			f32 t1 = (e-half_extents.y)/f;
			f32 t2 = (e+half_extents.y)/f;

			if (t1 > t2) { f32 w=t1;t1=t2;t2=w; }

			if (t2 < tmax)
				tmax = t2;
			if (t1 > tmin)
				tmin = t1;

			if (tmin > tmax)
				return -1.0f;
		}
		else
		{
			if (-e-half_extents.y > 0.0f || -e+half_extents.y < 0.0f)
				return -1.0f;
		}
	}

	{
		const Vector3 zaxis = vector3(tm.z.x, tm.z.y, tm.z.z);
		const f32 e = dot(zaxis, delta);
		const f32 f = dot(dir, zaxis);

		if (fabs(f) > 0.001f)
		{
			f32 t1 = (e-half_extents.z)/f;
			f32 t2 = (e+half_extents.z)/f;

			if (t1 > t2) { f32 w=t1;t1=t2;t2=w; }

			if (t2 < tmax)
				tmax = t2;
			if (t1 > tmin)
				tmin = t1;

			if (tmin > tmax)
				return -1.0f;

		}
		else
		{
			if (-e-half_extents.z > 0.0f || -e+half_extents.z < 0.0f)
				return -1.0f;
		}
	}

	return tmin;
}

f32 ray_triangle_intersection(const Vector3& from, const Vector3& dir, const Vector3& v0, const Vector3& v1, const Vector3& v2)
{
	const Vector3 verts[] = { v0, v1, v2 };
	const u16 inds[] = { 0, 1, 2 };
	return ray_mesh_intersection(from, dir, MATRIX4X4_IDENTITY, verts, sizeof(Vector3), inds, 3);
}

f32 ray_mesh_intersection(const Vector3& from, const Vector3& dir, const Matrix4x4& tm, const void* vertices, u32 stride, const u16* indices, u32 num)
{
	bool hit = false;
	f32 tmin = 999999999.9f;

	for (u32 i = 0; i < num; i += 3)
	{
		const u32 i0 = indices[i + 0];
		const u32 i1 = indices[i + 1];
		const u32 i2 = indices[i + 2];

		const Vector3& v0 = *(const Vector3*)((const char*)vertices + i0*stride) * tm;
		const Vector3& v1 = *(const Vector3*)((const char*)vertices + i1*stride) * tm;
		const Vector3& v2 = *(const Vector3*)((const char*)vertices + i2*stride) * tm;

		// https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm

		// Find vectors for two edges sharing v0
		const Vector3 e1 = v1 - v0;
		const Vector3 e2 = v2 - v0;

		// Begin calculating determinant - also used to calculate u parameter
		const Vector3 P = cross(dir, e2);

		// If determinant is near zero, ray lies in plane of triangle
		const f32 det = dot(e1, P);
		if (fequal(det, 0.0f))
			continue;

		const f32 inv_det = 1.0f / det;

		// Distance from v0 to ray origin
		const Vector3 T = from - v0;

		// u parameter and test bound
		const f32 u = dot(T, P) * inv_det;

		// The intersection lies outside of the triangle
		if (u < 0.0f || u > 1.0f)
			continue;

		// Prepare to test v parameter
		const Vector3 Q = cross(T, e1);

		// v parameter and test bound
		const f32 v = dot(dir, Q) * inv_det;

		// The intersection lies outside of the triangle
		if (v < 0.0f || u + v  > 1.0f)
			continue;

		const f32 t = dot(e2, Q) * inv_det;

		// Ray intersection
		if (t > FLOAT_EPSILON)
		{
			hit = true;
			tmin = min(t, tmin);
		}
	}

	return hit ? tmin : -1.0f;
}

bool plane_3_intersection(const Plane3& a, const Plane3& b, const Plane3& c, Vector3& ip)
{
	const Vector3 na = a.n;
	const Vector3 nb = b.n;
	const Vector3 nc = c.n;
	const f32 den    = -dot(cross(na, nb), nc);

	if (fequal(den, 0.0f))
		return false;

	const f32 inv_den = 1.0f / den;

	const Vector3 nbnc = a.d * cross(nb, nc);
	const Vector3 ncna = b.d * cross(nc, na);
	const Vector3 nanb = c.d * cross(na, nb);

	ip = (nbnc + ncna + nanb) * inv_den;

	return true;
}

bool frustum_sphere_intersection(const Frustum& f, const Sphere& s)
{
	if (plane3::distance_to_point(f.plane_left, s.c) < -s.r ||
		plane3::distance_to_point(f.plane_right, s.c) < -s.r)
	{
		return false;
	}

	if (plane3::distance_to_point(f.plane_bottom, s.c) < -s.r ||
		plane3::distance_to_point(f.plane_top, s.c) < -s.r)
	{
		return false;
	}

	if (plane3::distance_to_point(f.plane_near, s.c) < -s.r ||
		plane3::distance_to_point(f.plane_far, s.c) < -s.r)
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

	u8 out = 0;
	out += (plane3::distance_to_point(f.plane_left, v0) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_left, v1) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_left, v2) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_left, v3) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_left, v4) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_left, v5) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_left, v6) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_left, v7) < 0.0f) ? 1 : 0;
	if (out == 8) return false;

	out = 0;
	out += (plane3::distance_to_point(f.plane_right, v0) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_right, v1) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_right, v2) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_right, v3) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_right, v4) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_right, v5) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_right, v6) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_right, v7) < 0.0f) ? 1 : 0;
	if (out == 8) return false;

	out = 0;
	out += (plane3::distance_to_point(f.plane_bottom, v0) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_bottom, v1) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_bottom, v2) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_bottom, v3) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_bottom, v4) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_bottom, v5) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_bottom, v6) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_bottom, v7) < 0.0f) ? 1 : 0;
	if (out == 8) return false;

	out = 0;
	out += (plane3::distance_to_point(f.plane_top, v0) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_top, v1) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_top, v2) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_top, v3) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_top, v4) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_top, v5) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_top, v6) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_top, v7) < 0.0f) ? 1 : 0;
	if (out == 8) return false;

	out = 0;
	out += (plane3::distance_to_point(f.plane_near, v0) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_near, v1) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_near, v2) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_near, v3) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_near, v4) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_near, v5) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_near, v6) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_near, v7) < 0.0f) ? 1 : 0;
	if (out == 8) return false;

	out = 0;
	out += (plane3::distance_to_point(f.plane_far, v0) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_far, v1) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_far, v2) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_far, v3) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_far, v4) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_far, v5) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_far, v6) < 0.0f) ? 1 : 0;
	out += (plane3::distance_to_point(f.plane_far, v7) < 0.0f) ? 1 : 0;
	if (out == 8) return false;

	// If we are here, it is because either the box intersects or it is contained in the frustum
	return true;
}

} // namespace crown
