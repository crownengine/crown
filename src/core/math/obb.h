/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/math/types.h"

namespace crown
{
/// Functions to manipulate Oriented Bounding Box (OBB).
///
/// @ingroup Math
namespace obb
{
	/// Resets the OBB @a obb.
	void reset(OBB &obb);

	/// Returns the center of the box @a obb.
	Vector3 center(const OBB &obb);

	/// Returns the eight vertices of the box @a obb.
	void to_vertices(Vector3 vertices[8], const OBB &obb);

	/// Returns a new box which encloses both the box @a a and @a b.
	/// @note The returned box will have the same orientation as @a a and won't necessarily be the
	/// smallest enclosing box.
	OBB merge(const OBB &a, const OBB &b);

} // namespace obb

} // namespace crown
