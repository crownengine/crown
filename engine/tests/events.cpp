#include <cstdlib>
#include <ctime>

#include "Crown.h"
#include "EventBuffer.h"
#include "OsTypes.h"
#include "Mouse.h"
#include "OsThread.h"
#include "Log.h"

using namespace crown;

EventBuffer* g_buffer_1;
EventBuffer* g_buffer_2;
EventBuffer* g_tmp;

OsThread g_thread("consumer-thread");
Semaphore g_process_sem;
Semaphore g_thread_sem;

bool g_exit = false;

//-----------------------------------------------------------------------------
void push_event()
{
	uint32_t x = (rand() % 100) + 1;
	uint32_t y = (rand() % 100) + 1;

	OsMouseEvent ome;
	ome.button = MouseButton::LEFT;
	ome.x = x;
	ome.y = y;
	ome.pressed = true;

	g_buffer_1->push_event((uint32_t)OsEvent::MOUSE, &ome, sizeof(OsMouseEvent));
	Log::i("Event pushed");
}

//-----------------------------------------------------------------------------
void swap()
{
	g_tmp = g_buffer_1;
	g_buffer_1 = g_buffer_2;
	g_buffer_2 = g_tmp;
	Log::i("Buffers swapped");
}

//-----------------------------------------------------------------------------
int32_t thread_proc(void* /*user_data*/)
{
	OsMouseEvent* result;
	uint32_t et; size_t es;

	while (true)
	{
		while (!g_buffer_1->is_empty())
		{
			result = (OsMouseEvent*)g_buffer_1->get_next_event(et, es);

			if (result != NULL)
			{
				Log::d("x: %d, y: %d", result->x, result->y);
			}
			else
			{
				g_buffer_1->clear();

				swap();

				g_process_sem.post();
				g_thread_sem.wait();
			}
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------
int main()
{
	memory::init();
	srand(time(NULL));

	g_buffer_1 = CE_NEW(default_allocator(), EventBuffer);
	g_buffer_2 = CE_NEW(default_allocator(), EventBuffer);

	g_thread.start(thread_proc);

	while (!g_exit)
	{
		push_event();

		g_thread_sem.post();
		g_process_sem.wait();
	}

	g_thread.stop();

	memory::shutdown();

	return 0;
}