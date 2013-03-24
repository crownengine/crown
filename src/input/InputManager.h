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
#include "Point2.h"
#include "Vec2.h"

namespace crown
{

class MouseListener;
class KeyboardListener;
class TouchListener;

class InputManager
{

public:

	InputManager();
	~InputManager();

	//! Initializes the input manager.
	void Init();

	bool IsMouseAvailable() {}
	bool IsKeyboardAvailable() {}
	bool IsTouchAvailable() {}
	bool IsAccelerometerAvailable() {}


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

	inline void register_accelerometer_listener(AccelerometerListener* listener)
	{
//		mEventDispatcher.AddAccelerometerListener(listener)
	}

	inline EventDispatcher* GetEventDispatcher()
	{
		return &mEventDispatcher;
	}

	void EventLoop();

	//! Returns whether the cursor is visible.
	bool is_cursor_visible() const;

	//! Sets whether the cursor is visible.
	void set_cursor_visible(bool visible);

	/**
		Returns the position of the cursor in window space.
	@note
		Coordinates in window space have the origin at the
		upper-left corner of the window. +X extends from left
		to right and +Y extends from top to bottom.
	*/
	Point2 get_cursor_xy() const;

	/**
		Sets the position of the cursor in window space.
	@note
		Coordinates in window space have the origin at the
		upper-left corner of the window. +X extends from left
		to right and +Y extends from top to bottom.
	*/
	void set_cursor_xy(const Point2& position);

	/**
		Returns the relative position of the cursor in window space.
	@note
		Coordinates in window space have the origin at the
		upper-left corner of the window. +X extends from left
		to right and +Y extends from top to bottom.
	@note
		Relative coordinates are mapped to a real varying
		from 0.0 to 1.0 where 0.0 is the origin and 1.0 the
		maximum extent of the cosidered axis.
	*/
	Vec2 get_cursor_relative_xy() const;

	/**
		Sets the relative position of the cursor in window space.
	@note
		Coordinates in window space have the origin at the
		upper-left corner of the window. +X extends from left
		to right and +Y extends from top to bottom.
	@note
		Relative coordinates are mapped to a real varying
		from 0.0 to 1.0 where 0.0 is the origin and 1.0 the
		maximum extent of the cosidered axis.
	*/
	void set_cursor_relative_xy(const Vec2& position);

private:

	EventDispatcher		mEventDispatcher;

	bool				m_cursor_visible;
};

InputManager* GetInputManager();

} // namespace crown

