/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "math_types.h"

namespace crown
{
/// @addtogroup Math
/// @{

/// Returns the distance along ray (from, dir) to intersection point with plane @a p
/// or -1.0 if no intersection.
f32 ray_plane_intersection(const Vector3& from, const Vector3& dir, const Plane3& p);

/// Returns the distance along ray (from, dir) to intersection point with disc defined by
/// @a center, @a radius and @a normal or -1.0 if no intersection.
f32 ray_disc_intersection(const Vector3& from, const Vector3& dir, const Vector3& center, f32 radius, const Vector3& normal);

/// Returns the distance along ray (from, dir) to intersection point with sphere @a s
/// or -1.0 if no intersection.
f32 ray_sphere_intersection(const Vector3& from, const Vector3& dir, const Sphere& s);

/// Returns the distance along ray (from, dir) to intersection point with the oriented
/// bounding box (tm, half_extents) or -1.0 if no intersection.
f32 ray_obb_intersection(const Vector3& from, const Vector3& dir, const Matrix4x4& tm, const Vector3& half_extents);

/// Returns the distance along ray (from, dir) to intersection point with the triangle
/// (v0, v1, v2) or -1.0 if no intersection.
f32 ray_triangle_intersection(const Vector3& from, const Vector3& dir, const Vector3& v0, const Vector3& v1, const Vector3& v2);

/// Returns the distance along ray (from, dir) to intersection point with the triangle
/// mesh defined by (vertices, stride, indices, num) or -1.0 if no intersection.
f32 ray_mesh_intersection(const Vector3& from, const Vector3& dir, const Matrix4x4& tm, const void* vertices, u32 stride, const u16* indices, u32 num);

/// Returns whether the planes @a a, @a b and @a c intersects and if so fills @a ip with the intersection point.
bool plane_3_intersection(const Plane3& a, const Plane3& b, const Plane3& c, Vector3& ip);

/// Returns whether the frustum @a f and the sphere @a s intersects.
bool frustum_sphere_intersection(const Frustum& f, const Sphere& s);

/// Returns whether the frustum @a f and the AABB @a b intersects.
bool frustum_box_intersection(const Frustum& f, const AABB& b);

/// @}
} // namespace crown
