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

#include "List.h"
#include "Mouse.h"
#include "Keyboard.h"
#include "Touch.h"

namespace crown
{

class EventDispatcher : public MouseListener, public KeyboardListener, public TouchListener
{
	typedef List<MouseListener*> MouseListenerList;
	typedef List<KeyboardListener*> KeyboardListenerList;
	typedef List<TouchListener*> TouchListenerList;

public:

	/**
		Constructor.
	*/
	EventDispatcher() {}

	/**
		Destructor.	
	*/
	~EventDispatcher() {}

	/**
		Adds a MouseListener.
	@param listener
		The listener
	*/
	void AddMouseListener(MouseListener* listener);

	/**
		Adds a KeyboardListener.
	@param listener
		The listener
	*/
	void AddKeyboardListener(KeyboardListener* listener);

	/**
		Adds a TouchListener.
	@param listener
		The listener
	*/
	void AddTouchListener(TouchListener* listener);

	virtual void ButtonPressed(const MouseEvent&);
	virtual void ButtonReleased(const MouseEvent&);
	virtual void CursorMoved(const MouseEvent&);

	virtual void KeyPressed(const KeyboardEvent&);
	virtual void KeyReleased(const KeyboardEvent&);
	virtual void TextInput(const KeyboardEvent&);

	virtual void TouchDown(const TouchEvent& event);
	virtual void TouchUp(const TouchEvent& event);
	virtual void TouchMove(const TouchEvent& event);
	virtual void TouchCancel(const TouchEvent& event);

private:

	MouseListenerList mMouseListenerList;
	KeyboardListenerList mKeyboardListenerList;
	TouchListenerList mTouchListenerList;
};

} // namespace crown

