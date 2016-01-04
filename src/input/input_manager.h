/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "config.h"
#include "types.h"
#include "input_types.h"

namespace crown
{

/// Manages input devices.
///
/// @ingroup Input
class InputManager
{
public:

	InputManager();
	~InputManager();

	/// Returns the default keyboard input device.
	InputDevice* keyboard();

	/// Returns the default mouse input device.
	InputDevice* mouse();

	/// Returns the default touch input device.
	InputDevice* touch();

	/// Returns the number of joypads.
	uint8_t num_joypads();

	/// Returns the joypad @a i.
	InputDevice* joypad(uint8_t i);

	/// Updates the input devices
	void update();

private:

	InputDevice* _keyboard;
	InputDevice* _mouse;
	InputDevice* _touch;
	InputDevice* _joypad[CROWN_MAX_JOYPADS];
};

} // namespace crown
