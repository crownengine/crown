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

#include "OsWindow.h"
#include "GLContext.h"
#include "Assert.h"
#include "Log.h"

namespace crown
{

//-----------------------------------------------------------------------------
OsWindow::OsWindow()
	: m_x(0)
	, m_y(0)
	, m_width(0)
	, m_height(0)
{
}

//-----------------------------------------------------------------------------
OsWindow::~OsWindow()
{
}

//-----------------------------------------------------------------------------
void OsWindow::show()
{
}

//-----------------------------------------------------------------------------
void OsWindow::hide()
{
}

//-----------------------------------------------------------------------------
void OsWindow::get_size(uint32_t& width, uint32_t& height)
{
	width = m_width;
	height = m_height;
}

//-----------------------------------------------------------------------------
void OsWindow::get_position(uint32_t& x, uint32_t& y)
{
	x = m_x;
	y = m_y;
}

//-----------------------------------------------------------------------------
void OsWindow::resize(uint32_t /*width*/, uint32_t /*height*/)
{
}

//-----------------------------------------------------------------------------
void OsWindow::move(uint32_t /*x*/, uint32_t /*y*/)
{
}

//-----------------------------------------------------------------------------
void OsWindow::minimize()
{
}

//-----------------------------------------------------------------------------
void OsWindow::restore()
{
}

//-----------------------------------------------------------------------------
bool OsWindow::is_resizable() const
{
	return false;
}

//-----------------------------------------------------------------------------
void OsWindow::set_resizable(bool /*resizeable*/)
{
}

//-----------------------------------------------------------------------------
void OsWindow::show_cursor(bool /*show*/)
{
}

//-----------------------------------------------------------------------------
void OsWindow::get_cursor_xy(int32_t& /*x*/, int32_t& /*y*/)
{
}

//-----------------------------------------------------------------------------
void OsWindow::set_cursor_xy(int32_t /*x*/, int32_t /*y*/)
{
}

//-----------------------------------------------------------------------------
char* OsWindow::title()
{
	return NULL;
}

//-----------------------------------------------------------------------------
void OsWindow::set_title(const char* /*title*/)
{
}

//-----------------------------------------------------------------------------
void OsWindow::frame()
{
}

} // namespace crown