/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
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
	void reset(Sphere& s);

	/// Returns the volume of the sphere @a s.
	f32 volume(const Sphere& s);

	/// Adds @a num @a points to the sphere @a s, expanding its bounds if necessary.
	void add_points(Sphere& s, u32 num, u32 stride, const void* points);

	/// Adds @a num @a points to the sphere expanding if necessary.
	void add_points(Sphere& s, u32 num, const Vector3* points);

	/// Adds @a num @a spheres expanding if necessary.
	void add_spheres(Sphere& s, u32 num, const Sphere* spheres);

	/// Returns whether point @a p is contained into the sphere.
	bool contains_point(const Sphere& s, const Vector3& p);

} // namespace sphere

} // namespace crown
