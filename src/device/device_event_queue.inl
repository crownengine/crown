/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "device/types.h"
#include <atomic>
#include <string.h> // memcpy

namespace crown
{
/// Single Producer Single Consumer event queue.
/// Used only to pass events from os thread to main thread.
/// https://www.irif.fr/~guatto/papers/sbac13.pdf
///
/// @ingroup Device
struct DeviceEventQueue
{
	CE_ALIGN_DECL(CROWN_CACHE_LINE_SIZE, std::atomic_int _tail);
	CE_ALIGN_DECL(CROWN_CACHE_LINE_SIZE, std::atomic_int _head);
#define MAX_OS_EVENTS 128
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

	void push_axis_event(u16 device_id, u16 device_num, u16 axis_id, s16 axis_x, s16 axis_y, s16 axis_z)
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

	void push_text_event(u8 len, u8 utf8[4])
	{
		OsEvent ev;
		ev.text.type = OsEventType::TEXT;
		ev.text.len = len;
		memcpy(ev.text.utf8, utf8, sizeof(ev.text.utf8));

		push_event(ev);
	}

	bool push_event(const OsEvent& ev)
	{
		const int tail = _tail.load(std::memory_order_relaxed);
		const int head = _head.load(std::memory_order_acquire);
		const int tail_next = (tail + 1) % MAX_OS_EVENTS;

		if (CE_UNLIKELY(tail_next == head))
			return false;

		_queue[tail] = ev;
		_tail.store(tail_next, std::memory_order_release);
		return true;
	}

	bool pop_event(OsEvent& ev)
	{
		const int head = _head.load(std::memory_order_relaxed);
		const int tail = _tail.load(std::memory_order_acquire);
		const int head_next = (head + 1) % MAX_OS_EVENTS;

		if (CE_UNLIKELY(head == tail))
			return false;

		ev = _queue[head];
		_head.store(head_next, std::memory_order_release);
		return true;
	}
};

} // namespace crown
