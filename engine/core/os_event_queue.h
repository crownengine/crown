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
#include "types.h"
#include "mouse.h"
#include "keyboard.h"
#include "atomic_int.h"

namespace crown
{

struct OsMetricsEvent
{
	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t height;
};

struct OsExitEvent
{
	int32_t code;
};

/// Represents an event fired by mouse.
struct OsMouseEvent
{
	enum Enum
	{
		BUTTON,
		MOVE
	};

	OsMouseEvent::Enum type;
	MouseButton::Enum button;
	uint16_t x;
	uint16_t y;
	bool pressed;
};

/// Represents an event fired by keyboard.
struct OsKeyboardEvent
{
	KeyboardButton::Enum button;
	uint32_t modifier;
	bool pressed;
};

/// Represents an event fired by touch screen.
struct OsTouchEvent
{
	enum Enum
	{
		POINTER,
		MOVE
	};

	OsTouchEvent::Enum type;
	uint8_t pointer_id;
	uint16_t x;
	uint16_t y;
	bool pressed;
};

/// Represents an event fired by accelerometer.
struct OsAccelerometerEvent
{
	float x;
	float y;
	float z;
};

struct OsEvent
{
	/// Represents an event fired by the OS
	enum Enum
	{
		NONE			= 0,

		KEYBOARD		= 1,
		MOUSE			= 2,
		TOUCH			= 3,
		ACCELEROMETER	= 4,

		METRICS,
		PAUSE,
		RESUME,
		// Exit from program
		EXIT
	};

	OsEvent::Enum type;
	union
	{
		OsMetricsEvent metrics;
		OsExitEvent exit;
		OsMouseEvent mouse;
		OsKeyboardEvent keyboard;
		OsTouchEvent touch;
		OsAccelerometerEvent accelerometer;
	};
};

#define MAX_OS_EVENTS 64

/// Single Producer Single Consumer event queue.
/// Used only to pass events from os thread to main thread.
struct OsEventQueue
{
	OsEventQueue()
		: m_tail(0)
		, m_head(0)
	{
	}

	void push_mouse_event(uint16_t x, uint16_t y)
	{
		OsEvent ev;
		ev.type = OsEvent::MOUSE;
		ev.mouse.type = OsMouseEvent::MOVE;
		ev.mouse.x = x;
		ev.mouse.y = y;

		push_event(ev);
	}

	void push_mouse_event(uint16_t x, uint16_t y, MouseButton::Enum b, bool pressed)
	{
		OsEvent ev;
		ev.type = OsEvent::MOUSE;
		ev.mouse.type = OsMouseEvent::BUTTON;
		ev.mouse.x = x;
		ev.mouse.y = y;
		ev.mouse.button = b;
		ev.mouse.pressed = pressed;

		push_event(ev);
	}

	void push_keyboard_event(uint32_t modifier, KeyboardButton::Enum b, bool pressed)
	{
		OsEvent ev;
		ev.type = OsEvent::KEYBOARD;
		ev.keyboard.button = b;
		ev.keyboard.modifier = modifier;
		ev.keyboard.pressed = pressed;

		push_event(ev);
	}

	void push_touch_event(uint16_t x, uint16_t y, uint8_t pointer_id)
	{
		OsEvent ev;
		ev.type = OsEvent::TOUCH;
		ev.touch.type = OsTouchEvent::MOVE;
		ev.touch.x = x;
		ev.touch.y = y;
		ev.touch.pointer_id = pointer_id;

		push_event(ev);
	}

	void push_touch_event(uint16_t x, uint16_t y, uint8_t pointer_id, bool pressed)
	{
		OsEvent ev;
		ev.type = OsEvent::TOUCH;
		ev.touch.type = OsTouchEvent::POINTER;
		ev.touch.x = x;
		ev.touch.y = y;
		ev.touch.pointer_id = pointer_id;
		ev.touch.pressed = pressed;

		push_event(ev);
	}

	void push_exit_event(int32_t code)
	{
		OsEvent ev;
		ev.type = OsEvent::EXIT;
		ev.exit.code = code;

		push_event(ev);
	}

	void push_pause_event()
	{
		OsEvent ev;
		ev.type = OsEvent::PAUSE;
		push_event(ev);
	}

	void push_resume_event()
	{
		OsEvent ev;
		ev.type = OsEvent::RESUME;
		push_event(ev);
	}

	void push_metrics_event(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
	{
		OsEvent ev;
		ev.type = OsEvent::METRICS;
		ev.metrics.x = x;
		ev.metrics.y = y;
		ev.metrics.width = width;
		ev.metrics.height = height;

		push_event(ev);
	}

	void push_none_event()
	{
		OsEvent ev;
		ev.type = OsEvent::NONE;

		push_event(ev);
	}

	bool push_event(const OsEvent& ev)
	{
		int cur_tail = m_tail.load();
		int next_tail = increment(cur_tail);
		if(next_tail != m_head.load())
		{
			m_queue[cur_tail] = ev;
			m_tail.store(next_tail);
			return true;
		}

		return false;
	}

	bool pop_event(OsEvent& ev)
	{
		const int cur_head = m_head.load();
		if(cur_head == m_tail.load()) return false;

		ev = m_queue[cur_head];
		m_head.store(increment(cur_head));
		return true;
	}

	int increment(int idx) const
	{
	  return (idx + 1) % MAX_OS_EVENTS;
	}

private:

	OsEvent m_queue[MAX_OS_EVENTS];
	AtomicInt m_tail;
	AtomicInt m_head;
};

} // namespace crown
