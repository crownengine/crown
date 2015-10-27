/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "math_types.h"

namespace crown
{

struct InputDevice
{
	/// Returns the name of the input device.
	const char* name() const;

	/// Returns the number of buttons of the input device.
	uint8_t num_buttons() const;

	/// Returns the number of axes of the input devices.
	uint8_t num_axes() const;

	/// Returns whether the specified @a b button is pressed in the current frame.
	bool pressed(uint8_t i) const;

	/// Returns whether the specified @a b button is released in the current frame.
	bool released(uint8_t i) const;

	/// Returns wheter any button is pressed in the current frame.
	bool any_pressed() const;

	/// Returns whether any button is released in the current frame.
	bool any_released() const;

	/// Returns the value of the axis @a i.
	Vector3 axis(uint8_t i) const;

	void set_button_state(uint8_t i, bool state);

	void set_axis(uint8_t i, const Vector3& value);

	void update();

	uint8_t _num_buttons;
	uint8_t _num_axes;
	uint8_t _last_button;

	uint8_t* _last_state;    // num_buttons
	uint8_t* _current_state; // num_buttons
	Vector3* _axis;          // num_axes
	char* _name;             // strlen(name) + 1
};

} // namespace crown
