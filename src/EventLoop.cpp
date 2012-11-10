#include "EventLoop.h"
#include "OS.h"

namespace crown
{

void EventLoop::consume_events()
{
	while (1)
	{
		os::OSEvent event;

		event = os::pop_event();

		if (event.type == os::OSET_NONE)
		{
			break;
		}

		if (event.type == os::OSET_KEYBOARD)
		{

		}
	}
}

} // namespace crown

