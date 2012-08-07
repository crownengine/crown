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

#include "EventDispatcher.h"
#include "Exceptions.h"

namespace Crown
{

void EventDispatcher::AddMouseListener(MouseListener* listener)
{
	if (listener == 0)
	{
		throw ArgumentException("EventDispatcher::AddMouseListener: listener == NULL.");
	}

	mMouseListenerList.Append(listener);
}

void EventDispatcher::AddKeyboardListener(KeyboardListener* listener)
{
	if (listener == 0)
	{
		throw ArgumentException("EventDispatcher::AddKeyboardListener: listener == NULL.");
	}

	mKeyboardListenerList.Append(listener);
}

void EventDispatcher::AddTouchListener(TouchListener* listener)
{
	if (listener == 0)
	{
		throw ArgumentException("EventDispatcher::AddTouchListener: listener == NULL.");
	}

	mTouchListenerList.Append(listener);
}

void EventDispatcher::ButtonPressed(const MouseEvent& event)
{
	for (int i = 0; i < mMouseListenerList.GetSize(); i++)
	{
		mMouseListenerList[i]->ButtonPressed(event);
	}
}

void EventDispatcher::ButtonReleased(const MouseEvent& event)
{
	for (int i = 0; i < mMouseListenerList.GetSize(); i++)
	{
		mMouseListenerList[i]->ButtonReleased(event);
	}
}

void EventDispatcher::CursorMoved(const MouseEvent& event)
{
	for (int i = 0; i < mMouseListenerList.GetSize(); i++)
	{
		mMouseListenerList[i]->CursorMoved(event);
	}
}

void EventDispatcher::KeyPressed(const KeyboardEvent& event)
{
	for (int i = 0; i < mKeyboardListenerList.GetSize(); i++)
	{
		mKeyboardListenerList[i]->KeyPressed(event);
	}
}

void EventDispatcher::KeyReleased(const KeyboardEvent& event)
{
	for (int i = 0; i < mKeyboardListenerList.GetSize(); i++)
	{
		mKeyboardListenerList[i]->KeyReleased(event);
	}
}

void EventDispatcher::TextInput(const KeyboardEvent& event)
{
	for (int i = 0; i < mKeyboardListenerList.GetSize(); i++)
	{
		mKeyboardListenerList[i]->TextInput(event);
	}
}

void EventDispatcher::TouchDown(const TouchEvent& event)
{
	for (int i = 0; i < mTouchListenerList.GetSize(); i++)
	{
		mTouchListenerList[i]->TouchDown(event);
	}
}

void EventDispatcher::TouchUp(const TouchEvent& event)
{
	for (int i = 0; i < mTouchListenerList.GetSize(); i++)
	{
		mTouchListenerList[i]->TouchUp(event);
	}
}

void EventDispatcher::TouchMove(const TouchEvent& event)
{
	for (int i = 0; i < mTouchListenerList.GetSize(); i++)
	{
		mTouchListenerList[i]->TouchMove(event);
	}
}

void EventDispatcher::TouchCancel(const TouchEvent& event)
{
	for (int i = 0; i < mTouchListenerList.GetSize(); i++)
	{
		mTouchListenerList[i]->TouchCancel(event);
	}
}

} // namespace Crown

