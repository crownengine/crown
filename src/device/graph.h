/*
 * Copyright (c) 2012-2026 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "config.h"
#include "core/memory/types.h"
#include "device/types.h"
#include "world/types.h"

namespace crown
{
///
/// @ingroup Device
namespace graph_globals
{
	///
	void init(Allocator &a, Pipeline &pl, ConsoleServer &cs);

	///
	void shutdown();

	/// Draws all the graphs.
	void draw_all(u16 window_width, u16 window_height);
} // namespce graph_globals

} // namespace crown
