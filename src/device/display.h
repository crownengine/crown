/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/memory/types.h"
#include "core/types.h"

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
struct Display
{
	///
	virtual ~Display() {};

	/// Fills @a modes with all available display modes.
	virtual void modes(Array<DisplayMode>& modes) = 0;

	/// Sets the mode @a id.
	///
	/// @note
	/// The initial display mode is automatically reset when the program terminates.
	virtual void set_mode(u32 id) = 0;
};

/// Functions to manipulate Display.
///
/// @ingroup Device
namespace display
{
	/// Creates a new display.
	Display* create(Allocator& a);

	/// Destroys the display @a d.
	void destroy(Allocator& a, Display& d);

} // namespace display

} // namespace crown
