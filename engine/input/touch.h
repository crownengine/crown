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

#include "types.h"
#include "vector2.h"
#include <cstring> // mem*

namespace crown
{

/// Maximum number of pointers supported by Touch.
///
/// @ingroup Input
const uint32_t MAX_POINTER_IDS = 4;

/// Interface for accessing touch panel input device.
///
/// @ingroup Input
struct Touch
{
	//-----------------------------------------------------------------------------
	Touch()
		: _last_pointer(0xFF)
	{
		memset(_last_state, 0, MAX_POINTER_IDS);
		memset(_current_state, 0, MAX_POINTER_IDS);
	}

	/// Returns whether the @a p pointer is pressed in the current frame.
	bool pointer_down(uint8_t p)
	{
		if (p >= MAX_POINTER_IDS) return false;
		return (~_last_state[p] & _current_state[p]) != 0;
	}

	/// Returns whether the @a p pointer is released in the current frame.
	bool pointer_up(uint8_t p)
	{
		if (p >= MAX_POINTER_IDS) return false;
		return (_last_state[p] & ~_current_state[p]) != 0;
	}

	/// Returns wheter any pointer is pressed in the current frame.
	bool any_down()
	{
		return pointer_down(_last_pointer);
	}

	/// Returns whether any pointer is released in the current frame.
	bool any_up()
	{
		return pointer_up(_last_pointer);
	}

	/// Returns the position of the pointer @a p in window space.
	/// @note
	/// Coordinates in window space have the origin at the
	/// upper-left corner of the window. +X extends from left
	/// to right and +Y extends from top to bottom.
	Vector2 pointer_xy(uint8_t p)
	{
		if (p >= MAX_POINTER_IDS) return vector2::ZERO;
		return Vector2(_x[p], _y[p]);
	}

	//-----------------------------------------------------------------------------
	void set_position(uint8_t p, uint16_t x, uint16_t y)
	{
		if (p >= MAX_POINTER_IDS) return;
		_x[p] = x;
		_y[p] = y;
	}

	//-----------------------------------------------------------------------------
	void set_metrics(uint16_t width, uint16_t height)
	{
		_width = width;
		_height = height;
	}

	//-----------------------------------------------------------------------------
	void set_pointer_state(uint16_t x, uint16_t y, uint8_t p, bool state)
	{
		set_position(p, x, y);

		_last_pointer = p;
		_current_state[p] = state;
	}

	//-----------------------------------------------------------------------------
	void update()
	{
		memcpy(_last_state, _current_state, MAX_POINTER_IDS);
	}

public:

	uint8_t _last_pointer;
	uint8_t _last_state[MAX_POINTER_IDS];
	uint8_t _current_state[MAX_POINTER_IDS];

	uint16_t _x[MAX_POINTER_IDS];
	uint16_t _y[MAX_POINTER_IDS];

	// Window size
	uint16_t _width;
	uint16_t _height;
};

} // namespace crown
