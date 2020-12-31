/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/math/types.h"

namespace crown
{
/// Functions to manipulate AABB.
///
/// @ingroup Math
namespace aabb
{
	/// Resets the AABB @a b.
	void reset(AABB& b);

	/// Creates the AABB @a a from @a num @a points.
	void from_points(AABB& a, u32 num, u32 stride, const void* points);

	/// Creates the AABB @a a from @a num @a points.
	void from_points(AABB& b, u32 num, const Vector3* points);

	/// Creates the AABB @a a from @a num @a boxes.
	void from_boxes(AABB& b, u32 num, const AABB* boxes);

	/// Returns the center of the box @a b.
	Vector3 center(const AABB& b);

	/// Returns the radius of the box @a b.
	f32 radius(const AABB& b);

	/// Returns the volume of the box @a b.
	f32 volume(const AABB& b);

	/// Returns whether point @a p is contained in the box @a b.
	bool contains_point(const AABB& b, const Vector3& p);

	/// Returns the @a index -th vertex of the box.
	Vector3 vertex(const AABB& b, u32 index);

	/// Returns the box enclosing @a b transformed by @a m.
	AABB transformed(const AABB& b, const Matrix4x4& m);

	/// Returns the eight vertices of the box @a b.
	void to_vertices(const AABB& b, Vector3 v[8]);

	/// Returns the sphere enclosing the box @a b.
	Sphere to_sphere(const AABB& b);

} // namespace aabb

} // namespace crown
