/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/types.h"

namespace crown
{
struct ConsoleServer;
struct Device;
struct Pipeline;
struct Profiler;

struct OsEventType
{
	enum Enum
	{
		BUTTON,
		AXIS,
		STATUS,
		RESOLUTION,
		EXIT,
		PAUSE,
		RESUME,
		TEXT
	};
};

struct ButtonEvent
{
	u16 type       : 4;
	u16 device_id  : 3;
	u16 device_num : 2;
	u16 button_num : 8;
	u16 pressed    : 1;
};

struct AxisEvent
{
	u16 type       : 4;
	u16 device_id  : 3;
	u16 device_num : 2;
	u16 axis_num   : 4;
	s16 axis_x;
	s16 axis_y;
	s16 axis_z;
};

struct StatusEvent
{
	u16 type       : 4;
	u16 device_id  : 3;
	u16 device_num : 2;
	u16 connected  : 1;
};

struct ResolutionEvent
{
	u16 type : 4;
	u16 width;
	u16 height;
};

struct TextEvent
{
	u16 type : 4;
	u8 len;
	u8 utf8[4];
};

union OsEvent
{
	u16 type : 4;
	ButtonEvent button;
	AxisEvent axis;
	StatusEvent status;
	ResolutionEvent resolution;
	TextEvent text;
};

struct TimestepPolicy
{
	enum Enum
	{
		VARIABLE,
		SMOOTHED,

		COUNT
	};
};

} // namespace crown
