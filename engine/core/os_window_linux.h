/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>

namespace crown
{

void oswindow_set_window(Display* dpy, Window win);

struct OsWindow
{
	OsWindow();
	~OsWindow();

	void show();
	void hide();

	void get_size(uint32_t& width, uint32_t& height);
	void get_position(uint32_t& x, uint32_t& y);

	void resize(uint32_t width, uint32_t height);
	void move(uint32_t x, uint32_t y);

	void minimize();
	void restore();

	bool is_resizable() const;
	void set_resizable(bool resizable);

	void show_cursor(bool show);

	void get_cursor_xy(int32_t& x, int32_t& y);
	void set_cursor_xy(int32_t x, int32_t y);

	char* title();
	void set_title(const char* title);

public:

	uint32_t m_x;
	uint32_t m_y;
	uint32_t m_width;
	uint32_t m_height;
	bool m_resizable;
};

} // namespace crown
