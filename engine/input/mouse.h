/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "vector2.h"
#include <cstring> // mem*

namespace crown
{

/// Enumerates mouse buttons.
///
/// @ingroup Input
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
///
/// @ingroup Input
struct Mouse
{
	Mouse()
		: _last_button(MouseButton::NONE)
		, _wheel(0.0f)
	{
		memset(_last_state, 0, MouseButton::COUNT);
		memset(_current_state, 0, MouseButton::COUNT);
	}

	/// Returns whether the @a b button is pressed in the current frame.
	bool button_pressed(MouseButton::Enum b)
	{
		return (~_last_state[b] & _current_state[b]) != 0;
	}

	/// Returns whether the @a b button is released in the current frame.
	bool button_released(MouseButton::Enum b)
	{
		return (_last_state[b] & ~_current_state[b]) != 0;
	}

	/// Returns wheter any button is pressed in the current frame.
	bool any_pressed()
	{
		return button_pressed(_last_button);
	}

	/// Returns whether any button is released in the current frame.
	bool any_released()
	{
		return button_released(_last_button);
	}

	/// Returns the position of the cursor in window space.
	/// @note
	/// Coordinates in window space have the origin at the
	/// upper-left corner of the window. +X extends from left
	/// to right and +Y extends from top to bottom.
	Vector2 cursor_xy()
	{
		return Vector2(_x, _y);
	}

	/// Sets the position of the cursor in window space.
	/// @note
	/// Coordinates in window space have the origin at the
	/// upper-left corner of the window. +X extends from left
	/// to right and +Y extends from top to bottom.
	void set_cursor_xy(const Vector2& position)
	{
		_x = (uint16_t) position.x;
		_y = (uint16_t) position.y;
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
	Vector2 cursor_relative_xy()
	{
		return Vector2((float) _x / _width, (float) _y / _height);
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
	void set_cursor_relative_xy(const Vector2& position)
	{
		set_cursor_xy(Vector2(position.x * (float) _width, position.y * (float) _height));
	}

	/// Returns the mouse wheel state in the current frame.
	/// A positive or negative value is returned when the wheel is up or down
	/// respectively, 0.0 otherwise.
	float wheel()
	{
		return _wheel;
	}

	void set_position(uint16_t x, uint16_t y)
	{
		_x = x;
		_y = y;
	}

	void set_metrics(uint16_t width, uint16_t height)
	{
		_width = width;
		_height = height;
	}

	void set_button_state(uint16_t x, uint16_t y, MouseButton::Enum b, bool state)
	{
		set_position(x, y);
		_last_button = b;
		_current_state[b] = state;
	}

	void set_wheel(uint16_t x, uint16_t y, float wheel)
	{
		set_position(x, y);
		_wheel = wheel;
	}

	void update()
	{
		_wheel = 0.0f;
		memcpy(_last_state, _current_state, MouseButton::COUNT);
	}

public:

	MouseButton::Enum _last_button;
	float _wheel;
	uint8_t _last_state[MouseButton::COUNT];
	uint8_t _current_state[MouseButton::COUNT];

	// Position within the window
	uint16_t _x;
	uint16_t _y;

	// Window size
	uint16_t _width;
	uint16_t _height;
};

} // namespace crown

