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
enum ModifierKey
{
	MK_SHIFT	= 1,
	MK_CTRL		= 2,
	MK_ALT		= 4
};

/// Interface for accessing keyboard input device.
struct Keyboard
{
	Keyboard()
		: m_modifier(0), m_current_frame(0), m_last_key(KC_NOKEY)
	{
		for (uint32_t i = 0; i < MAX_KEYCODES; i++)
		{
			m_keys[i] = ~0;
			m_state[i] = false;
		}
	}

	/// Returns whether the specified @a modifier is pressed.
	/// @note
	/// A modifier is a special key that modifies the normal action
	/// of another key when the two are pressed in combination. (Thanks wikipedia.)
	/// @note
	/// Crown currently supports three different modifier keys: Shift, Ctrl and Alt.
	bool modifier_pressed(ModifierKey modifier) const
	{
		return (m_modifier & modifier) == modifier;
	}

	/// Returns whether the specified @a key is pressed in the current frame.
	bool key_pressed(KeyCode key) const
	{
		CE_ASSERT(key >= 0 && key < MAX_KEYCODES, "KeyCode out of range: %d", key);

		return (m_state[key] == true);
	}

	/// Returns whether the specified @a key is released in the current frame.
	bool key_released(KeyCode key) const
	{
		CE_ASSERT(key >= 0 && key < MAX_KEYCODES, "KeyCode out of range: %d", key);

		return (m_state[key] == false) && (m_keys[key] == m_current_frame);
	}

	/// Returns wheter any key is pressed in the current frame.
	bool any_pressed() const
	{
		return key_pressed(m_last_key);
	}

	/// Returns whether any key is released in the current frame.
	bool any_released() const
	{
		return key_pressed(m_last_key);
	}

	void update(uint64_t frame, KeyCode k, bool state)
	{
		CE_ASSERT(k >= 0 && k < MAX_KEYCODES, "KeyCode out of range: %d", k);

		m_last_key = k;
		m_keys[k] = frame;
		m_state[k] = state;
	}

public:

	uint8_t			m_modifier;

	// The current frame number
	uint64_t		m_current_frame;

	// Last key updated
	KeyCode			m_last_key;
	uint64_t		m_keys[MAX_KEYCODES];
	bool			m_state[MAX_KEYCODES];

	friend class	Device;
};

} // namespace crown
