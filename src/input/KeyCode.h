/*
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

namespace crown
{

const uint16_t MAX_KEYCODES = 256;

/// A KeyCode is the number which uniquely identifies a key
/// on the keyboard.
enum KeyCode
{
	KC_NOKEY		= 0x00,

	KC_TAB			= 0x09,		// ASCII value
	KC_ENTER		= 0x0D,		// ASCII value
	KC_ESCAPE		= 0x1B,		// ASCII value
	KC_SPACE		= 0x20,		// ASCII value
	KC_BACKSPACE	= 0x7F,		// ASCII value

	/* KeyPad */
	KC_KP_0			= 0x80,
	KC_KP_1			= 0x81,
	KC_KP_2			= 0x82,
	KC_KP_3			= 0x83,
	KC_KP_4			= 0x84,
	KC_KP_5			= 0x85,
	KC_KP_6			= 0x86,
	KC_KP_7			= 0x87,
	KC_KP_8			= 0x88,
	KC_KP_9			= 0x89,

	/* Function keys */
	KC_F1			= 0x90,
	KC_F2			= 0x91,
	KC_F3			= 0x92,
	KC_F4			= 0x93,
	KC_F5			= 0x94,
	KC_F6			= 0x95,
	KC_F7			= 0x96,
	KC_F8			= 0x97,
	KC_F9			= 0x98,
	KC_F10			= 0x99,
	KC_F11			= 0x9A,
	KC_F12			= 0x9B,

	/* Other keys */
	KC_HOME			= 0xA0,
	KC_LEFT			= 0xA1,
	KC_UP			= 0xA2,
	KC_RIGHT		= 0xA3,
	KC_DOWN			= 0xA4,
	KC_PAGE_UP		= 0xA5,
	KC_PAGE_DOWN	= 0xA6,

	/* Modifier keys */
	KC_LCONTROL		= 0xB0,
	KC_RCONTROL		= 0xB1,
	KC_LSHIFT		= 0xB2,
	KC_RSHIFT		= 0xB3,
	KC_CAPS_LOCK	= 0xB4,
	KC_LALT			= 0xB5,
	KC_RALT			= 0xB6,
	KC_LSUPER		= 0xB7,
	KC_RSUPER		= 0xB8,

	/* [0x30, 0x39] reserved for ASCII digits */
	/* [0x41, 0x5A] and [0x61, 0x7A] reserved for ASCII alphabet */

	// The last key _must_ be <= 0xFF
	KC_COUNT		= 0xFF
};

typedef uint8_t Key;

} // namespace crown

