/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "main.h"
#include "mouse.h"
#include "keyboard.h"
#include "touch.h"
#include "device.h"
#include "os_event_queue.h"
#include "input.h"
#include <bgfx.h>

namespace crown
{

bool process_events()
{
	using namespace input_globals;

	OsEvent event;
	bool exit = false;

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
					case OsTouchEvent::POINTER: touch().set_pointer_state(ev.x, ev.y, ev.pointer_id, ev.pressed); break;
					case OsTouchEvent::MOVE: touch().set_position(ev.pointer_id, ev.x, ev.y); break;
					default: CE_FATAL("Oops, unknown touch event type"); break;
				}
				break;
			}
			case OsEvent::MOUSE:
			{
				const OsMouseEvent& ev = event.mouse;
				switch (ev.type)
				{
					case OsMouseEvent::BUTTON: mouse().set_button_state(ev.x, ev.y, ev.button, ev.pressed); break;
					case OsMouseEvent::WHEEL: mouse().set_wheel(ev.x, ev.y, ev.wheel); break;
					case OsMouseEvent::MOVE: mouse().set_position(ev.x, ev.y); break;
					default: CE_FATAL("Oops, unknown mouse event type"); break;
				}
				break;
			}
			case OsEvent::KEYBOARD:
			{
				const OsKeyboardEvent& ev = event.keyboard;
				keyboard().set_button_state(ev.button, ev.pressed);
				break;
			}
			case OsEvent::METRICS:
			{
				const OsMetricsEvent& ev = event.metrics;
				mouse().set_metrics(ev.width, ev.height);
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
