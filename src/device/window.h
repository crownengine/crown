/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "memory_types.h"
#include "types.h"

namespace crown
{
/// Window interface.
///
/// @ingroup Device
class Window
{
public:

	/// Opens the window.
	virtual void open(u16 x, u16 y, u16 width, u16 height, u32 parent) = 0;

	/// Closes the window.
	virtual void close() = 0;

	/// Shows the window.
	virtual void show() = 0;

	/// Hides the window.
	virtual void hide() = 0;

	/// Resizes the window to @a width and @a height.
	virtual void resize(u16 width, u16 height) = 0;

	/// Moves the window to @a x and @a y.
	virtual void move(u16 x, u16 y) = 0;

	/// Minimizes the window.
	virtual void minimize() = 0;

	/// Restores the window.
	virtual void restore() = 0;

	/// Returns the title of the window.
	virtual const char* title() = 0;

	/// Sets the title of the window.
	virtual void set_title (const char* title) = 0;

	/// Returns the native window handle.
	virtual void* handle() = 0;

	virtual void bgfx_setup() = 0;

	static Window* create(Allocator& a);
	static void destroy(Allocator& a, Window& w);
};

} // namespace crown
