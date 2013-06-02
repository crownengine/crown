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

#include "Mouse.h"
#include "Device.h"
#include "OsWindow.h"

namespace crown
{

//-----------------------------------------------------------------------------
Mouse::Mouse()
{
	m_buttons[MB_LEFT] = false;
	m_buttons[MB_MIDDLE] = false;
	m_buttons[MB_RIGHT] = false;
}

//-----------------------------------------------------------------------------
bool Mouse::button_pressed(MouseButton button) const
{
	return m_buttons[button] == true;
}

//-----------------------------------------------------------------------------
bool Mouse::button_released(MouseButton button) const
{
	return m_buttons[button] == false;
}

//-----------------------------------------------------------------------------
Vec2 Mouse::cursor_xy() const
{
	int32_t x, y;

	device()->window()->get_cursor_xy(x, y);

	return Vec2(x, y);
}

//-----------------------------------------------------------------------------
void Mouse::set_cursor_xy(const Vec2& position)
{
	device()->window()->set_cursor_xy((int32_t) position.x, (int32_t) position.y);
}

//-----------------------------------------------------------------------------
Vec2 Mouse::cursor_relative_xy() const
{
	uint32_t window_width;
	uint32_t window_height;

	device()->window()->get_size(window_width, window_height);

	Vec2 pos = cursor_xy();

	pos.x = pos.x / (float) window_width;
	pos.y = pos.y / (float) window_height;

	return pos;
}

//-----------------------------------------------------------------------------
void Mouse::set_cursor_relative_xy(const Vec2& position)
{
	uint32_t window_width;
	uint32_t window_height;

	device()->window()->get_size(window_width, window_height);

	set_cursor_xy(Vec2(position.x * (float) window_width, position.y * (float) window_height));
}

} // namespace crown