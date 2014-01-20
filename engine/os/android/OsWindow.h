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

#include <sys/types.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>

namespace crown
{

class OsWindow
{
public:

	/// Stub method, does nothing under Android.
					OsWindow();
					~OsWindow();

	/// Stub method, does nothing under Android.
	void			show();

	/// Stub method, does nothing under Android.
	void			hide();

	/// Returns the size in pixel of the window.
	void			get_size(uint32_t& width, uint32_t& height);

	/// Returns always (0, 0) under Android.
	void			get_position(uint32_t& x, uint32_t& y);

	/// Stub method, does nothing under Android.
	void			resize(uint32_t width, uint32_t height);

	/// Stub method, does nothing under Android.
	void			move(uint32_t x, uint32_t y);

	/// Stub method, does nothing under Android.	
	void			minimize();

	/// Stub method, does nothing under Android.
	void			restore();

	/// Returns always false.
	bool			is_resizable() const;

	/// Stub method, does nothing under Android.	
	void			set_resizable(bool resizeable);

	/// Stub method, does nothing under Android.
	void			show_cursor(bool show);

	/// Stub method, does nothing under Android.
	void			get_cursor_xy(int32_t& x, int32_t& y);

	/// Stub method, does nothing under Android.
	void			set_cursor_xy(int32_t x, int32_t y);

	/// Returns always NULL under Android.
	char*			title();

	/// Stub method, does nothing under Android.
	void			set_title(const char* title);

	/// Stub method, does nothing under Android.
	void			frame();

public:

	uint32_t		m_x;
	uint32_t		m_y;
	uint32_t		m_width;
	uint32_t		m_height;
};

} // namespace crown