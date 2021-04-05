/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/types.h"

/// @defgroup Math Math
/// @ingroup Core
namespace crown
{
/// @addtogroup Math
/// @{

struct Vector2
{
	f32 x, y;
};

struct Vector3
{
	f32 x, y, z;
};

struct Vector4
{
	f32 x, y, z, w;
};

/// RGBA color.
typedef Vector4 Color4;

struct Quaternion
{
	f32 x, y, z, w;
};

struct Matrix3x3
{
	Vector3 x, y, z;
};

struct Matrix4x4
{
	Vector4 x, y, z, t;
};

struct AABB
{
	Vector3 min;
	Vector3 max;
};

struct OBB
{
	Matrix4x4 tm;
	Vector3 half_extents;
};

/// 3D Plane.
/// The form is ax + by + cz + d = 0
/// where: d = vector3::dot(n, p)
struct Plane3
{
	Vector3 n;
	f32 d;
};

struct Frustum
{
	Plane3 planes[6];
};

struct Sphere
{
	Vector3 c;
	f32 r;
};

/// @}

} // namespace crown
