/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/memory/types.h"

namespace crown
{
/// Global physics-related functions
///
/// @ingroup World
namespace physics_globals
{
	/// Initializes the physics system.
	/// This is the place where to create and initialize per-application objects.
	void init(Allocator& a);

	/// It should reverse the actions performed by physics_globals::init().
	void shutdown(Allocator& a);

} // namespace physics_globals

} // namespace crown
