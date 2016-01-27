/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "config.h"
#include "types.h"
#include "input_types.h"
#include "memory_types.h"

namespace crown
{
/// Manages input devices.
///
/// @ingroup Input
class InputManager
{
	Allocator* _allocator;
	InputDevice* _keyboard;
	InputDevice* _mouse;
	InputDevice* _touch;
	InputDevice* _joypad[CROWN_MAX_JOYPADS];

public:

	InputManager(Allocator& a);
	~InputManager();

	/// Returns the default keyboard input device.
	InputDevice* keyboard();

	/// Returns the default mouse input device.
	InputDevice* mouse();

	/// Returns the default touch input device.
	InputDevice* touch();

	/// Returns the number of joypads.
	u8 num_joypads();

	/// Returns the joypad @a i.
	InputDevice* joypad(u8 i);

	/// Updates the input devices
	void update();
};

} // namespace crown
