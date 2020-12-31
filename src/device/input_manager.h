/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "config.h"
#include "core/memory/types.h"
#include "core/types.h"
#include "device/input_types.h"
#include "device/types.h"

namespace crown
{
/// Manages input devices.
///
/// @ingroup Input
struct InputManager
{
	Allocator* _allocator;
	InputDevice* _keyboard;
	InputDevice* _mouse;
	InputDevice* _touch;
	InputDevice* _joypad[CROWN_MAX_JOYPADS];
	s16 _mouse_last_x;
	s16 _mouse_last_y;
	bool _has_delta_axis_event;

	/// Constructor.
	InputManager(Allocator& a);

	/// Destructor.
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

	///
	void read(const OsEvent& ev);

	/// Updates the input devices.
	void update();
};

} // namespace crown
