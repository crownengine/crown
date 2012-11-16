#include "OS.h"

namespace crown
{

namespace os
{

static OSEvent		event_queue[MAX_OS_EVENTS];
static int32_t			event_queue_head = 0;
static int32_t			event_queue_tail = 0;

//-----------------------------------------------------------------------------
void push_event(OSEventType type, int32_t data_a, int32_t data_b, int32_t data_c, int32_t data_d)
{
	if ((event_queue_tail + 1) % MAX_OS_EVENTS == event_queue_head)
	{
		os::printf("OS event queue full!\n");
		return;
	}

	OSEvent* event = &event_queue[event_queue_tail];
	event_queue_tail = (event_queue_tail + 1) % MAX_OS_EVENTS;

	event->type = type;
	event->data_a = data_a;
	event->data_b = data_b;
	event->data_c = data_c;
	event->data_d = data_d;
}

//-----------------------------------------------------------------------------
OSEvent& pop_event()
{
	static OSEvent event;

	if (event_queue_head == event_queue_tail)
	{
		event.type = OSET_NONE;
		return event;
	}

	event = event_queue[event_queue_head];
	event_queue_head = (event_queue_head + 1) % MAX_OS_EVENTS;

	return event;
}

} // namespace os

} // namespace crown

