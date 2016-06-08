/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"

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
/// where: d = -vector3::dot(n, p)
struct Plane3
{
	Vector3 n;
	f32 d;
};

struct Frustum
{
	Plane3 left;
	Plane3 right;
	Plane3 bottom;
	Plane3 top;
	Plane3 near;
	Plane3 far;
};

struct Sphere
{
	Vector3 c;
	f32 r;
};

const Vector2 VECTOR2_ZERO  = { 0.0f, 0.0f };
const Vector2 VECTOR2_ONE   = { 1.0f, 1.0f };
const Vector2 VECTOR2_XAXIS = { 1.0f, 0.0f };
const Vector2 VECTOR2_YAXIS = { 0.0f, 1.0f };

const Vector3 VECTOR3_ZERO     = {  0.0f,  0.0f,  0.0f };
const Vector3 VECTOR3_ONE      = {  1.0f,  1.0f,  1.0f };
const Vector3 VECTOR3_XAXIS    = {  1.0f,  0.0f,  0.0f };
const Vector3 VECTOR3_YAXIS    = {  0.0f,  1.0f,  0.0f };
const Vector3 VECTOR3_ZAXIS    = {  0.0f,  0.0f,  1.0f };
const Vector3 VECTOR3_RIGHT    = {  1.0f,  0.0f,  0.0f };
const Vector3 VECTOR3_LEFT     = { -1.0f,  0.0f,  0.0f };
const Vector3 VECTOR3_UP       = {  0.0f,  1.0f,  0.0f };
const Vector3 VECTOR3_DOWN     = {  0.0f, -1.0f,  0.0f };
const Vector3 VECTOR3_FORWARD  = {  0.0f,  0.0f,  1.0f };
const Vector3 VECTOR3_BACKWARD = {  0.0f,  0.0f, -1.0f };

const Vector4 VECTOR4_ZERO  = { 0.0f, 0.0f, 0.0f, 0.0f };
const Vector4 VECTOR4_ONE   = { 1.0f, 1.0f, 1.0f, 1.0f };
const Vector4 VECTOR4_XAXIS = { 1.0f, 0.0f, 0.0f, 0.0f };
const Vector4 VECTOR4_YAXIS = { 0.0f, 1.0f, 0.0f, 0.0f };
const Vector4 VECTOR4_ZAXIS = { 0.0f, 0.0f, 1.0f, 0.0f };
const Vector4 VECTOR4_WAXIS = { 0.0f, 0.0f, 0.0f, 1.0f };

const Color4 COLOR4_BLACK  = { 0.0f, 0.0f, 0.0f, 1.0f };
const Color4 COLOR4_WHITE  = { 1.0f, 1.0f, 1.0f, 1.0f };
const Color4 COLOR4_RED    = { 1.0f, 0.0f, 0.0f, 1.0f };
const Color4 COLOR4_GREEN  = { 0.0f, 1.0f, 0.0f, 1.0f };
const Color4 COLOR4_BLUE   = { 0.0f, 0.0f, 1.0f, 1.0f };
const Color4 COLOR4_YELLOW = { 1.0f, 1.0f, 0.0f, 1.0f };
const Color4 COLOR4_ORANGE = { 1.0f, 0.5f, 0.0f, 1.0f };

const Quaternion QUATERNION_IDENTITY = { 0.0f, 0.0f, 0.0f, 1.0f };

const Matrix3x3 MATRIX3X3_IDENTITY = { VECTOR3_XAXIS, VECTOR3_YAXIS, VECTOR3_ZAXIS };

const Matrix4x4 MATRIX4X4_IDENTITY = { VECTOR4_XAXIS, VECTOR4_YAXIS, VECTOR4_ZAXIS, VECTOR4_WAXIS };

const Plane3 PLANE3_ZERO  = { VECTOR3_ZERO,  0.0f };
const Plane3 PLANE3_XAXIS = { VECTOR3_XAXIS, 0.0f };
const Plane3 PLANE3_YAXIS = { VECTOR3_YAXIS, 0.0f };
const Plane3 PLANE3_ZAXIS = { VECTOR3_ZAXIS, 0.0f };
/// @}

} // namespace crown
