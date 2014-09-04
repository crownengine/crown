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

#include "config.h"

#if CROWN_PLATFORM_ANDROID

#include "os_event_queue.h"
#include "os_window_android.h"
#include "thread.h"
#include "main.h"
#include "apk_filesystem.h"
#include "crown.h"
#include <jni.h>
#include <android/sensor.h>
#include <android_native_app_glue.h>
#include <bgfxplatform.h>

extern "C"
{
#include <android_native_app_glue.c>
}

namespace crown
{

// //-----------------------------------------------------------------------------
// void display_modes(Array<DisplayMode>& /*modes*/)
// {
// 	// Do nothing
// }

// //-----------------------------------------------------------------------------
// void set_display_mode(uint32_t /*id*/)
// {
// 	// Do nothing
// }

// //-----------------------------------------------------------------------------
// void set_fullscreen(bool /*full*/)
// {
// 	// Do nothing
// }

static bool s_exit = false;

struct MainThreadArgs
{
	Filesystem* fs;
	ConfigSettings* cs;
};

int32_t func(void* data)
{
	MainThreadArgs* args = (MainThreadArgs*) data;
	crown::init(*args->fs, *args->cs);
	crown::update();
	crown::shutdown();
	s_exit = true;
	return EXIT_SUCCESS;
}

struct AndroidDevice
{
	void run(struct android_app* app, Filesystem& fs, ConfigSettings& cs)
	{
		_margs.fs = &fs;
		_margs.cs = &cs;

		app->userData = this;
		app->onAppCmd = crown::AndroidDevice::on_app_cmd;
		app->onInputEvent = crown::AndroidDevice::on_input_event;

		while (app->destroyRequested == 0)
		{
			int32_t num;
			android_poll_source* source;
			/*int32_t id =*/ ALooper_pollAll(-1, NULL, &num, (void**)&source);

			if (NULL != source)
			{
				source->process(app, source);
			}
		}

		_main_thread.stop();
	}

	//-----------------------------------------------------------------------------
	void process_command(struct android_app* app, int32_t cmd)
	{
		switch (cmd)
		{
			case APP_CMD_SAVE_STATE:
			{
				break;
			}
			case APP_CMD_INIT_WINDOW:
			{
				CE_ASSERT(app->window != NULL, "Android window is NULL");
				bgfx::androidSetWindow(app->window);
				// Push metrics here since Android does not trigger APP_CMD_WINDOW_RESIZED
				const int32_t width = ANativeWindow_getWidth(app->window);
				const int32_t height = ANativeWindow_getHeight(app->window);
				_queue.push_metrics_event(0, 0, width, height);
				_main_thread.start(func, &_margs);
				break;
			}
			case APP_CMD_TERM_WINDOW:
			{
				// The window is being hidden or closed, clean it up.
				break;
			}
			case APP_CMD_WINDOW_RESIZED:
			{
				// Not triggered by Android
				break;
			}
			case APP_CMD_GAINED_FOCUS:
			{
				break;
			}
			case APP_CMD_LOST_FOCUS:
			{
				break;
			}
			case APP_CMD_DESTROY:
			{
				_queue.push_exit_event(0);
				break;
			}
		}
	}

	//-----------------------------------------------------------------------------
	int32_t process_input(struct android_app* app, AInputEvent* event)
	{
		if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
		{
			const int32_t action = AMotionEvent_getAction(event);
			const int32_t pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
			const int32_t pointerCount = AMotionEvent_getPointerCount(event);

			const int32_t pointerId = AMotionEvent_getPointerId(event, pointerIndex);
			const float x = AMotionEvent_getX(event, pointerIndex);
			const float y = AMotionEvent_getY(event, pointerIndex);

			const int32_t actionMasked = (action & AMOTION_EVENT_ACTION_MASK);

			switch (actionMasked)
			{
				case AMOTION_EVENT_ACTION_DOWN:
				case AMOTION_EVENT_ACTION_POINTER_DOWN:
				{
					_queue.push_touch_event((uint16_t) x, (uint16_t) y, (uint8_t) pointerId, true);
					break;
				}
				case AMOTION_EVENT_ACTION_UP:
				case AMOTION_EVENT_ACTION_POINTER_UP:
				{
					_queue.push_touch_event((uint16_t) x, (uint16_t) y, (uint8_t) pointerId, false);
					break;
				}
				case AMOTION_EVENT_ACTION_OUTSIDE:
				case AMOTION_EVENT_ACTION_CANCEL:
				{
					_queue.push_touch_event((uint16_t) x, (uint16_t) y, (uint8_t) pointerId, false);
					break;
				}
				case AMOTION_EVENT_ACTION_MOVE:
				{
					for (int index = 0; index < pointerCount; index++)
					{
						const float xx = AMotionEvent_getX(event, index);
						const float yy = AMotionEvent_getY(event, index);
						const int32_t id = AMotionEvent_getPointerId(event, index);
						_queue.push_touch_event((uint16_t) xx, (uint16_t) yy, (uint8_t) id);
					}
					break;
				}
			}

			return 1;
		}
		else if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY)
		{
			const int32_t keycode = AKeyEvent_getKeyCode(event);
			const int32_t keyaction = AKeyEvent_getAction(event);

			if (keycode == AKEYCODE_BACK)
			{
				_queue.push_keyboard_event(0, KeyboardButton::ESCAPE, keyaction == AKEY_EVENT_ACTION_DOWN ? true : false);
			}

			return 1;
		}

		return 0;
	}

	//-----------------------------------------------------------------------------
	static int32_t on_input_event(struct android_app* app, AInputEvent* event)
	{
		return ((AndroidDevice*) app->userData)->process_input(app, event);
	}

	//-----------------------------------------------------------------------------
	static void on_app_cmd(struct android_app* app, int32_t cmd)
	{
		((AndroidDevice*) app->userData)->process_command(app, cmd);
	}

public:

	OsEventQueue _queue;
	Thread _main_thread;
	MainThreadArgs _margs;
};

static AndroidDevice s_advc;

bool next_event(OsEvent& ev)
{
	return s_advc._queue.pop_event(ev);
}

} // namespace crown

void android_main(struct android_app* app)
{
	using namespace crown;

	// Make sure glue isn't stripped.
	app_dummy();

	memory_globals::init();
	// DiskFilesystem src_fs(cls.source_dir);
	ApkFilesystem src_fs(app->activity->assetManager);
	ConfigSettings cs = parse_config_file(src_fs);

	console_server_globals::init();
	console_server_globals::console().init(cs.console_port, false);

	crown::s_advc.run(app, src_fs, cs);

	console_server_globals::shutdown();
	memory_globals::shutdown();
}

#endif // CROWN_PLATFORM_ANDROID
