#include "OS.h"

namespace crown
{

namespace os
{

#define				MAX_OS_EVENTS 256

static int			event_queue_count = 0;
static OSEvent		event_queue[MAX_OS_EVENTS];

//-----------------------------------------------------------------------------
void push_event(OSEventType type, int data_a, int data_b, int data_c, int data_d)
{
	// The queue wraps silently when MAX_OS_EVENTS reached
	OSEvent* event = &event_queue[event_queue_count % MAX_OS_EVENTS];

	event_queue_count++;

	event->type = type;
	event->data_a = data_a;
	event->data_b = data_b;
	event->data_c = data_c;
	event->data_d = data_d;
}

//-----------------------------------------------------------------------------
OSEvent& pop_event()
{
	OSEvent event;

	if (event_queue_count > 0)
	{
		event = event_queue[event_queue_count - 1 % MAX_OS_EVENTS];
		event_queue_count--;

		return event;
	}

	event.type = OSET_NONE;

	return event;
}

} // namespace os

} // namespace crown

