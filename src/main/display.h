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
	u32 id;
	u32 width;
	u32 height;
};

/// Display interface.
///
/// @ingroup Device
class Display
{
public:

	/// Opens the window.
	virtual void modes(Array<DisplayMode>& modes) = 0;
	virtual void set_mode(u32 id) = 0;

	static Display* create(Allocator& a);
	static void destroy(Allocator& a, Display& d);
};

} // namespace crown
