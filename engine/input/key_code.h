/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

namespace crown
{

/// Enumerates keyboard modifier buttons.
///
/// @ingroup Input
struct ModifierButton
{
	enum Enum
	{
		SHIFT = 1,
		CTRL  = 2,
		ALT   = 4
	};
};

/// Enumerates keyboard buttons.
///
/// @ingroup Input
struct KeyboardButton
{
	enum Enum
	{
		NONE      = 0x00,

		TAB       = 0x09,
		ENTER     = 0x0D,
		ESCAPE    = 0x1B,
		SPACE     = 0x20,
		BACKSPACE = 0x7F,

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
		F11       = 0x9A,
		F12       = 0x9B,

		/* Other keys */
		HOME      = 0xA0,
		LEFT      = 0xA1,
		UP        = 0xA2,
		RIGHT     = 0xA3,
		DOWN      = 0xA4,
		PAGE_UP   = 0xA5,
		PAGE_DOWN = 0xA6,

		/* Modifier keys */
		LCONTROL  = 0xB0,
		RCONTROL  = 0xB1,
		LSHIFT    = 0xB2,
		RSHIFT    = 0xB3,
		CAPS_LOCK = 0xB4,
		LALT      = 0xB5,
		RALT      = 0xB6,
		LSUPER    = 0xB7,
		RSUPER    = 0xB8,

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

		/* [0x41, 0x5A] reserved for ASCII alphabet */
		A         = 0x41,
		B         = 0x42,
		C         = 0x43,
		D         = 0x44,
		E         = 0x45,
		F         = 0x46,
		G         = 0x47,
		H         = 0x48,
		I         = 0x49,
		J         = 0x4A,
		K         = 0x4B,
		L         = 0x4C,
		M         = 0x4D,
		N         = 0x4E,
		O         = 0x4F,
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
		Z         = 0x5A,

		/* [0x61, 0x7A] reserved for ASCII alphabet */

		// The last key _must_ be <= 0xFF
		COUNT     = 0xFF
	};
};

} // namespace crown

