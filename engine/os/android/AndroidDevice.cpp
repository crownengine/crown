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

#include <jni.h>
#include "Allocator.h"
#include "Device.h"
#include "Log.h"
#include "OsEventQueue.h"
#include "Renderer.h"
#include "Touch.h"
#include "OsWindow.h"

namespace crown
{

class AndroidDevice : public Device
{
public:

	//-----------------------------------------------------------------------------
	AndroidDevice()
		: m_game_thread("game_thread")
	{
		#if defined(CROWN_DEBUG) || defined(CROWN_DEVELOPMENT)
			m_fileserver = 1;
		#endif
	}

	//-----------------------------------------------------------------------------
	int32_t run(int, char**)
	{
		m_game_thread.start(main_loop, (void*)this);

		//while (true) {}
		return 0;
	}

	//-----------------------------------------------------------------------------
	int32_t loop()
	{
		Device::init();

		while (is_running() && !process_events())
		{
			Device::frame();
			m_touch->update();
			m_keyboard->update();
		}

		Device::shutdown();
		return 0;
	}

	//-----------------------------------------------------------------------------
	static int32_t main_loop(void* thiz)
	{
		return ((AndroidDevice*) thiz)->loop();
	}

	//-----------------------------------------------------------------------------
	bool process_events()
	{
		OsEvent event;

		while (m_queue.pop_event(event))
		{
			if (event.type == OsEvent::NONE) continue;

			switch (event.type)
			{
				case OsEvent::TOUCH:
				{
					const OsTouchEvent& ev = event.touch;
					switch (ev.type)
					{
						case OsTouchEvent::POINTER: m_touch->set_pointer_state(ev.x, ev.y, ev.pointer_id, ev.pressed); break;
						case OsTouchEvent::MOVE: m_touch->set_position(ev.pointer_id, ev.x, ev.y); break;
						default: CE_FATAL("Oops, unknown touch event type"); break;
					}

					break;
				}
				case OsEvent::KEYBOARD:
				{
					const OsKeyboardEvent& ev = event.keyboard;
					m_keyboard->set_button_state(ev.button, ev.pressed);
					Log::i("KEYBOARD EVENT RECEIVED");
					break;
				}
				case OsEvent::METRICS:
				{
					const OsMetricsEvent& ev = event.metrics;
					m_window->m_x = 0;
					m_window->m_y = 0;
					m_window->m_width = ev.width;
					m_window->m_height = ev.height;
					Log::i("METRICS EVENT RECEIVED");
					break;
				}
				case OsEvent::EXIT:
				{
					Log::i("EXIT EVENT RECEIVED");
					return true;
				}
				case OsEvent::PAUSE:
				{
					Log::i("PAUSE EVENT RECEIVED, pausing...");
					pause();
					break;
				}
				case OsEvent::RESUME:
				{
					Log::i("RESUME EVENT RECEIVED, resuming...");
					unpause();
					break;
				}
				default:
				{
					CE_FATAL("Unknown Os Event");
					break;
				}
			}
		}

		return false;
	}

	//-----------------------------------------------------------------------------
	void push_keyboard_event(uint32_t modifier, KeyboardButton::Enum b, bool pressed)
	{
		m_queue.push_keyboard_event(modifier, b, pressed);
	}

	//-----------------------------------------------------------------------------
	void push_touch_event(uint16_t x, uint16_t y, uint8_t pointer_id)
	{
		m_queue.push_touch_event(x, y, pointer_id);
	}

	//-----------------------------------------------------------------------------
	void push_touch_event(uint16_t x, uint16_t y, uint8_t pointer_id, bool pressed)
	{
		m_queue.push_touch_event(x, y, pointer_id, pressed);
	}

	void push_metrics_event(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
	{
		m_queue.push_metrics_event(x, y, width, height);
	}

	//-----------------------------------------------------------------------------
	void push_pause_event()
	{
		m_queue.push_pause_event();
	}

	//-----------------------------------------------------------------------------
	void push_resume_event()
	{
		m_queue.push_resume_event();
	}

	//-----------------------------------------------------------------------------
	void push_exit_event(int32_t code)
	{
		m_queue.push_exit_event(code);
	}

private:

	OsEventQueue m_queue;
	OsThread m_game_thread;
};

static AndroidDevice* g_engine;

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT void JNICALL Java_crown_android_CrownLib_initCrown(JNIEnv* /*env*/, jobject /*obj*/)
{
	memory::init();
	os::init_os();

	g_engine = CE_NEW(default_allocator(), AndroidDevice);
	set_device(g_engine);
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT void JNICALL Java_crown_android_CrownLib_shutdownCrown(JNIEnv* /*env*/, jobject /*obj*/)
{
	CE_DELETE(default_allocator(), g_engine);
	memory::shutdown();
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT void JNICALL Java_crown_android_CrownLib_run(JNIEnv* /*env*/, jobject /*obj*/)
{
	g_engine->run(0, NULL);
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT void JNICALL Java_crown_android_CrownLib_pushKeyboardEvent(JNIEnv * /*env*/, jobject /*obj*/, jint modifier, jint b, jint pressed)
{
	g_engine->push_keyboard_event(modifier, (KeyboardButton::Enum) b, pressed);
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT void JNICALL Java_crown_android_CrownLib_pushTouchEventMove(JNIEnv * /*env*/, jobject /*obj*/, jint pointer_id, jint x, jint y)
{
	g_engine->push_touch_event(x, y, pointer_id);
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT void JNICALL Java_crown_android_CrownLib_pushTouchEventPointer(JNIEnv * /*env*/, jobject /*obj*/, jint pointer_id, jint x, jint y, jint pressed)
{
	g_engine->push_touch_event(x, y, pointer_id, pressed);
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT void JNICALL Java_crown_android_CrownLib_pushAccelerometerEvent(JNIEnv * /*env*/, jobject /*obj*/, jint type, jfloat x, jfloat y, jfloat z)
{
}

extern "C" JNIEXPORT void JNICALL Java_crown_android_CrownLib_pushMetricsEvent(JNIEnv * /*env*/, jobject /*obj*/, jint x, jint y, jint width, jint height)
{
	g_engine->push_metrics_event(x, y, width, height);
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT void JNICALL Java_crown_android_CrownLib_pushPauseEvent(JNIEnv * /*env*/, jobject /*obj*/)
{
	g_engine->push_pause_event();
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT void JNICALL Java_crown_android_CrownLib_pushResumeEvent(JNIEnv * /*env*/, jobject /*obj*/)
{
	g_engine->push_resume_event();
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT void JNICALL Java_crown_android_CrownLib_pushExitEvent(JNIEnv * /*env*/, jobject /*obj*/, jint code)
{
	g_engine->push_exit_event(code);
}

} // namespace crown
