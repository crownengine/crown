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
#include "SoundRenderer.h"
#include "Touch.h"

namespace crown
{

class AndroidDevice : public Device
{
public:

	int32_t run(int, char**)
	{
		process_events();
		Device::frame();
		m_touch->update();
		return 0;
	}

	//-----------------------------------------------------------------------------
	bool process_events()
	{
		OsEvent event;
		do
		{
			m_queue.pop_event(&event);

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
				case OsEvent::EXIT:
				{
					return true;
				}
				default:
				{
					CE_FATAL("Unknown Os Event");
					break;
				}
			}
		}
		while (event.type != OsEvent::NONE);

		return false;
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

private:

	OsEventQueue m_queue;
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
extern "C" JNIEXPORT void JNICALL Java_crown_android_CrownLib_initDevice(JNIEnv* /*env*/, jobject /*obj*/)
{
	device()->init();
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT void JNICALL Java_crown_android_CrownLib_stopDevice(JNIEnv* /*env*/, jobject /*obj*/)
{
	device()->stop();
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT void JNICALL Java_crown_android_CrownLib_shutdownDevice(JNIEnv* /*env*/, jobject /*obj*/)
{
	device()->shutdown();
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT void JNICALL Java_crown_android_CrownLib_pauseDevice(JNIEnv* /*env*/, jobject /*obj*/)
{
	device()->pause();
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT void JNICALL Java_crown_android_CrownLib_unpauseDevice(JNIEnv* /*env*/, jobject /*obj*/)
{
	device()->unpause();
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT bool JNICALL Java_crown_android_CrownLib_isDeviceInit(JNIEnv* /*env*/, jobject /*obj*/)
{
	return device()->is_init();
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT bool JNICALL Java_crown_android_CrownLib_isDeviceRunning(JNIEnv* /*env*/, jobject /*obj*/)
{
	return device()->is_running();
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT bool JNICALL Java_crown_android_CrownLib_isDevicePaused(JNIEnv* /*env*/, jobject /*obj*/)
{
	return device()->is_paused();
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT void JNICALL Java_crown_android_CrownLib_run(JNIEnv* /*env*/, jobject /*obj*/)
{
	g_engine->run(0, NULL);
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT void JNICALL Java_crown_android_CrownLib_initRenderer(JNIEnv* /*env*/, jobject /*obj*/)
{
	device()->renderer()->init();
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT void JNICALL Java_crown_android_CrownLib_shutdownRenderer(JNIEnv* /*env*/, jobject /*obj*/)
{
	device()->renderer()->shutdown();
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT void JNICALL Java_crown_android_CrownLib_pauseSoundRenderer(JNIEnv* /*env*/, jobject /*obj*/)
{
	device()->sound_renderer()->pause();
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT void JNICALL Java_crown_android_CrownLib_unpauseSoundRenderer(JNIEnv* /*env*/, jobject /*obj*/)
{
	device()->sound_renderer()->unpause();
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

} // namespace crown
