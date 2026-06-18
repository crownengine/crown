/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/memory/types.h"
#include "core/types.h"
#include "device/types.h"
#include "resource/types.h"
#include "world/types.h"

namespace crown
{
///
/// @ingroup Device
namespace stat_globals
{
	///
	void init(Allocator &a
		, ResourceManager &rm
		, ShaderManager &sm
		, MaterialManager &mm
		, Pipeline &pl
		, ConsoleServer &cs
		, const StatConfigResource *config
		);

	///
	void shutdown();

	/// Draws all enabled statistic panels.
	void draw(u16 window_width, u16 window_height);

	/// Reloads the statistic panel @a config.
	void reload(const StatConfigResource *config);

} // namespace stat_globals

} // namespace crown
