/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "main.h"
#include "device.h"
#include "os_event_queue.h"
#include "input_manager.h"
#include "input_device.h"
#include "error.h"
#include "vector3.h"
#include <bgfx.h>

namespace crown
{

bool process_events()
{
	OsEvent event;
	bool exit = false;
	InputManager* im = device()->input_manager();

	while(next_event(event))
	{
		if (event.type == OsEvent::NONE) continue;

		switch (event.type)
		{
			case OsEvent::TOUCH:
			{
				const OsTouchEvent& ev = event.touch;
				switch (ev.type)
				{
					case OsTouchEvent::POINTER:
						im->touch()->set_button_state(ev.pointer_id, ev.pressed);
						break;
					case OsTouchEvent::MOVE:
						im->touch()->set_axis(ev.pointer_id, vector3(ev.x, ev.y, 0.0f));
						break;
					default:
						CE_FATAL("Unknown touch event type");
						break;
				}
				break;
			}
			case OsEvent::MOUSE:
			{
				const OsMouseEvent& ev = event.mouse;
				switch (ev.type)
				{
					case OsMouseEvent::BUTTON:
						im->mouse()->set_button_state(ev.button, ev.pressed);
						break;
					case OsMouseEvent::MOVE:
						im->mouse()->set_axis(0, vector3(ev.x, ev.y, 0.0f));
						break;
					case OsMouseEvent::WHEEL:
						im->mouse()->set_axis(1, vector3(ev.wheel, 0.0f, 0.0f));
						break;
					default:
						CE_FATAL("Unknown mouse event type");
						break;
				}
				break;
			}
			case OsEvent::KEYBOARD:
			{
				const OsKeyboardEvent& ev = event.keyboard;
				im->keyboard()->set_button_state(ev.button, ev.pressed);
				break;
			}
			case OsEvent::JOYPAD:
			{
				const OsJoypadEvent& ev = event.joypad;
				switch (ev.type)
				{
					case OsJoypadEvent::BUTTON:
						im->joypad()->set_button_state(ev.button, ev.pressed);
						break;
					case OsJoypadEvent::AXIS:
						im->joypad()->set_axis(ev.button, vector3(ev.x, ev.y, ev.z));
						break;
					default:
						CE_FATAL("Unknown joypad event");
						break;
				}
				break;
			}
			case OsEvent::METRICS:
			{
				const OsMetricsEvent& ev = event.metrics;
				device()->update_resolution(ev.width, ev.height);
				bgfx::reset(ev.width, ev.height, BGFX_RESET_VSYNC);
				break;
			}
			case OsEvent::EXIT:
			{
				exit = true;
				break;
			}
			case OsEvent::PAUSE:
			{
				device()->pause();
				break;
			}
			case OsEvent::RESUME:
			{
				device()->unpause();
				break;
			}
			default:
			{
				CE_FATAL("Unknown Os Event");
				break;
			}
		}
	}

	return exit;
}

} // namespace crown
