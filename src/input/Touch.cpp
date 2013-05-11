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

#include "Touch.h"

namespace crown
{

//-----------------------------------------------------------------------------
Touch::Touch()
{

}

//-----------------------------------------------------------------------------
bool Touch::touch_up(uint16_t id) const
{
	return m_pointers[id].up == true;
}

//-----------------------------------------------------------------------------
bool Touch::touch_down(uint16_t id) const
{
	return m_pointers[id].up == false;
}

//----------------------------------------------------------------------------- 
Point2 Touch::touch_xy(uint16_t id) const
{
	const PointerData& data = m_pointers[id];

	return Point2(data.x, data.y);
}

//-----------------------------------------------------------------------------
Vec2 Touch::touch_relative_xy(uint16_t id)
{
	const PointerData& data = m_pointers[id];

	return Vec2(data.relative_x, data.relative_y);
}

} // namespace crown
