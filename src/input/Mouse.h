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

#pragma once

#include "Types.h"

namespace crown
{

class InputManager;

/**
	Enumerates mouse buttons.
*/
enum MouseButton
{
	MB_LEFT		= 0,
	MB_MIDDLE	= 1,
	MB_RIGHT	= 2
};

struct MouseEvent
{
	MouseButton button;
	int32_t x;
	int32_t y;
	float wheel;
};

/**
	Interface for managing mouse input
*/
class MouseListener
{

public:

	virtual void button_pressed(const MouseEvent& event) { (void)event; }
	virtual void button_released(const MouseEvent& event) { (void)event; }
	virtual void cursor_moved(const MouseEvent& event) { (void)event; }
};

} // namespace crown

