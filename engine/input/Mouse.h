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
#undef MB_RIGHT

#include "Types.h"
#include "Vec2.h"
#include "Device.h"
#include "OsWindow.h"

namespace crown
{

const uint32_t MAX_MOUSE_BUTTONS = 3;

/// Enumerates mouse buttons.
enum MouseButton
{
	MB_LEFT		= 0,
	MB_MIDDLE	= 1,
	MB_RIGHT	= 2
};

/// Interface for accessing mouse input device.
class Mouse
{
public:

	//-----------------------------------------------------------------------------
	Mouse()
		: m_current_frame(0), m_last_button(MB_LEFT)
	{
		m_buttons[MB_LEFT] = ~0;
		m_buttons[MB_MIDDLE] = ~0;
		m_buttons[MB_RIGHT] = ~0;

		m_state[MB_LEFT] = false;
		m_state[MB_MIDDLE] = false;
		m_state[MB_RIGHT] = false;
	}

	/// Returns whether @a button is pressed in the current frame.
	bool button_pressed(MouseButton button) const
	{
		CE_ASSERT(button >= 0 && button < MAX_MOUSE_BUTTONS, "MouseButton out of range: %d", button);

		return (m_state[button] == true) && (m_buttons[button] == m_current_frame);
	}

	/// Returns whether @a button is released in the current frame.
	bool button_released(MouseButton button) const
	{
		CE_ASSERT(button >= 0 && button < MAX_MOUSE_BUTTONS, "MouseButton out of range: %d", button);

		return (m_state[button] == false) && (m_buttons[button] == m_current_frame);
	}

	/// Returns wheter any button is pressed in the current frame.
	bool any_pressed() const
	{
		return button_pressed(m_last_button);
	}

	/// Returns whether any button is released in the current frame.
	bool any_released() const
	{
		return button_released(m_last_button);
	}

	/// Returns the position of the cursor in window space.
	/// @note
	/// Coordinates in window space have the origin at the
	/// upper-left corner of the window. +X extends from left
	/// to right and +Y extends from top to bottom.
	Vec2 cursor_xy() const
	{
		// int32_t x, y;

		// device()->window()->get_cursor_xy(x, y);

		// return Vec2(x, y);
		return Vec2::ZERO;
	}

	/// Sets the position of the cursor in window space.
	/// @note
	/// Coordinates in window space have the origin at the
	/// upper-left corner of the window. +X extends from left
	/// to right and +Y extends from top to bottom.
	void set_cursor_xy(const Vec2& position)
	{
		// device()->window()->set_cursor_xy((int32_t) position.x, (int32_t) position.y);
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
	Vec2 cursor_relative_xy() const
	{
		// uint32_t window_width;
		// uint32_t window_height;

		// device()->window()->get_size(window_width, window_height);

		// Vec2 pos = cursor_xy();

		// pos.x = pos.x / (float) window_width;
		// pos.y = pos.y / (float) window_height;

		// return pos;

		return Vec2::ZERO;
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
		// uint32_t window_width;
		// uint32_t window_height;

		// device()->window()->get_size(window_width, window_height);

		// set_cursor_xy(Vec2(position.x * (float) window_width, position.y * (float) window_height));
	}

	//-----------------------------------------------------------------------------
	void update(uint64_t frame, MouseButton b, bool state)
	{
		CE_ASSERT(b >= 0 && b < MAX_MOUSE_BUTTONS, "MouseButton out of range: %d", b);

		m_last_button = b;
		m_buttons[b] = frame;
		m_state[b] = state;
	}

public:

	// The current frame number
	uint64_t		m_current_frame;

	// Last button updated
	MouseButton		m_last_button;

	uint64_t		m_buttons[MAX_MOUSE_BUTTONS];
	bool			m_state[MAX_MOUSE_BUTTONS];

	friend class	Device;
};

} // namespace crown

