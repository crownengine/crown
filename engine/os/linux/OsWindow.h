/*
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>

#include "Types.h"

namespace crown
{

class OsWindow
{
public:

	/// Creates the window with the given @a width and @a height.
	/// When @a parent is != 0, it is interpreted as the OS-specific
	/// handle of the parent window.
					OsWindow(uint32_t width, uint32_t height, uint32_t parent);
					~OsWindow();

	void			show();
	void			hide();

	void			get_size(uint32_t& width, uint32_t& height);
	void			get_position(uint32_t& x, uint32_t& y);

	void			resize(uint32_t width, uint32_t height);
	void			move(uint32_t x, uint32_t y);

	void			minimize();
	void			restore();

	bool			is_resizable() const;
	void			set_resizable(bool resizable);

	void			show_cursor(bool show);

	void			get_cursor_xy(int32_t& x, int32_t& y);
	void			set_cursor_xy(int32_t x, int32_t y);

	char*			title();
	void			set_title(const char* title);

private:



	uint32_t		m_x;
	uint32_t		m_y;
	uint32_t		m_width;
	uint32_t		m_height;
	bool			m_resizable;

	bool			m_x11_detectable_autorepeat;

};

} // namespace crown
