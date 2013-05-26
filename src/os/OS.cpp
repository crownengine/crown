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

#include "OS.h"

namespace crown
{

static const size_t		MAX_EVENTS = 512;
static OsEvent			event_queue[MAX_EVENTS];
static uint32_t			event_queue_head = 0;
static uint32_t			event_queue_tail = 0;

//-----------------------------------------------------------------------------
void push_event(OsEventType type, OsEventParameter data_a, OsEventParameter data_b, OsEventParameter data_c, OsEventParameter data_d)
{
	if ((event_queue_tail + 1) % MAX_EVENTS == event_queue_head)
	{
		os::printf("OS event queue full!\n");
		return;
	}

	OsEvent* event = &event_queue[event_queue_tail];
	event_queue_tail = (event_queue_tail + 1) % MAX_EVENTS;

	event->type = type;
	event->data_a = data_a;
	event->data_b = data_b;
	event->data_c = data_c;
	event->data_d = data_d;
}

//-----------------------------------------------------------------------------
OsEvent& pop_event()
{
	static OsEvent event;

	if (event_queue_head == event_queue_tail)
	{
		event.type = OSET_NONE;
		return event;
	}

	event = event_queue[event_queue_head];
	event_queue_head = (event_queue_head + 1) % MAX_EVENTS;

	return event;
}

} // namespace crown

