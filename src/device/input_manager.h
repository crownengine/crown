/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
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
	Allocator *_allocator;
	InputDevice *_keyboard;
	InputDevice *_mouse;
	InputDevice *_touch;
	InputDevice *_joypad[CROWN_MAX_JOYPADS];
	s16 _mouse_last_x;
	s16 _mouse_last_y;
	bool _has_delta_axis_event;
	u32 _num_events;
	InputEvent _events[CROWN_MAX_OS_EVENTS];

	/// Constructor.
	explicit InputManager(Allocator &a);

	/// Destructor.
	~InputManager();

	/// Returns the InputDevice associated with @a type and @a num.
	InputDevice *device_from_type(u16 type, u16 num);

	/// Returns the default keyboard input device.
	InputDevice *keyboard();

	/// Returns the default mouse input device.
	InputDevice *mouse();

	/// Returns the default touch input device.
	InputDevice *touch();

	/// Returns the number of joypads.
	u8 num_joypads();

	/// Returns the joypad @a i.
	InputDevice *joypad(u8 i);

	///
	void read(const OsEvent &ev);

	/// Updates the input devices.
	void update();
};

} // namespace crown
