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

#include "InputManager.h"

namespace Crown
{

class X11InputManager : public InputManager
{

	friend class X11Mouse;
	friend class X11Keyboard;

public:

	/** @copydoc InputManager::InputManager() */
	X11InputManager();

	/** @copydoc InputManager::~InputManager() */
	virtual ~X11InputManager();

	/** @copydoc InputManager::Init() */
	virtual void Init(const EventSource& source);

	/** @copydoc InputManager::IsMouseAvailable() */
	virtual bool IsMouseAvailable()
	{
		return mIsMouseAvailable;
	}

	/** @copydoc InputManager::IsKeyboardAvailable() */
	virtual bool IsKeyboardAvailable()
	{
		return mIsKeyboardAvailable;
	}

	/** @copydoc InputManager::IsTouchAvailable() */
	virtual bool IsTouchAvailable()
	{
		return false;
	}

	/**
		Returns the X11 Window "attached" to this manager.
	@return
		The X11 Window
	*/
	inline unsigned long GetXWindow()
	{
		return mXWindow;
	}

private:

	void SetMouseAvailable(bool available) { mIsMouseAvailable = available; }
	void SetKeyboardAvailable(bool available) { mIsKeyboardAvailable = available; }

	bool mIsMouseAvailable		: 1;
	bool mIsKeyboardAvailable	: 1;

	// X11 related
	unsigned long mXWindow;
};

} // namespace Crown

