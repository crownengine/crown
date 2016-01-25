/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "memory_types.h"
#include "types.h"

namespace crown
{
/// Display mode.
///
/// @ingroup Device
struct DisplayMode
{
	uint32_t id;
	uint32_t width;
	uint32_t height;
};

/// Display interface.
///
/// @ingroup Device
class Display
{
public:

	/// Opens the window.
	virtual void modes(Array<DisplayMode>& modes) = 0;
	virtual void set_mode(uint32_t id) = 0;

	static Display* create(Allocator& a);
	static void destroy(Allocator& a, Display& d);
};

} // namespace crown
