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

#include "Types.h"

namespace crown
{

/// Represents an event fired by mouse.
struct OsMouseEvent
{
	MouseButton::Enum button;
	uint32_t x;
	uint32_t y;
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
	uint32_t pointer_id;
	uint32_t x;
	uint32_t y;
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

		// Exit from program
		EXIT
	};

	OsEvent::Enum type;
	union
	{
		OsMouseEvent mouse;
		OsKeyboardEvent keyboard;
		OsTouchEvent touch;
		OsAccelerometerEvent accelerometer;
	};
};

} // namespace crown
