#include "EventLoop.h"
#include "OS.h"

namespace Crown
{

void EventLoop::consume_events()
{
	while (1)
	{
		OS::OSEvent event;

		event = OS::pop_event();

		if (event.type == OS::OSET_NONE)
		{
			break;
		}

		if (event.type == OS::OSET_KEYBOARD)
		{

		}
	}
}

} // namespace Crown

