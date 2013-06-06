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
#include "OS.h"

namespace crown
{

class InputManager;


/// Enumerates modifier keys.
enum ModifierKey
{
	MK_SHIFT	= 1,
	MK_CTRL		= 2,
	MK_ALT		= 4
};

struct KeyboardEvent
{
	Key			key;
	uint8_t		modifier;
	char		text[4];
};

class KeyboardListener
{

public:

	virtual void key_pressed(const KeyboardEvent& event) { (void)event; }
	virtual void key_released(const KeyboardEvent& event) { (void)event; }
	virtual void text_input(const KeyboardEvent& event) { (void)event; }
};

/// Interface for accessing keyboard input device.
class Keyboard
{
public:

					Keyboard();

	/// Returns whether the specified @a modifier is pressed.
	/// @note
	/// A modifier is a special key that modifies the normal action
	/// of another key when the two are pressed in combination. (Thanks wikipedia.)
	/// @note
	/// Crown currently supports three different modifier keys: Shift, Ctrl and Alt.
	bool			modifier_pressed(ModifierKey modifier) const;

	/// Returns whether the specified @a key is pressed.
	bool			key_pressed(KeyCode key) const;

	/// Returns whether the specified @a key is released.
	bool			key_released(KeyCode key) const;

private:

	uint8_t			m_modifier;

	// True if key pressed, false otherwise.
	bool			m_keys[MAX_KEYCODES];

	friend class	InputManager;
};

} // namespace crown

