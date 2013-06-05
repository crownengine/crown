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

#include "Types.h"
#include "Vec2.h"

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

struct MouseEvent
{
	MouseButton button;
	int32_t x;
	int32_t y;
	float wheel;
};

/// Interface for managing mouse input.
class MouseListener
{

public:

	virtual void button_pressed(const MouseEvent& event) { (void)event; }
	virtual void button_released(const MouseEvent& event) { (void)event; }
	virtual void cursor_moved(const MouseEvent& event) { (void)event; }
};

/// Interface for accessing mouse input device.
class Mouse
{
public:

			Mouse();


	/// Returns whether @a button is pressed.
	bool	button_pressed(MouseButton button) const;

	/// Returns whether @a button is released.
	bool	button_released(MouseButton button) const;

	/// Returns the position of the cursor in window space.
	/// @note
	/// Coordinates in window space have the origin at the
	/// upper-left corner of the window. +X extends from left
	/// to right and +Y extends from top to bottom.
	Vec2	cursor_xy() const;

	/// Sets the position of the cursor in window space.
	/// @note
	/// Coordinates in window space have the origin at the
	/// upper-left corner of the window. +X extends from left
	/// to right and +Y extends from top to bottom.
	void	set_cursor_xy(const Vec2& position);

	/// Returns the relative position of the cursor in window space.
	/// @note
	/// Coordinates in window space have the origin at the
	/// upper-left corner of the window. +X extends from left
	/// to right and +Y extends from top to bottom.
	/// @note
	/// Relative coordinates are mapped to a float varying
	/// from 0.0 to 1.0 where 0.0 is the origin and 1.0 the
	/// maximum extent of the cosidered axis.
	Vec2	cursor_relative_xy() const;

	/// Sets the relative position of the cursor in window space.
	/// @note
	/// Coordinates in window space have the origin at the
	/// upper-left corner of the window. +X extends from left
	/// to right and +Y extends from top to bottom.
	/// @note
	/// Relative coordinates are mapped to a float varying
	/// from 0.0 to 1.0 where 0.0 is the origin and 1.0 the
	/// maximum extent of the cosidered axis.
	void	set_cursor_relative_xy(const Vec2& position);

private:

	// True if correspondig button is pressed, false otherwise.
	bool	m_buttons[MAX_MOUSE_BUTTONS];

	friend class	InputManager;
};

} // namespace crown

