/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "config.h"

#if CROWN_PLATFORM_ANDROID

#include "os_window_android.h"
#include "assert.h"
#include "log.h"

namespace crown
{

OsWindow::OsWindow()
	: m_x(0)
	, m_y(0)
	, m_width(0)
	, m_height(0)
{
}

OsWindow::~OsWindow()
{
}

void OsWindow::show()
{
}

void OsWindow::hide()
{
}

void OsWindow::get_size(uint32_t& width, uint32_t& height)
{
	width = m_width;
	height = m_height;
}

void OsWindow::get_position(uint32_t& x, uint32_t& y)
{
	x = m_x;
	y = m_y;
}

void OsWindow::resize(uint32_t /*width*/, uint32_t /*height*/)
{
}

void OsWindow::move(uint32_t /*x*/, uint32_t /*y*/)
{
}

void OsWindow::minimize()
{
}

void OsWindow::restore()
{
}

bool OsWindow::is_resizable() const
{
	return false;
}

void OsWindow::set_resizable(bool /*resizeable*/)
{
}

void OsWindow::show_cursor(bool /*show*/)
{
}

void OsWindow::get_cursor_xy(int32_t& /*x*/, int32_t& /*y*/)
{
}

void OsWindow::set_cursor_xy(int32_t /*x*/, int32_t /*y*/)
{
}

char* OsWindow::title()
{
	return NULL;
}

void OsWindow::set_title(const char* /*title*/)
{
}

void OsWindow::frame()
{
}

} // namespace crown

#endif // CROWN_PLATFORM_ANDROID
