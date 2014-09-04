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

#include "main.h"
#include "mouse.h"
#include "keyboard.h"
#include "touch.h"
#include "device.h"
#include "os_event_queue.h"
#include "input.h"

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
