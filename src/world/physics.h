/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/memory/types.h"

namespace crown
{
/// Global physics settings.
/// @ingroup World
struct PhysicsSettings
{
	s32 step_frequency;
	s32 max_substeps;
};

/// Global physics-related functions
///
/// @ingroup World
namespace physics_globals
{
	/// Initializes the physics system.
	/// This is the place where to create and initialize per-application objects.
	void init(Allocator &linear, Allocator &heap, const PhysicsSettings *settings);

	/// It should reverse the actions performed by physics_globals::init().
	void shutdown(Allocator &linear, Allocator &heap);

} // namespace physics_globals

} // namespace crown
