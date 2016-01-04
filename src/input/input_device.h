/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "math_types.h"
#include "memory_types.h"
#include "string_id.h"

namespace crown
{

/// Represents a generic input device.
///
/// @ingroup Input
struct InputDevice
{
	/// Returns the name of the input device.
	const char* name() const;

	/// Returns whether the input device is connected and functioning.
	bool connected() const;

	/// Returns the number of buttons of the input device.
	uint8_t num_buttons() const;

	/// Returns the number of axes of the input devices.
	uint8_t num_axes() const;

	/// Returns whether the button @a id is pressed in the current frame.
	bool pressed(uint8_t id) const;

	/// Returns whether the button @a id is released in the current frame.
	bool released(uint8_t id) const;

	/// Returns whether any button is pressed in the current frame.
	bool any_pressed() const;

	/// Returns whether any button is released in the current frame.
	bool any_released() const;

	/// Returns the value of the axis @a id.
	Vector3 axis(uint8_t id) const;

	/// Returns the name of the button @a id.
	const char* button_name(uint8_t id);

	/// Returns the name of the axis @a id.
	const char* axis_name(uint8_t id);

	/// Returns the id of the button @a name.
	uint8_t button_id(StringId32 name);

	/// Returns the id of the axis @a name.
	uint8_t axis_id(StringId32 name);

	void set_connected(bool connected);

	void set_button_state(uint8_t i, bool state);

	void set_axis(uint8_t i, const Vector3& value);

	void update();

public:

	bool _connected;
	uint8_t _num_buttons;
	uint8_t _num_axes;
	uint8_t _last_button;

	uint8_t* _last_state;      // num_buttons
	uint8_t* _current_state;   // num_buttons
	Vector3* _axis;            // num_axes
	const char** _button_name; // num_buttons
	const char** _axis_name;   // num_axes
	StringId32* _button_hash;  // num_buttons
	StringId32* _axis_hash;    // num_axes
	char* _name;               // strlen32(name) + 1

public:

	static InputDevice* create(Allocator& a, const char* name, uint8_t num_buttons, uint8_t num_axes, const char** button_names, const char** axis_names);
	static void destroy(Allocator& a, InputDevice* id);
};

} // namespace crown
