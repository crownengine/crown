/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/memory/types.h"

namespace crown
{
Allocator &default_allocator();
Allocator &default_scratch_allocator();

namespace memory_globals
{
	/// Constructs the initial default allocators.
	/// @note
	/// Has to be called before anything else during the engine startup.
	void init();

	/// Destroys the allocators created with memory_globals::init().
	/// @note
	/// Should be the last call of the program.
	void shutdown();

} // namespace memory_globals

} // namespace crown
