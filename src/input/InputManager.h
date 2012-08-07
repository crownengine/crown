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

#include "EventDispatcher.h"

struct android_app;

namespace Crown
{

union EventSource
{
	unsigned long WindowHandle;
	android_app* AndroidApp;
};

class Mouse;
class Keyboard;
class Touch;
class MouseListener;
class KeyboardListener;
class TouchListener;
class RenderWindow;

class InputManager
{

public:

	static InputManager* CreateInputManager(RenderWindow* window);
	static void DestroyInputManager(InputManager* manager);

	/**
		Constructor.
	*/
	InputManager() : mMouse(NULL), mKeyboard(NULL), mTouch(NULL) {}

	/**
		Destructor.
	*/
	virtual ~InputManager() {}

	/**
		Initializes the input manager.
	*/
	virtual void Init(const EventSource& source) = 0;

	/**
		Returns whether the mouse is available.
	@return
		True if available, false otherwise
	*/
	virtual bool IsMouseAvailable() = 0;

	/**
		Returns whether the keyboard is available.
	@return
		True if available, false otherwise
	*/
	virtual bool IsKeyboardAvailable() = 0;

	/**
		Returns whether the touch is available.
	@return
		True if available, false otherwise
	*/
	virtual bool IsTouchAvailable() = 0;

	/**
		Returns the handle to the mouse input device.
	@return
		The mouse input device or NULL
	*/
	inline Mouse* GetMouse()
	{
		return mMouse;
	}

	/**
		Returns the handle to the keyboard input device.
	@return
		The keyboard input device or NULL
	*/
	inline Keyboard* GetKeyboard()
	{
		return mKeyboard;
	}

	/**
		Return the handle to the touch input device.
	@return
		The touch input device or NULL
	*/
	inline Touch* GetTouch()
	{
		return mTouch;
	}

	inline void RegisterMouseListener(MouseListener* listener)
	{
		mEventDispatcher.AddMouseListener(listener);
	}

	inline void RegisterKeyboardListener(KeyboardListener* listener)
	{
		mEventDispatcher.AddKeyboardListener(listener);
	}

	inline void RegisterTouchListener(TouchListener* listener)
	{
		mEventDispatcher.AddTouchListener(listener);
	}

	inline EventDispatcher* GetEventDispatcher()
	{
		return &mEventDispatcher;
	}

protected:

	EventDispatcher mEventDispatcher;
	Mouse* mMouse;
	Keyboard* mKeyboard;
	Touch* mTouch;
};

} // namespace Crown

