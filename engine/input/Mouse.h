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

#include <cstring>
#include "Types.h"
#include "Vec2.h"

namespace crown
{

/// Enumerates mouse buttons.
struct MouseButton
{
	enum Enum
	{
		NONE,
		LEFT,
		MIDDLE,
		RIGHT,
		COUNT
	};
};

/// Interface for accessing mouse input device.
class Mouse
{
public:

	//-----------------------------------------------------------------------------
	Mouse()
		: m_last_button(MouseButton::NONE)
	{
		memset(m_last_state, 0, MouseButton::COUNT);
		memset(m_current_state, 0, MouseButton::COUNT);
	}

	/// Returns whether the @a b button is pressed in the current frame.
	bool button_pressed(MouseButton::Enum b)
	{
		return bool(~m_last_state[b] & m_current_state[b]);
	}

	/// Returns whether the @a b button is released in the current frame.
	bool button_released(MouseButton::Enum b)
	{
		return bool(m_last_state[b] & ~m_current_state[b]);
	}

	/// Returns wheter any button is pressed in the current frame.
	bool any_pressed()
	{
		return button_pressed(m_last_button);
	}

	/// Returns whether any button is released in the current frame.
	bool any_released()
	{
		return button_released(m_last_button);
	}

	/// Returns the position of the cursor in window space.
	/// @note
	/// Coordinates in window space have the origin at the
	/// upper-left corner of the window. +X extends from left
	/// to right and +Y extends from top to bottom.
	Vec2 cursor_xy()
	{
		return Vec2(m_x, m_y);
	}

	/// Sets the position of the cursor in window space.
	/// @note
	/// Coordinates in window space have the origin at the
	/// upper-left corner of the window. +X extends from left
	/// to right and +Y extends from top to bottom.
	void set_cursor_xy(const Vec2& position)
	{
		m_x = (uint16_t) position.x;
		m_y = (uint16_t) position.y;
	}

	/// Returns the relative position of the cursor in window space.
	/// @note
	/// Coordinates in window space have the origin at the
	/// upper-left corner of the window. +X extends from left
	/// to right and +Y extends from top to bottom.
	/// @note
	/// Relative coordinates are mapped to a float varying
	/// from 0.0 to 1.0 where 0.0 is the origin and 1.0 the
	/// maximum extent of the cosidered axis.
	Vec2 cursor_relative_xy()
	{
		return Vec2(m_x / m_width, m_y / m_height);
	}

	/// Sets the relative position of the cursor in window space.
	/// @note
	/// Coordinates in window space have the origin at the
	/// upper-left corner of the window. +X extends from left
	/// to right and +Y extends from top to bottom.
	/// @note
	/// Relative coordinates are mapped to a float varying
	/// from 0.0 to 1.0 where 0.0 is the origin and 1.0 the
	/// maximum extent of the cosidered axis.
	void set_cursor_relative_xy(const Vec2& position)
	{
		set_cursor_xy(Vec2(position.x * (float) m_width, position.y * (float) m_height));
	}

	//-----------------------------------------------------------------------------
	void set_position(uint16_t x, uint16_t y)
	{
		m_x = x;
		m_y = y;
	}

	//-----------------------------------------------------------------------------
	void set_metrics(uint16_t width, uint16_t height)
	{
		m_width = width;
		m_height = height;
	}

	//-----------------------------------------------------------------------------
	void set_button_state(uint16_t x, uint16_t y, MouseButton::Enum b, bool state)
	{
		set_position(x, y);

		m_last_button = b;
		m_current_state[b] = state;
	}

	//-----------------------------------------------------------------------------
	void update()
	{
		memcpy(m_last_state, m_current_state, MouseButton::COUNT);
	}

public:

	MouseButton::Enum m_last_button;
	uint8_t m_last_state[MouseButton::COUNT];
	uint8_t m_current_state[MouseButton::COUNT];

	// Position within the window
	uint16_t m_x;
	uint16_t m_y;

	// Window size
	uint16_t m_width;
	uint16_t m_height;
};

} // namespace crown

