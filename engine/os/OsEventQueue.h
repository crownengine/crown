/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
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

#include <cstring>
#include "Queue.h"
#include "Mutex.h"
#include "ProxyAllocator.h"
#include "OsTypes.h"

namespace crown
{

struct OsEventQueue
{
	//-----------------------------------------------------------------------------
	OsEventQueue() 
		: m_allocator("os-event-queue", default_allocator()), m_queue(m_allocator)
	{
	}

	//-----------------------------------------------------------------------------
	void push_mouse_event(uint16_t x, uint16_t y)
	{
		OsEvent ev;
		ev.type = OsEvent::MOUSE;
		ev.mouse.type = OsMouseEvent::MOVE;
		ev.mouse.x = x;
		ev.mouse.y = y;

		push_event(&ev);
	}

	//-----------------------------------------------------------------------------
	void push_mouse_event(uint16_t x, uint16_t y, MouseButton::Enum b, bool pressed)
	{
		OsEvent ev;
		ev.type = OsEvent::MOUSE;
		ev.mouse.type = OsMouseEvent::BUTTON;
		ev.mouse.x = x;
		ev.mouse.y = y;
		ev.mouse.button = b;
		ev.mouse.pressed = pressed;

		push_event(&ev);
	}

	//-----------------------------------------------------------------------------
	void push_keyboard_event(uint32_t modifier, KeyboardButton::Enum b, bool pressed)
	{
		OsEvent ev;
		ev.type = OsEvent::KEYBOARD;
		ev.keyboard.button = b;
		ev.keyboard.modifier = modifier;
		ev.keyboard.pressed = pressed;

		push_event(&ev);
	}

	//-----------------------------------------------------------------------------
	void push_exit_event(int32_t code)
	{
		OsEvent ev;
		ev.type = OsEvent::EXIT;
		ev.exit.code = code;

		push_event(&ev);
	}

	//-----------------------------------------------------------------------------
	void push_metrics_event(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
	{
		OsEvent ev;
		ev.type = OsEvent::METRICS;
		ev.metrics.x = x;
		ev.metrics.y = y;
		ev.metrics.width = width;
		ev.metrics.height = height;

		push_event(&ev);
	}

	//-----------------------------------------------------------------------------
	void push_event(OsEvent* ev)
	{
		CE_ASSERT_NOT_NULL(ev);

		m_mutex.lock();
		m_queue.push(ev, sizeof(OsEvent));
		m_mutex.unlock();
	}

	//-----------------------------------------------------------------------------
	void pop_event(OsEvent* ev)
	{
		CE_ASSERT_NOT_NULL(ev);

		m_mutex.lock();
		if (m_queue.size() > 0)
		{
			memcpy(ev, m_queue.begin(), sizeof(OsEvent));
			m_queue.pop(sizeof(OsEvent));
		}
		else
		{
			ev->type = OsEvent::NONE;
		}
		m_mutex.unlock();
	}

private:

	ProxyAllocator m_allocator;
	Queue<OsEvent> m_queue;
	Mutex m_mutex;
};

} // namespace crown
