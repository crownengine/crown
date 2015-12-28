/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

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

/// Holds RGBA color as four floats.
typedef Vector4 Color4;

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

const Vector2 VECTOR2_ZERO = { 0.0f, 0.0f };

const Vector3 VECTOR3_ZERO     = {  0.0f,  0.0f,  0.0f };
const Vector3 VECTOR3_XAXIS    = {  1.0f,  0.0f,  0.0f };
const Vector3 VECTOR3_YAXIS    = {  0.0f,  1.0f,  0.0f };
const Vector3 VECTOR3_ZAXIS    = {  0.0f,  0.0f,  1.0f };
const Vector3 VECTOR3_FORWARD  = {  0.0f,  0.0f,  1.0f };
const Vector3 VECTOR3_BACKWARD = {  0.0f,  0.0f, -1.0f };
const Vector3 VECTOR3_LEFT     = { -1.0f,  0.0f,  0.0f };
const Vector3 VECTOR3_RIGHT    = {  1.0f,  0.0f,  0.0f };
const Vector3 VECTOR3_UP       = {  0.0f,  1.0f,  0.0f };
const Vector3 VECTOR3_DOWN     = {  0.0f, -1.0f,  0.0f };

const Vector4 VECTOR4_ZERO  = { 0.0f, 0.0f, 0.0f, 0.0f };
const Vector4 VECTOR4_XAXIS = { 1.0f, 0.0f, 0.0f, 0.0f };
const Vector4 VECTOR4_YAXIS = { 0.0f, 1.0f, 0.0f, 0.0f };
const Vector4 VECTOR4_ZAXIS = { 0.0f, 0.0f, 1.0f, 0.0f };
const Vector4 VECTOR4_WAXIS = { 0.0f, 0.0f, 0.0f, 1.0f };

const Quaternion QUATERNION_IDENTITY = { 0.0f, 0.0f, 0.0f, 1.0f };

const Matrix3x3 MATRIX3X3_IDENTITY = { VECTOR3_XAXIS, VECTOR3_YAXIS, VECTOR3_ZAXIS };

const Matrix4x4 MATRIX4X4_IDENTITY = { VECTOR4_XAXIS, VECTOR4_YAXIS, VECTOR4_ZAXIS, VECTOR4_WAXIS };

const Plane PLANE_ZERO  = { VECTOR3_ZERO,  0.0f };
const Plane PLANE_XAXIS = { VECTOR3_XAXIS, 0.0f };
const Plane PLANE_YAXIS = { VECTOR3_YAXIS, 0.0f };
const Plane PLANE_ZAXIS = { VECTOR3_ZAXIS, 0.0f };

} // namespace crown
