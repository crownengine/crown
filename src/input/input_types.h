/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

namespace crown
{

/// @defgroup Input Input

class InputManager;
struct InputDevice;

/// Enumerates keyboard buttons.
///
/// @ingroup Input
struct KeyboardButton
{
	enum Enum
	{
		TAB       = 0x09,
		ENTER     = 0x0d,
		ESCAPE    = 0x1b,
		SPACE     = 0x20,
		BACKSPACE = 0x7f,

		/* KeyPad */
		KP_0      = 0x80,
		KP_1      = 0x81,
		KP_2      = 0x82,
		KP_3      = 0x83,
		KP_4      = 0x84,
		KP_5      = 0x85,
		KP_6      = 0x86,
		KP_7      = 0x87,
		KP_8      = 0x88,
		KP_9      = 0x89,

		/* Function keys */
		F1        = 0x90,
		F2        = 0x91,
		F3        = 0x92,
		F4        = 0x93,
		F5        = 0x94,
		F6        = 0x95,
		F7        = 0x96,
		F8        = 0x97,
		F9        = 0x98,
		F10       = 0x99,
		F11       = 0x9a,
		F12       = 0x9b,

		/* Other keys */
		HOME      = 0xa0,
		LEFT      = 0xa1,
		UP        = 0xa2,
		RIGHT     = 0xa3,
		DOWN      = 0xa4,
		PAGE_UP   = 0xa5,
		PAGE_DOWN = 0xa6,

		/* Modifier keys */
		LCONTROL  = 0xb0,
		RCONTROL  = 0xb1,
		LSHIFT    = 0xb2,
		RSHIFT    = 0xb3,
		CAPS_LOCK = 0xb4,
		LALT      = 0xb5,
		RALT      = 0xb6,
		LSUPER    = 0xb7,
		RSUPER    = 0xb8,

		/* [0x30, 0x39] reserved for ASCII digits */
		NUM_0     = 0x30,
		NUM_1     = 0x31,
		NUM_2     = 0x32,
		NUM_3     = 0x33,
		NUM_4     = 0x34,
		NUM_5     = 0x35,
		NUM_6     = 0x36,
		NUM_7     = 0x37,
		NUM_8     = 0x38,
		NUM_9     = 0x39,

		/* [0x41, 0x5a] reserved for ASCII alphabet */
		A         = 0x41,
		B         = 0x42,
		C         = 0x43,
		D         = 0x44,
		E         = 0x45,
		F         = 0x46,
		G         = 0x47,
		H         = 0x48,
		I         = 0x49,
		J         = 0x4a,
		K         = 0x4b,
		L         = 0x4c,
		M         = 0x4d,
		N         = 0x4e,
		O         = 0x4f,
		P         = 0x50,
		Q         = 0x51,
		R         = 0x52,
		S         = 0x53,
		T         = 0x54,
		U         = 0x55,
		V         = 0x56,
		W         = 0x57,
		X         = 0x58,
		Y         = 0x59,
		Z         = 0x5a,

		/* [0x61, 0x7a] reserved for ASCII alphabet */

		// The last key _must_ be <= 0xff
		COUNT     = 0xff
	};
};

/// Enumerates mouse buttons.
///
/// @ingroup Input
struct MouseButton
{
	enum Enum
	{
		LEFT,
		MIDDLE,
		RIGHT,
		EXTRA_1,
		EXTRA_2,
		COUNT
	};
};

/// Enumerates mouse axes.
///
/// @ingroup Input
struct MouseAxis
{
	enum Enum
	{
		CURSOR,
		WHEEL,
		COUNT
	};
};

/// Enumerates touch panel buttons.
///
/// @ingroup Input
struct TouchButton
{
	enum Enum
	{
		POINTER_0,
		POINTER_1,
		POINTER_2,
		POINTER_3,
		COUNT
	};
};

/// Enumerates joypad buttons.
///
/// @ingroup Input
struct JoypadButton
{
	enum Enum
	{
		UP,
		DOWN,
		LEFT,
		RIGHT,
		START,
		BACK,
		GUIDE,
		LEFT_THUMB,
		RIGHT_THUMB,
		LEFT_SHOULDER,
		RIGHT_SHOULDER,
		A,
		B,
		X,
		Y,
		COUNT
	};
};

/// Enumerates joypad axes.
///
/// @ingroup Input
struct JoypadAxis
{
	enum Enum
	{
		LEFT,
		RIGHT,
		COUNT
	};
};

} // namespace crown
