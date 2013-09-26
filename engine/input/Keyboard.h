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
#include "KeyCode.h"
#include "Assert.h"

#undef MK_SHIFT
#undef MK_ALT

namespace crown
{

/// Enumerates modifier keys.
struct ModifierButton
{
	enum Enum
	{
		SHIFT	= 1,
		CTRL	= 2,
		ALT		= 4
	};
};

/// Interface for accessing keyboard input device.
struct Keyboard
{
	Keyboard()
		: m_modifier(0), m_any_pressed(false)
	{
		for (uint32_t i = 0; i < KeyboardButton::COUNT; i++)
		{
			m_last_state[i] = false;
			m_current_state[i] = false;
		}
	}

	/// Returns whether the specified @a modifier is pressed.
	/// @note
	/// A modifier is a special key that modifies the normal action
	/// of another key when the two are pressed in combination. (Thanks wikipedia.)
	/// @note
	/// Crown currently supports three different modifier keys: Shift, Ctrl and Alt.
	bool modifier_pressed(ModifierButton::Enum modifier)
	{
		return (m_modifier & (uint8_t) modifier) == modifier;
	}

	/// Returns whether the specified @a key is pressed in the current frame.
	bool button_pressed(KeyboardButton::Enum b)
	{
		bool pressed = (m_current_state[b] == true && m_last_state[b] == false);

		return pressed;
	}

	/// Returns whether the specified @a key is released in the current frame.
	bool button_released(KeyboardButton::Enum b)
	{
		bool released = (m_current_state[b] == false && m_last_state[b] == true);

		return released;
	}

	/// Returns wheter any key is pressed in the current frame.
	bool any_pressed()
	{
		return m_any_pressed;
	}

	/// Returns whether any key is released in the current frame.
	bool any_released()
	{
		return false;
	}

	void set_button_state(KeyboardButton::Enum b, bool state)
	{
		m_last_state[b] = m_current_state[b];
		m_current_state[b] = state;
	}

public:

	uint8_t m_modifier;

	// Last key updated
	bool m_any_pressed;
	bool m_last_state[KeyboardButton::COUNT];
	bool m_current_state[KeyboardButton::COUNT];
};

} // namespace crown
