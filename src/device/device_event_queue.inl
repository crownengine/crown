/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "device/types.h"
#include <string.h> // memcpy

namespace crown
{
typedef bool (*QueuePushFunction)(const OsEvent &ev);

/// Used only to pass events from OS to main thread.
///
/// @ingroup Device
struct DeviceEventQueue
{
	QueuePushFunction _queue_push_function;

	///
	explicit DeviceEventQueue(QueuePushFunction fn)
		: _queue_push_function(fn)
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

		_queue_push_function(ev);
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

		_queue_push_function(ev);
	}

	void push_status_event(u16 device_id, u16 device_num, bool connected)
	{
		OsEvent ev;
		ev.status.type = OsEventType::STATUS;
		ev.status.device_id = device_id;
		ev.status.device_num = device_num;
		ev.status.connected = connected;

		_queue_push_function(ev);
	}

	void push_resolution_event(u16 width, u16 height)
	{
		OsEvent ev;
		ev.resolution.type = OsEventType::RESOLUTION;
		ev.resolution.width = width;
		ev.resolution.height = height;

		_queue_push_function(ev);
	}

	void push_exit_event()
	{
		OsEvent ev;
		ev.type = OsEventType::EXIT;

		_queue_push_function(ev);
	}

	void push_text_event(u8 len, u8 utf8[4])
	{
		OsEvent ev;
		ev.text.type = OsEventType::TEXT;
		ev.text.len = len;
		memcpy(ev.text.utf8, utf8, sizeof(ev.text.utf8));

		_queue_push_function(ev);
	}
};

} // namespace crown
