/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/math/types.h"

/// @defgroup Math Math
/// @ingroup Core
namespace crown
{
/// @addtogroup Math
/// @{

static const Vector2 VECTOR2_ZERO  = { 0.0f, 0.0f };
static const Vector2 VECTOR2_ONE   = { 1.0f, 1.0f };
static const Vector2 VECTOR2_XAXIS = { 1.0f, 0.0f };
static const Vector2 VECTOR2_YAXIS = { 0.0f, 1.0f };

static const Vector3 VECTOR3_ZERO     = {  0.0f,  0.0f,  0.0f };
static const Vector3 VECTOR3_ONE      = {  1.0f,  1.0f,  1.0f };
static const Vector3 VECTOR3_XAXIS    = {  1.0f,  0.0f,  0.0f };
static const Vector3 VECTOR3_YAXIS    = {  0.0f,  1.0f,  0.0f };
static const Vector3 VECTOR3_ZAXIS    = {  0.0f,  0.0f,  1.0f };
static const Vector3 VECTOR3_RIGHT    = {  1.0f,  0.0f,  0.0f };
static const Vector3 VECTOR3_LEFT     = { -1.0f,  0.0f,  0.0f };
static const Vector3 VECTOR3_UP       = {  0.0f,  1.0f,  0.0f };
static const Vector3 VECTOR3_DOWN     = {  0.0f, -1.0f,  0.0f };
static const Vector3 VECTOR3_FORWARD  = {  0.0f,  0.0f,  1.0f };
static const Vector3 VECTOR3_BACKWARD = {  0.0f,  0.0f, -1.0f };

static const Vector4 VECTOR4_ZERO  = { 0.0f, 0.0f, 0.0f, 0.0f };
static const Vector4 VECTOR4_ONE   = { 1.0f, 1.0f, 1.0f, 1.0f };
static const Vector4 VECTOR4_XAXIS = { 1.0f, 0.0f, 0.0f, 0.0f };
static const Vector4 VECTOR4_YAXIS = { 0.0f, 1.0f, 0.0f, 0.0f };
static const Vector4 VECTOR4_ZAXIS = { 0.0f, 0.0f, 1.0f, 0.0f };
static const Vector4 VECTOR4_WAXIS = { 0.0f, 0.0f, 0.0f, 1.0f };

static const Color4 COLOR4_BLACK  = { 0.0f, 0.0f, 0.0f, 1.0f };
static const Color4 COLOR4_WHITE  = { 1.0f, 1.0f, 1.0f, 1.0f };
static const Color4 COLOR4_RED    = { 1.0f, 0.0f, 0.0f, 1.0f };
static const Color4 COLOR4_GREEN  = { 0.0f, 1.0f, 0.0f, 1.0f };
static const Color4 COLOR4_BLUE   = { 0.0f, 0.0f, 1.0f, 1.0f };
static const Color4 COLOR4_YELLOW = { 1.0f, 1.0f, 0.0f, 1.0f };
static const Color4 COLOR4_ORANGE = { 1.0f, 0.5f, 0.0f, 1.0f };

static const Quaternion QUATERNION_IDENTITY = { 0.0f, 0.0f, 0.0f, 1.0f };

static const Matrix3x3 MATRIX3X3_IDENTITY = { VECTOR3_XAXIS, VECTOR3_YAXIS, VECTOR3_ZAXIS };

static const Matrix4x4 MATRIX4X4_IDENTITY = { VECTOR4_XAXIS, VECTOR4_YAXIS, VECTOR4_ZAXIS, VECTOR4_WAXIS };

static const Plane3 PLANE3_ZERO  = { VECTOR3_ZERO,  0.0f };
static const Plane3 PLANE3_XAXIS = { VECTOR3_XAXIS, 0.0f };
static const Plane3 PLANE3_YAXIS = { VECTOR3_YAXIS, 0.0f };
static const Plane3 PLANE3_ZAXIS = { VECTOR3_ZAXIS, 0.0f };
/// @}

} // namespace crown
