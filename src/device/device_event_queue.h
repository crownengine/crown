/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "atomic_int.h"
#include "types.h"

namespace crown
{
struct OsEventType
{
	enum Enum
	{
		BUTTON,
		AXIS,
		STATUS,
		RESOLUTION,
		EXIT,
		PAUSE,
		RESUME,
		NONE
	};
};

struct ButtonEvent
{
	u16 type;
	u16 device_id  : 3;
	u16 device_num : 2;
	u16 button_num : 8;
	u16 pressed    : 1;
};

struct AxisEvent
{
	u16 type;
	u16 device_id  : 3;
	u16 device_num : 2;
	u16 axis_num   : 4;
	f32 axis_x;
	f32 axis_y;
	f32 axis_z;
};

struct StatusEvent
{
	u16 type;
	u16 device_id  : 3;
	u16 device_num : 2;
	u16 connected  : 1;
};

struct ResolutionEvent
{
	u16 type;
	u16 width;
	u16 height;
};

union OsEvent
{
	u16 type;
	ButtonEvent button;
	AxisEvent axis;
	StatusEvent status;
	ResolutionEvent resolution;
};

#define MAX_OS_EVENTS 4096

/// Single Producer Single Consumer event queue.
/// Used only to pass events from os thread to main thread.
struct DeviceEventQueue
{
	AtomicInt _tail;
	AtomicInt _head;
	OsEvent _queue[MAX_OS_EVENTS];

	DeviceEventQueue()
		: _tail(0)
		, _head(0)
	{
	}

	void push_button_event(u16 device_id, u16 device_num, u16 button_id, bool pressed)
	{
		OsEvent ev;
		ev.button.type = OsEventType::BUTTON;
		ev.button.device_id = device_id;
		ev.button.device_num = device_num;
		ev.button.button_num = button_id;
		ev.button.pressed = pressed;

		push_event(ev);
	}

	void push_axis_event(u16 device_id, u16 device_num, u16 axis_id, f32 axis_x, f32 axis_y, f32 axis_z)
	{
		OsEvent ev;
		ev.axis.type = OsEventType::AXIS;
		ev.axis.device_id = device_id;
		ev.axis.device_num = device_num;
		ev.axis.axis_num = axis_id;
		ev.axis.axis_x = axis_x;
		ev.axis.axis_y = axis_y;
		ev.axis.axis_z = axis_z;

		push_event(ev);
	}

	void push_status_event(u16 device_id, u16 device_num, bool connected)
	{
		OsEvent ev;
		ev.status.type = OsEventType::STATUS;
		ev.status.device_id = device_id;
		ev.status.device_num = device_num;
		ev.status.connected = connected;

		push_event(ev);
	}

	void push_resolution_event(u16 width, u16 height)
	{
		OsEvent ev;
		ev.resolution.type = OsEventType::RESOLUTION;
		ev.resolution.width = width;
		ev.resolution.height = height;

		push_event(ev);
	}

	void push_exit_event()
	{
		OsEvent ev;
		ev.type = OsEventType::EXIT;

		push_event(ev);
	}

	void push_none_event()
	{
		OsEvent ev;
		ev.type = OsEventType::NONE;

		push_event(ev);
	}

	bool push_event(const OsEvent& ev)
	{
		int cur_tail = _tail.load();
		int next_tail = increment(cur_tail);

		if (next_tail != _head.load())
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

		if (cur_head == _tail.load())
			return false;

		ev = _queue[cur_head];
		_head.store(increment(cur_head));

		return true;
	}

	int increment(int idx) const
	{
		return (idx + 1) % MAX_OS_EVENTS;
	}
};

} // namespace crown
