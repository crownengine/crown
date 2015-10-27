/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "input_types.h"
#include "math_types.h"
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
		WHEEL,
		MOVE
	};

	OsMouseEvent::Enum type;
	MouseButton::Enum button;
	int16_t x;
	int16_t y;
	bool pressed;
	float wheel;
};

/// Represents an event fired by keyboard.
struct OsKeyboardEvent
{
	KeyboardButton::Enum button;
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
	int16_t x;
	int16_t y;
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
		NONE,

		KEYBOARD,
		MOUSE,
		TOUCH,
		ACCELEROMETER,

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
		: _tail(0)
		, _head(0)
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

	void push_mouse_event(int16_t x, int16_t y, MouseButton::Enum b, bool pressed)
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

	void push_mouse_event(int16_t x, int16_t y, float wheel)
	{
		OsEvent ev;
		ev.type = OsEvent::MOUSE;
		ev.mouse.type = OsMouseEvent::WHEEL;
		ev.mouse.x = x;
		ev.mouse.y = y;
		ev.mouse.wheel = wheel;

		push_event(ev);
	}

	void push_keyboard_event(KeyboardButton::Enum b, bool pressed)
	{
		OsEvent ev;
		ev.type = OsEvent::KEYBOARD;
		ev.keyboard.button = b;
		ev.keyboard.pressed = pressed;

		push_event(ev);
	}

	void push_touch_event(int16_t x, int16_t y, uint8_t pointer_id)
	{
		OsEvent ev;
		ev.type = OsEvent::TOUCH;
		ev.touch.type = OsTouchEvent::MOVE;
		ev.touch.x = x;
		ev.touch.y = y;
		ev.touch.pointer_id = pointer_id;

		push_event(ev);
	}

	void push_touch_event(int16_t x, int16_t y, uint8_t pointer_id, bool pressed)
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
		int cur_tail = _tail.load();
		int next_tail = increment(cur_tail);
		if(next_tail != _head.load())
		{
			_queue[cur_tail] = ev;
			_tail.store(next_tail);
			return true;
		}

		return false;
	}

	bool pop_event(OsEvent& ev)
	{
		const int cur_head = _head.load();
		if(cur_head == _tail.load()) return false;

		ev = _queue[cur_head];
		_head.store(increment(cur_head));
		return true;
	}

	int increment(int idx) const
	{
	  return (idx + 1) % MAX_OS_EVENTS;
	}

private:

	OsEvent _queue[MAX_OS_EVENTS];
	AtomicInt _tail;
	AtomicInt _head;
};

} // namespace crown
