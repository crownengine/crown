/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */
#include "config.h"

#if CROWN_PLATFORM_WINDOWS

#include "os_window_windows.h"
#include "string_utils.h"

namespace crown
{

HWND m_windows_window = 0;

void oswindow_set_window(HWND handle_win)
{
	m_windows_window = handle_win;
}

OsWindow::OsWindow()
	: m_resizable(true)
{
	set_title("");
}

OsWindow::~OsWindow()
{
}

void OsWindow::show()
{
	ShowWindow(m_windows_window, SW_SHOW);
}

void OsWindow::hide()
{
	ShowWindow(m_windows_window, SW_HIDE);
}

void OsWindow::resize(uint32_t width, uint32_t height)
{
	SetWindowPos(m_windows_window, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
}

void OsWindow::move(uint32_t x, uint32_t y)
{
	SetWindowPos(m_windows_window, NULL, x, y, 0, 0, SWP_NOMOVE | SWP_NOZORDER);
}

void OsWindow::minimize()
{
	ShowWindow(m_windows_window, SW_MINIMIZE);
}

void OsWindow::restore()
{
	ShowWindow(m_windows_window, SW_RESTORE);
}

bool OsWindow::is_resizable() const
{
	return m_resizable;
}

void OsWindow::set_resizable(bool resizable)
{
	m_resizable = resizable;
}

char* OsWindow::title()
{
	return m_title;
}

void OsWindow::set_title(const char* title)
{
	strncpy(m_title, title, 32);
	SetWindowText(m_windows_window, m_title);
}

} // namespace crown

#endif // CROWN_PLATFORM_WINDOWS
