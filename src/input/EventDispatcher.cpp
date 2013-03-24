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
#include "Allocator.h"

namespace crown
{

//-----------------------------------------------------------------------------
EventDispatcher::EventDispatcher() :
	mMouseListenerList(get_default_allocator()),
	mKeyboardListenerList(get_default_allocator()),
	mTouchListenerList(get_default_allocator()),
	m_acc_listener_list(get_default_allocator())
{
}

//-----------------------------------------------------------------------------
EventDispatcher::~EventDispatcher()
{
}

//-----------------------------------------------------------------------------
void EventDispatcher::AddMouseListener(MouseListener* listener)
{
	assert(listener != NULL);

	mMouseListenerList.push_back(listener);
}

//-----------------------------------------------------------------------------
void EventDispatcher::AddKeyboardListener(KeyboardListener* listener)
{
	assert(listener != NULL);

	mKeyboardListenerList.push_back(listener);
}

//-----------------------------------------------------------------------------
void EventDispatcher::AddTouchListener(TouchListener* listener)
{
	assert(listener != NULL);

	mTouchListenerList.push_back(listener);
}

//-----------------------------------------------------------------------------
void EventDispatcher::ButtonPressed(const MouseEvent& event)
{
	for (uint32_t i = 0; i < mMouseListenerList.size(); i++)
	{
		mMouseListenerList[i]->ButtonPressed(event);
	}
}

//-----------------------------------------------------------------------------
void EventDispatcher::ButtonReleased(const MouseEvent& event)
{
	for (uint32_t i = 0; i < mMouseListenerList.size(); i++)
	{
		mMouseListenerList[i]->ButtonReleased(event);
	}
}

//-----------------------------------------------------------------------------
void EventDispatcher::CursorMoved(const MouseEvent& event)
{
	for (uint32_t i = 0; i < mMouseListenerList.size(); i++)
	{
		mMouseListenerList[i]->CursorMoved(event);
	}
}

//-----------------------------------------------------------------------------
void EventDispatcher::KeyPressed(const KeyboardEvent& event)
{
	for (uint32_t i = 0; i < mKeyboardListenerList.size(); i++)
	{
		mKeyboardListenerList[i]->KeyPressed(event);
	}
}

//-----------------------------------------------------------------------------
void EventDispatcher::KeyReleased(const KeyboardEvent& event)
{
	for (uint32_t i = 0; i < mKeyboardListenerList.size(); i++)
	{
		mKeyboardListenerList[i]->KeyReleased(event);
	}
}

//-----------------------------------------------------------------------------
void EventDispatcher::TextInput(const KeyboardEvent& event)
{
	for (uint32_t i = 0; i < mKeyboardListenerList.size(); i++)
	{
		mKeyboardListenerList[i]->TextInput(event);
	}
}

//-----------------------------------------------------------------------------
void EventDispatcher::TouchDown(const TouchEvent& event)
{
	for (uint32_t i = 0; i < mTouchListenerList.size(); i++)
	{
		mTouchListenerList[i]->TouchDown(event);
	}
}

//-----------------------------------------------------------------------------
void EventDispatcher::TouchUp(const TouchEvent& event)
{
	for (uint32_t i = 0; i < mTouchListenerList.size(); i++)
	{
		mTouchListenerList[i]->TouchUp(event);
	}
}

//-----------------------------------------------------------------------------
void EventDispatcher::TouchMove(const TouchEvent& event)
{
	for (uint32_t i = 0; i < mTouchListenerList.size(); i++)
	{
		mTouchListenerList[i]->TouchMove(event);
	}
}

//-----------------------------------------------------------------------------
void EventDispatcher::TouchCancel(const TouchEvent& event)
{
	for (uint32_t i = 0; i < mTouchListenerList.size(); i++)
	{
		mTouchListenerList[i]->TouchCancel(event);
	}
}

//-----------------------------------------------------------------------------
void EventDispatcher::accelerometer_changed(const AccelerometerEvent& event)
{
	for (uint32_t i = 0; i < m_acc_listener_list.size(); i++)
	{
		m_acc_listener_list[i]->accelerometer_changed(event);
	}
}

} // namespace crown

