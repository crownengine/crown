/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

namespace crown
{
/// Global physics-related functions
///
/// @ingroup World
namespace physics_globals
{
	/// Initializes the physics system.
	/// This is the place where to create and initialize per-application objects.
	void init();

	/// It should reverse the actions performed by physics_globals::init().
	void shutdown();
} // namespace physics_globals
} // namespace crown
