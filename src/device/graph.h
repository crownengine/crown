/*
 * Copyright (c) 2012-2025 Daniele Bartolini and individual contributors.
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
#if CROWN_DEBUG
	///
	void init(Allocator &a, ShaderManager &sm, ConsoleServer &cs);

	///
	void shutdown();

	/// Draws all the graphs.
	void draw_all(u16 window_width, u16 window_height);
#else
	inline void init(Allocator &a, ShaderManager &sm, ConsoleServer &cs)
	{
		CE_UNUSED_3(a, sm, cs);
		CE_NOOP();
	}

	inline void shutdown()
	{
		CE_NOOP();
	}

	inline void draw_all(u16 window_width, u16 window_height)
	{
		CE_UNUSED_2(window_width, window_height);
		CE_NOOP();
	}
#endif // if CROWN_DEBUG
} // namespce graph_globals

} // namespace crown
