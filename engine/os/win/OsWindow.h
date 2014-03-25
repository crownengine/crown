/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#include "Types.h"
#include "Macros.h"

namespace crown
{

CE_EXPORT void oswindow_set_window(HWND handle_win);

class OsWindow
{
public:

					OsWindow();
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

	char*			title();
	void			set_title(const char* title);

private:

	char 			m_title[32];

	uint32_t		m_x;
	uint32_t		m_y;
	uint32_t		m_width;
	uint32_t		m_height;
	bool			m_resizable;
};

} // namespace crown