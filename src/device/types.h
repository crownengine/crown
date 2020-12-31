/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
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
	u16 type;
	u16 device_id  : 3;
	u16 device_num : 2;
	u16 button_num : 8;
	u16 pressed    : 1;
};

struct AxisEvent
{
	u16 type;
	u16 device_id  : 3;
	u16 device_num : 2;
	u16 axis_num   : 4;
	s16 axis_x;
	s16 axis_y;
	s16 axis_z;
};

struct StatusEvent
{
	u16 type;
	u16 device_id  : 3;
	u16 device_num : 2;
	u16 connected  : 1;
};

struct ResolutionEvent
{
	u16 type;
	u16 width;
	u16 height;
};

struct TextEvent
{
	u16 type;
	u8 len;
	u8 utf8[4];
};

union OsEvent
{
	u16 type;
	ButtonEvent button;
	AxisEvent axis;
	StatusEvent status;
	ResolutionEvent resolution;
	TextEvent text;
};

} // namespace crown
