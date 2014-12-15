/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */
#include "config.h"

#if CROWN_PLATFORM_LINUX

#include "os_window_linux.h"
#include "ceassert.h"
#include "string_utils.h"
#include "log.h"

namespace crown
{

Display* m_x11_display = NULL;
Window m_x11_window = None;

void oswindow_set_window(Display* dpy, Window win)
{
	m_x11_display = dpy;
	m_x11_window = win;
}

OsWindow::OsWindow()
	: m_x(0)
	, m_y(0)
	, m_width(0)
	, m_height(0)
	, m_resizable(true)
{
}

OsWindow::~OsWindow()
{
}

void OsWindow::show()
{
	XMapRaised(m_x11_display, m_x11_window);
}

void OsWindow::hide()
{
	XUnmapWindow(m_x11_display, m_x11_window);
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

void OsWindow::resize(uint32_t width, uint32_t height)
{
	XResizeWindow(m_x11_display, m_x11_window, width, height);
}

void OsWindow::move(uint32_t x, uint32_t y)
{
	XMoveWindow(m_x11_display, m_x11_window, x, y);
}

void OsWindow::minimize()
{
	XIconifyWindow(m_x11_display, m_x11_window, DefaultScreen(m_x11_display));
}

void OsWindow::restore()
{
	XMapRaised(m_x11_display, m_x11_window);
}

bool OsWindow::is_resizable() const
{
	return m_resizable;
}

void OsWindow::set_resizable(bool resizable)
{
	XSizeHints hints;
	hints.flags = PMinSize | PMaxSize;
	hints.min_width = resizable ? 1 : m_width;
	hints.min_height = resizable ? 1 : m_height;
	hints.max_width = resizable ? 65535 : m_width;
	hints.max_height = resizable ? 65535 : m_height;

	XSetWMNormalHints(m_x11_display, m_x11_window, &hints);

	m_resizable = resizable;
}

char* OsWindow::title()
{
	static char title[1024];

	char* tmp_title;
	XFetchName(m_x11_display, m_x11_window, &tmp_title);

	strncpy(title, tmp_title, 1024);
	XFree(tmp_title);

	return title;
}

void OsWindow::set_title(const char* title)
{
	XStoreName(m_x11_display, m_x11_window, title);
}

} // namespace crown

#endif // CROWN_PLATFORM_LINUX
