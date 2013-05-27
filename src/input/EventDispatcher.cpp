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
#include "Log.h"

namespace crown
{

//-----------------------------------------------------------------------------
EventDispatcher::EventDispatcher() :
	m_mouse_listener_list(get_default_allocator()),
	m_keyboard_listener_list(get_default_allocator()),
	m_touch_listener_list(get_default_allocator()),
	m_acc_listener_list(get_default_allocator())
{
}

//-----------------------------------------------------------------------------
EventDispatcher::~EventDispatcher()
{
}

//-----------------------------------------------------------------------------
void EventDispatcher::add_mouse_listener(MouseListener* listener)
{
	CE_ASSERT(listener != NULL, "Listener must be != NULL");

	m_mouse_listener_list.push_back(listener);
}

//-----------------------------------------------------------------------------
void EventDispatcher::add_keyboard_listener(KeyboardListener* listener)
{
	CE_ASSERT(listener != NULL, "Listener must be != NULL");

	m_keyboard_listener_list.push_back(listener);
}

//-----------------------------------------------------------------------------
void EventDispatcher::add_touch_listener(TouchListener* listener)
{
	CE_ASSERT(listener != NULL, "Listener must be != NULL");

	m_touch_listener_list.push_back(listener);
}

void EventDispatcher::add_accelerometer_listener(AccelerometerListener* listener)
{
	CE_ASSERT(listener != NULL, "Listener must be != NULL");
	m_acc_listener_list.push_back(listener);
}

//-----------------------------------------------------------------------------
void EventDispatcher::button_pressed(const MouseEvent& event)
{
	for (uint32_t i = 0; i < m_mouse_listener_list.size(); i++)
	{
		m_mouse_listener_list[i]->button_pressed(event);
	}
}

//-----------------------------------------------------------------------------
void EventDispatcher::button_released(const MouseEvent& event)
{
	for (uint32_t i = 0; i < m_mouse_listener_list.size(); i++)
	{
		m_mouse_listener_list[i]->button_released(event);
	}
}

//-----------------------------------------------------------------------------
void EventDispatcher::cursor_moved(const MouseEvent& event)
{
	for (uint32_t i = 0; i < m_mouse_listener_list.size(); i++)
	{
		m_mouse_listener_list[i]->cursor_moved(event);
	}
}

//-----------------------------------------------------------------------------
void EventDispatcher::key_pressed(const KeyboardEvent& event)
{
	for (uint32_t i = 0; i < m_keyboard_listener_list.size(); i++)
	{
		m_keyboard_listener_list[i]->key_pressed(event);
	}
}

//-----------------------------------------------------------------------------
void EventDispatcher::key_released(const KeyboardEvent& event)
{
	for (uint32_t i = 0; i < m_keyboard_listener_list.size(); i++)
	{
		m_keyboard_listener_list[i]->key_released(event);
	}
}

//-----------------------------------------------------------------------------
void EventDispatcher::text_input(const KeyboardEvent& event)
{
	for (uint32_t i = 0; i < m_keyboard_listener_list.size(); i++)
	{
		m_keyboard_listener_list[i]->text_input(event);
	}
}

//-----------------------------------------------------------------------------
void EventDispatcher::touch_down(const TouchEvent& event)
{
	for (uint32_t i = 0; i < m_touch_listener_list.size(); i++)
	{
		m_touch_listener_list[i]->touch_down(event);
	}
}

//-----------------------------------------------------------------------------
void EventDispatcher::touch_up(const TouchEvent& event)
{
	for (uint32_t i = 0; i < m_touch_listener_list.size(); i++)
	{
		m_touch_listener_list[i]->touch_up(event);
	}
}

//-----------------------------------------------------------------------------
void EventDispatcher::touch_move(const TouchEvent& event)
{
	for (uint32_t i = 0; i < m_touch_listener_list.size(); i++)
	{
		m_touch_listener_list[i]->touch_move(event);
	}
}

//-----------------------------------------------------------------------------
void EventDispatcher::touch_cancel(const TouchEvent& event)
{
	for (uint32_t i = 0; i < m_touch_listener_list.size(); i++)
	{
		m_touch_listener_list[i]->touch_cancel(event);
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

