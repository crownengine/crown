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
	bool _connected;
	u8 _num_buttons;
	u8 _num_axes;
	u8 _last_button;

	u8* _last_state;           // num_buttons
	u8* _state;                // num_buttons
	Vector3* _axis;            // num_axes
	const char** _button_name; // num_buttons
	const char** _axis_name;   // num_axes
	StringId32* _button_hash;  // num_buttons
	StringId32* _axis_hash;    // num_axes
	char* _name;               // strlen32(name) + 1

	/// Returns the name of the input device.
	const char* name() const;

	/// Returns whether the input device is connected and functioning.
	bool connected() const;

	/// Returns the number of buttons of the input device.
	u8 num_buttons() const;

	/// Returns the number of axes of the input devices.
	u8 num_axes() const;

	/// Returns whether the button @a id is pressed in the current frame.
	bool pressed(u8 id) const;

	/// Returns whether the button @a id is released in the current frame.
	bool released(u8 id) const;

	/// Returns whether any button is pressed in the current frame.
	bool any_pressed() const;

	/// Returns whether any button is released in the current frame.
	bool any_released() const;

	/// Returns the value of the axis @a id.
	Vector3 axis(u8 id) const;

	/// Returns the name of the button @a id.
	const char* button_name(u8 id);

	/// Returns the name of the axis @a id.
	const char* axis_name(u8 id);

	/// Returns the id of the button @a name.
	u8 button_id(StringId32 name);

	/// Returns the id of the axis @a name.
	u8 axis_id(StringId32 name);

	void set_connected(bool connected);

	void set_button_state(u8 i, bool state);

	void set_axis(u8 i, const Vector3& value);

	void update();
};

/// Functions to manipulate InputDevice.
///
/// @ingroup Input
namespace input_device
{
	/// Creates a new input device.
	InputDevice* create(Allocator& a, const char* name, u8 num_buttons, u8 num_axes, const char** button_names, const char** axis_names);

	/// Destroys the input device @a id.
	void destroy(Allocator& a, InputDevice& id);
} // namespace input_device

} // namespace crown
