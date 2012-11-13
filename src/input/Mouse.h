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

#include "Point2.h"
#include "Vec2.h"

namespace crown
{

class InputManager;

/**
	Enumerates mouse buttons.
*/
enum MouseButton
{
	MB_LEFT		= 0,
	MB_MIDDLE	= 1,
	MB_RIGHT	= 2
};

struct MouseEvent
{
	MouseButton button;
	int x;
	int y;
	float wheel;
};

class MouseListener
{

public:

	virtual void ButtonPressed(const MouseEvent& event) { (void)event; }
	virtual void ButtonReleased(const MouseEvent& event) { (void)event; }
	virtual void CursorMoved(const MouseEvent& event) { (void)event; }
};

/**
	Interface for accessing mouse input device.
*/
class Mouse
{

public:

	/**
		Constructor.
	*/
	Mouse() : mListener(NULL) {}

	/**
		Destructor.
	*/
	virtual ~Mouse() {}

	/**
		Returns whether the cursor is visible.
	*/
	virtual bool IsCursorVisible() const = 0;

	/**
		Sets whether the cursor is visible.
	*/
	virtual void SetCursorVisible(bool visible) = 0;

	/**
		Returns the position of the cursor in window space.
	@note
		Coordinates in window space have the origin at the
		upper-left corner of the window. +X extends from left
		to right and +Y extends from top to bottom.
	*/
	virtual Point2 GetCursorXY() const = 0;

	/**
		Sets the position of the cursor in window space.
	@note
		Coordinates in window space have the origin at the
		upper-left corner of the window. +X extends from left
		to right and +Y extends from top to bottom.
	*/
	virtual void SetCursorXY(const Point2& position) = 0;

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
	virtual Vec2 GetCursorRelativeXY() const = 0;

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
	virtual void SetCursorRelativeXY(const Vec2& position) = 0;

	/**
		Sets the listener for this device.
	*/
	inline void SetListener(MouseListener* listener) { mListener = listener; }

protected:

	MouseListener* mListener;
};

} // namespace crown

