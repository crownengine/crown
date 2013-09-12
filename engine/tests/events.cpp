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
void fill_events(EventBuffer& event_buffer, uint32_t num)
{
	TestEvent event;
	event.a = 1;
	event.b = 2;
	event.c = 3;

	for (uint32_t i = 0; i < num; i++)
	{
		event_buffer.push_event(0, &event, sizeof(TestEvent));
	}

	uint32_t size = num * (sizeof(TestEvent) + sizeof(uint32_t) + sizeof(size_t));

	CE_ASSERT(event_buffer.size() == size, "Something ha gone wrong, size is %d, should be %d", event_buffer.size(), size);
}

void get_events(EventBuffer& event_buffer, uint32_t num)
{
	TestEvent* result;
	uint32_t type = 0;
	size_t size = 0;

	uint32_t count = 0;

	for (uint32_t i = 0; i < num; i++)
	{
		result = (TestEvent*)event_buffer.get_next_event(type, size);

		if (result)
		{
			count++;
		}

		Log::i("count: %d", count);
	}

	CE_ASSERT(count == num, "Something ha gone wrong, count: %d, num: %d", count, num);
}

void get_event_until_end(EventBuffer& event_buffer)
{
	TestEvent* result;
	uint32_t type = 0;
	size_t size = 0;

	int32_t count = 0;

	while ((result = (TestEvent*)event_buffer.get_next_event(type, size)) != NULL)
	{
		count++;
	}

	Log::i("count: %d", count);
}

//-----------------------------------------------------------------------------
int main()
{
	EventBuffer event_buffer;

	fill_events(event_buffer, 10);
	get_events(event_buffer, 10);

	event_buffer.clear();

	fill_events(event_buffer, 5);
	get_events(event_buffer, 5);

	event_buffer.clear();

	fill_events(event_buffer, 30);
	get_event_until_end(event_buffer);

	return 0;
}