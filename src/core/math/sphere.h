/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/math/types.h"

namespace crown
{
/// Functions to manipulate Sphere.
///
/// @ingroup Math
namespace sphere
{
	/// Resets the sphere @a s.
	void reset(Sphere &s);

	/// Returns the volume of the sphere @a s.
	f32 volume(const Sphere &s);

	/// Creates the minimum enclosing sphere of a cone.
	Sphere from_cone(const Vector3 &tip, const Vector3 &dir, f32 range, f32 angle);

	/// Adds @a num @a points to the sphere @a s, expanding its bounds if necessary.
	void add_points(Sphere &s, u32 num, u32 stride, const void *points);

	/// Adds @a num @a points to the sphere expanding if necessary.
	void add_points(Sphere &s, u32 num, const Vector3 *points);

	/// Returns whether point @a p is contained into the sphere.
	bool contains_point(const Sphere &s, const Vector3 &p);

	///
	void transform(Sphere &out, const Sphere &s, const Matrix4x4 &m);

} // namespace sphere

} // namespace crown
