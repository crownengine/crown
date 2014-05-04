// #include <cstdlib>
// #include <ctime>

// #include "Crown.h"
// #include "EventBuffer.h"
// #include "OsTypes.h"
// #include "Mouse.h"
// #include "OsThread.h"
// #include "Log.h"

// using namespace crown;

// EventBuffer* g_write;
// EventBuffer* g_read;

// OsThread g_thread("consumer-thread");
// Semaphore g_write_sem;
// Semaphore g_read_sem;

// bool g_exit = false;

// //-----------------------------------------------------------------------------
// void push_event(int32_t x, int32_t y)
// {
// 	OsMouseEvent ome;
// 	ome.button = MouseButton::LEFT;
// 	ome.x = x;
// 	ome.y = y;
// 	ome.pressed = true;

// 	g_write->push_event((uint32_t)OsEvent::MOUSE, &ome, sizeof(OsMouseEvent));
// 	CE_LOGI("Event pushed");
// }

// //-----------------------------------------------------------------------------
// void swap()
// {
// 	EventBuffer* tmp;

// 	tmp = g_write;
// 	g_write = g_read;
// 	g_read = tmp;

// 	CE_LOGI("Buffers swapped");
// }

// //-----------------------------------------------------------------------------
// int32_t thread_proc(void* /*user_data*/)
// {
// 	static uint32_t count = 0;

// 	OsMouseEvent* result;
// 	uint32_t et; size_t es;

// 	while (true)
// 	{
// 		CE_LOGI("%p", g_read);
		
// 		do
// 		{
// 			result = (OsMouseEvent*)g_read->get_next_event(et, es);

// 			if (result != NULL)
// 			{
// 				CE_LOGD("x: %d, y: %d", result->x, result->y);
// 			}
// 		}
// 		while (result != NULL);

// 		g_exit = ++count == 3 ? true : false;

// 		g_read->clear();

// 		g_write_sem.post();
// 		g_read_sem.wait();
// 	}

// 	return 0;
// }

// //-----------------------------------------------------------------------------
// int main()
// {
// 	memory::init();

// 	g_write = CE_NEW(default_allocator(), EventBuffer);
// 	g_read = CE_NEW(default_allocator(), EventBuffer);

// 	g_thread.start(thread_proc);

// 	while (!g_exit)
// 	{
// 		g_write_sem.wait();

// 		push_event(10, 10);
// 		push_event(20, 20);
// 		push_event(30, 30);

// 		swap();

// 		g_read_sem.post();
// 	}

// 	g_thread.stop();

// 	memory::shutdown();

// 	return 0;
// }

int main() {}