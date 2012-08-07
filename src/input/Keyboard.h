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

#include "Types.h"
#include "KeyCode.h"

#define MAX_KEYCODES 256

namespace Crown
{

class InputManager;

/**
	Enumerates modifier keys.
*/
enum ModifierKey
{
	MK_SHIFT	= 1,
	MK_CTRL		= 2,
	MK_ALT		= 4
};

struct KeyboardEvent
{
	Key key;
	char text[4];
};

class KeyboardListener
{

public:

	virtual void KeyPressed(const KeyboardEvent& event) { (void)event; }
	virtual void KeyReleased(const KeyboardEvent& event) { (void)event; }
	virtual void TextInput(const KeyboardEvent& event) { (void)event; }
};

/**
	Interface for accessing keyboard input device.
*/
class Keyboard
{

public:

	/**
		Constructor.
	*/
	Keyboard(InputManager* creator) : mCreator(creator), mListener(0) {}

	/**
		Destructor.
	*/
	virtual ~Keyboard() {}

	/**
		Returns whether the specified modifier is pressed.
	@note
		A modifier is a special key that modifies the normal action
		of another key when the two are pressed in combination. (Thanks wikipedia.)
	@note
		Crown currently supports three different modifier keys: Shift, Ctrl and Alt.
	@param modifier
		The modifier
	@return True if pressed, false otherwise
	*/
	virtual bool IsModifierPressed(ModifierKey modifier) const = 0;

	/**
		Returns whether the specified key is pressed.
	@param key
		The key
	@return
		True if pressed, false otherwise
	*/
	virtual bool IsKeyPressed(KeyCode key) const = 0;

	/**
		Returns whether the specified key is released.
	@param key
		The key
	@return
		True if released, false otherwise
	*/
	virtual bool IsKeyReleased(KeyCode key) const = 0;

	/**
		Captures and reports keyboard-related events.
	*/
	virtual void EventLoop() = 0;

	/**
		Sets the listener for this device.
	@param listener
		The listener
	*/
	inline void SetListener(KeyboardListener* listener) { mListener = listener; }

protected:

	// The InputManager which created the istance
	InputManager* mCreator;
	KeyboardListener* mListener;
};

} // namespace Crown

