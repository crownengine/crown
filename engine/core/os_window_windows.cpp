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

#include "config.h"

#if CROWN_PLATFORM_WINDOWS

#include "os_window.h"
#include "string_utils.h"

namespace crown
{

HWND m_windows_window = 0;

void oswindow_set_window(HWND handle_win)
{
	m_windows_window = handle_win;
}

//-----------------------------------------------------------------------------
OsWindow::OsWindow()
	: m_resizable(true)
{
	set_title("");
}

//-----------------------------------------------------------------------------
OsWindow::~OsWindow()
{
}

//-----------------------------------------------------------------------------
void OsWindow::show()
{
	ShowWindow(m_windows_window, SW_SHOW);
}

//-----------------------------------------------------------------------------
void OsWindow::hide()
{
	ShowWindow(m_windows_window, SW_HIDE);
}

//-----------------------------------------------------------------------------
void OsWindow::resize(uint32_t width, uint32_t height)
{
	SetWindowPos(m_windows_window, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
}

//-----------------------------------------------------------------------------
void OsWindow::move(uint32_t x, uint32_t y)
{
	SetWindowPos(m_windows_window, NULL, x, y, 0, 0, SWP_NOMOVE | SWP_NOZORDER);
}

//-----------------------------------------------------------------------------
void OsWindow::minimize()
{
	ShowWindow(m_windows_window, SW_MINIMIZE);
}

//-----------------------------------------------------------------------------
void OsWindow::restore()
{
	ShowWindow(m_windows_window, SW_RESTORE);
}

//-----------------------------------------------------------------------------
bool OsWindow::is_resizable() const
{
	return m_resizable;
}

//-----------------------------------------------------------------------------
void OsWindow::set_resizable(bool resizable)
{
	m_resizable = resizable;
}

//-----------------------------------------------------------------------------
char* OsWindow::title()
{
	return m_title;
}

//-----------------------------------------------------------------------------
void OsWindow::set_title(const char* title)
{
	string::strncpy(m_title, title, 32);
	SetWindowText(m_windows_window, m_title);
}

} // namespace crown

#endif // CROWN_PLATFORM_WINDOWS
