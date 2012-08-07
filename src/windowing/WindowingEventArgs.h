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

#include "CoreEventArgs.h"
#include "Device.h"
#include "Mouse.h"
#include "KeyCode.h"

namespace Crown
{

class Widget;

class WindowingEventArgs: public EventArgs
{
public:
	WindowingEventArgs(Widget* source):
		mSource(source), mPropagationStopped(false)
	{
	}
	virtual ~WindowingEventArgs() { }

	inline Widget* GetSource() const
	{
		return mSource;
	}
	inline bool IsPropagationStopped() const
	{
		return mPropagationStopped;
	}

	inline void StopPropagation(bool value)
	{
		mPropagationStopped = value;
	}

private:
	Widget* mSource;
	bool mPropagationStopped;
};

class MouseEventArgs: public WindowingEventArgs
{
public:
	MouseEventArgs(Widget* source):
		WindowingEventArgs(source)
	{
	}
	virtual ~MouseEventArgs() { }

	const Vec2& GetMousePosition();
	const Vec2& GetMousePosition(Widget* w);
};

class MouseMoveEventArgs: public MouseEventArgs
{
public:
	MouseMoveEventArgs(Widget* source, const Point2& delta):
		MouseEventArgs(source), mDelta(delta)
	{
	}

	virtual ~MouseMoveEventArgs()
	{
	}

	inline const Point2& GetDelta()
	{
		return mDelta;
	}

private:
	Point2 mDelta;
};

class MouseButtonEventArgs: public MouseEventArgs
{
public:
	MouseButtonEventArgs(Widget* source, MouseButton button, bool isclick):
		MouseEventArgs(source), mButton(button), mIsClick(isclick)
	{
	}

	virtual ~MouseButtonEventArgs()
	{
	}

	inline MouseButton GetButton()
	{
		return mButton;
	}

	inline bool IsClick()
	{
		return mIsClick;
	}

private:
	MouseButton mButton;
	bool mIsClick;
};

class KeyboardEventArgs: public WindowingEventArgs
{
public:
	KeyboardEventArgs(Widget* source, Key key):
		WindowingEventArgs(source), mKey(key)
	{
	}

	virtual ~KeyboardEventArgs()
	{
	}

	inline Key GetKey()
	{
		return mKey;
	}

private:
	Key mKey;
};

class TextInputEventArgs: public WindowingEventArgs
{
public:
	TextInputEventArgs(Widget* source, Str text):
		WindowingEventArgs(source), mText(text)
	{
	}

	virtual ~TextInputEventArgs()
	{
	}

	inline const Str& GetText() const
	{
		return mText;
	}

private:
	Str mText;
};

class WindowCloseEventArgs: public WindowingEventArgs
{
public:
	WindowCloseEventArgs(Widget* source):
		WindowingEventArgs(source), cancel(false)
	{
	}
	virtual ~WindowCloseEventArgs() { }

	bool cancel;
};

class SelectionChangedEventArgs: public WindowingEventArgs
{
public:
	SelectionChangedEventArgs(Widget* source, int oldIndex, int newIndex):
		WindowingEventArgs(source), mOldIndex(oldIndex), mNewIndex(newIndex)
	{
	}
	virtual ~SelectionChangedEventArgs() { }

	inline int GetOldIndex()
	{
		return mOldIndex;
	}
	inline int GetNewIndex()
	{
		return mNewIndex;
	}

private:
	int mOldIndex;
	int mNewIndex;
};

} //namespace Crown
