/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "types.h"
#include "mouse.h"
#include "keyboard.h"

namespace crown
{

struct OsMetricsEvent
{
	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t height;
};

struct OsExitEvent
{
	int32_t code;
};

/// Represents an event fired by mouse.
struct OsMouseEvent
{
	enum Enum
	{
		BUTTON,
		MOVE
	};

	OsMouseEvent::Enum type;
	MouseButton::Enum button;
	uint16_t x;
	uint16_t y;
	bool pressed;
};

/// Represents an event fired by keyboard.
struct OsKeyboardEvent
{
	KeyboardButton::Enum button;
	uint32_t modifier;
	bool pressed;
};

/// Represents an event fired by touch screen.
struct OsTouchEvent
{
	enum Enum
	{
		POINTER,
		MOVE
	};

	OsTouchEvent::Enum type;
	uint8_t pointer_id;
	uint16_t x;
	uint16_t y;
	bool pressed;
};

/// Represents an event fired by accelerometer.
struct OsAccelerometerEvent
{
	float x;
	float y;
	float z;
};

struct OsEvent
{
	/// Represents an event fired by the OS
	enum Enum
	{
		NONE			= 0,

		KEYBOARD		= 1,
		MOUSE			= 2,
		TOUCH			= 3,
		ACCELEROMETER	= 4,

		METRICS,
		PAUSE,
		RESUME,
		// Exit from program
		EXIT
	};

	OsEvent::Enum type;
	union
	{
		OsMetricsEvent metrics;
		OsExitEvent exit;
		OsMouseEvent mouse;
		OsKeyboardEvent keyboard;
		OsTouchEvent touch;
		OsAccelerometerEvent accelerometer;
	};
};

} // namespace crown
