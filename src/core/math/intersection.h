/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "math_types.h"

namespace crown
{

/// Returns the distance along ray (from, dir) to intersection point with plane @a p
/// or -1.0 if no intersection.
float ray_plane_intersection(const Vector3& from, const Vector3& dir, const Plane& p);

/// Returns the distance along ray (from, dir) to intersection point with sphere @a s
/// or -1.0 if no intersection.
float ray_sphere_intersection(const Vector3& from, const Vector3& dir, const Sphere& s);

/// Returns the distance along ray (from, dir) to intersection point with the oriented
/// bounding-box represented by @a tm and @a half_extents.
/// If no intersection is found, it returns -1.0.
float ray_obb_intersection(const Vector3& from, const Vector3& dir, const Matrix4x4& tm, const Vector3& half_extents);

bool plane_3_intersection(const Plane& p1, const Plane& p2, const Plane& p3, Vector3& ip);
bool frustum_sphere_intersection(const Frustum& f, const Sphere& s);
bool frustum_box_intersection(const Frustum& f, const AABB& b);

} // namespace crown
