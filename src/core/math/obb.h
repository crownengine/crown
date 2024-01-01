/*
 * Copyright (c) 2012-2024 Daniele Bartolini et al.
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

} // namespace obb

} // namespace crown
