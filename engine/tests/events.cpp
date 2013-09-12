#include "Crown.h"
#include "EventBuffer.h"

using namespace crown;

//-----------------------------------------------------------------------------
struct TestEvent
{
	uint32_t a;
	uint32_t b;
	uint32_t c;
};

//-----------------------------------------------------------------------------
int main()
{
	EventBuffer event_buffer;

	TestEvent event;
	event.a = 1;
	event.b = 2;
	event.c = 3;

	for (uint32_t i = 0; i < 10; i++)
	{
		event_buffer.push_event(0, &event, sizeof(TestEvent));
	}

	CE_ASSERT(event_buffer.size() == (10 * (sizeof(TestEvent) + sizeof(uint32_t) + sizeof(size_t))), "Something ha gone wrong");

	TestEvent* result;
	uint32_t type = 0;
	size_t size = 0;

	uint32_t count = 0;
	while ((result = (TestEvent*)event_buffer.get_next_event(type, size)) != NULL)
	{
		Log::i("type => %d", type);
		Log::i("size => %d", size);	
		Log::i("a    => %d", result->a);
		Log::i("b    => %d", result->b);
		Log::i("c    => %d", result->c);
		count++;
	}

	CE_ASSERT(count == 10, "Something ha gone wrong");

	return 0;
}