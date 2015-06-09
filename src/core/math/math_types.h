/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"

namespace crown
{

/// @defgroup Math Math

/// @ingroup Math
struct Vector2
{
	float x, y;
};

/// @ingroup Math
struct Vector3
{
	float x, y, z;
};

/// @ingroup Math
struct Vector4
{
	float x, y, z, w;
};

/// @ingroup Math
struct Quaternion
{
	float x, y, z, w;
};

/// @ingroup Math
struct Matrix3x3
{
	Vector3 x, y, z;
};

/// @ingroup Math
struct Matrix4x4
{
	Vector4 x, y, z, t;
};

/// @ingroup Math
struct AABB
{
	Vector3 min;
	Vector3 max;
};

/// @ingroup Math
struct OBB
{
	Matrix4x4 tm;
	AABB aabb;
};

/// 3D Plane.
/// The form is ax + by + cz + d = 0
/// where: d = -vector3::dot(n, p)
///
/// @ingroup Math
struct Plane
{
	Vector3 n;
	float d;
};

/// @ingroup Math
struct Frustum
{
	Plane left;
	Plane right;
	Plane bottom;
	Plane top;
	Plane near;
	Plane far;
};

/// @ingroup Math
struct Sphere
{
	Vector3 c;
	float r;
};

} // namespace crown
