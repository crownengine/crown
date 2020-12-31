/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/math/types.h"
#include "core/memory/types.h"
#include "core/strings/string_id.h"
#include "core/types.h"
#include "device/input_types.h"

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
	u8 _first_button[2];
	const char** _button_name;
	const char** _axis_name;

	u8* _last_state;          // num_buttons
	u8* _state;               // num_buttons
	Vector3* _axis;           // num_axes
	u32* _deadzone_mode;      // num_axes
	f32* _deadzone_size;      // num_axes
	StringId32* _button_hash; // num_buttons
	StringId32* _axis_hash;   // num_axes
	char* _name;              // strlen32(name) + 1

	void set_button(u8 id, u8 state);
	void set_axis(u8 id, f32 x, f32 y, f32 z);
	void update();

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

	/// Returns the id of the first button that was pressed in the current frame
	/// or UINT8_MAX if no buttons were pressed at all.
	u8 any_pressed() const;

	/// Returns the id of the first button that was released in the current frame
	/// or UINT8_MAX if no buttons were released at all.
	u8 any_released() const;

	/// Returns the value of the button @a id in the range [0..1].
	f32 button(u8 id) const;

	/// Returns the value of the axis @a id.
	Vector3 axis(u8 id) const;

	/// Returns the name of the button @a id or NULL if no matching button is found.
	const char* button_name(u8 id);

	/// Returns the name of the axis @a id of NULL if no matching axis is found.
	const char* axis_name(u8 id);

	/// Returns the id of the button @a name or UINT8_MAX if no matching button is found.
	u8 button_id(StringId32 name);

	/// Returns the id of the axis @a name of UINT8_MAX if no matching axis is found.
	u8 axis_id(StringId32 name);

	/// Returns the deadzone size and the deadzone @a mode for the axis @a id.
	f32 deadzone(u8 id, DeadzoneMode::Enum* deadzone_mode);

	/// Sets the @a deadzone_mode and @a deadzone_size for the axis @a id.
	void set_deadzone(u8 id, DeadzoneMode::Enum deadzone_mode, f32 deadzone_size);
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
