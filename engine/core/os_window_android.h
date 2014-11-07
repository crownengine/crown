/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
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
	void show();

	/// Stub method, does nothing under Android.
	void hide();

	/// Returns the size in pixel of the window.
	void get_size(uint32_t& width, uint32_t& height);

	/// Returns always (0, 0) under Android.
	void get_position(uint32_t& x, uint32_t& y);

	/// Stub method, does nothing under Android.
	void resize(uint32_t width, uint32_t height);

	/// Stub method, does nothing under Android.
	void move(uint32_t x, uint32_t y);

	/// Stub method, does nothing under Android.
	void minimize();

	/// Stub method, does nothing under Android.
	void restore();

	/// Returns always false.
	bool is_resizable() const;

	/// Stub method, does nothing under Android.
	void set_resizable(bool resizeable);

	/// Stub method, does nothing under Android.
	void show_cursor(bool show);

	/// Stub method, does nothing under Android.
	void get_cursor_xy(int32_t& x, int32_t& y);

	/// Stub method, does nothing under Android.
	void set_cursor_xy(int32_t x, int32_t y);

	/// Returns always NULL under Android.
	char* title();

	/// Stub method, does nothing under Android.
	void set_title(const char* title);

	/// Stub method, does nothing under Android.
	void frame();

public:

	uint32_t m_x;
	uint32_t m_y;
	uint32_t m_width;
	uint32_t m_height;
};

} // namespace crown
