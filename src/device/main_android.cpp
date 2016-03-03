/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "config.h"

#if CROWN_PLATFORM_ANDROID

#include "device.h"
#include "os_event_queue.h"
#include "thread.h"
#include <stdlib.h>
#include <jni.h>
#include <android/sensor.h>
#include <android_native_app_glue.h>
#include <bgfx/bgfxplatform.h>

extern "C"
{
#include <android_native_app_glue.c>
}

namespace crown
{
static bool s_exit = false;

struct MainThreadArgs
{
	DeviceOptions* opts;
};

s32 func(void* data)
{
	MainThreadArgs* args = (MainThreadArgs*)data;
	crown::init(*args->opts);
	crown::shutdown();
	s_exit = true;
	return EXIT_SUCCESS;
}

struct AndroidDevice
{
	void run(struct android_app* app, DeviceOptions& opts)
	{
		_margs.opts = &opts;

		app->userData = this;
		app->onAppCmd = crown::AndroidDevice::on_app_cmd;
		app->onInputEvent = crown::AndroidDevice::on_input_event;

		while (app->destroyRequested == 0)
		{
			s32 num;
			android_poll_source* source;
			/*s32 id =*/ ALooper_pollAll(-1, NULL, &num, (void**)&source);

			if (NULL != source)
			{
				source->process(app, source);
			}
		}

		_main_thread.stop();
	}

	void process_command(struct android_app* app, s32 cmd)
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
				const s32 width = ANativeWindow_getWidth(app->window);
				const s32 height = ANativeWindow_getHeight(app->window);
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

	s32 process_input(struct android_app* app, AInputEvent* event)
	{
		if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
		{
			const s32 action = AMotionEvent_getAction(event);
			const s32 pointer_index = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
			const s32 pointer_count = AMotionEvent_getPointerCount(event);

			const s32 pointer_id = AMotionEvent_getPointerId(event, pointer_index);
			const f32 x = AMotionEvent_getX(event, pointer_index);
			const f32 y = AMotionEvent_getY(event, pointer_index);

			const s32 actionMasked = (action & AMOTION_EVENT_ACTION_MASK);

			switch (actionMasked)
			{
				case AMOTION_EVENT_ACTION_DOWN:
				case AMOTION_EVENT_ACTION_POINTER_DOWN:
				{
					_queue.push_touch_event((s16)x, (s16)y, (u8)pointer_id, true);
					break;
				}
				case AMOTION_EVENT_ACTION_UP:
				case AMOTION_EVENT_ACTION_POINTER_UP:
				{
					_queue.push_touch_event((s16)x, (s16)y, (u8)pointer_id, false);
					break;
				}
				case AMOTION_EVENT_ACTION_OUTSIDE:
				case AMOTION_EVENT_ACTION_CANCEL:
				{
					_queue.push_touch_event((s16)x, (s16)y, (u8)pointer_id, false);
					break;
				}
				case AMOTION_EVENT_ACTION_MOVE:
				{
					for (int index = 0; index < pointer_count; index++)
					{
						const f32 xx = AMotionEvent_getX(event, index);
						const f32 yy = AMotionEvent_getY(event, index);
						const s32 id = AMotionEvent_getPointerId(event, index);
						_queue.push_touch_event((s16)xx, (s16)yy, (u8)id);
					}
					break;
				}
			}

			return 1;
		}
		else if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY)
		{
			const s32 keycode = AKeyEvent_getKeyCode(event);
			const s32 keyaction = AKeyEvent_getAction(event);

			if (keycode == AKEYCODE_BACK)
			{
				_queue.push_keyboard_event(KeyboardButton::ESCAPE
					, keyaction == AKEY_EVENT_ACTION_DOWN ? true : false);
			}

			return 1;
		}

		return 0;
	}

	static s32 on_input_event(struct android_app* app, AInputEvent* event)
	{
		return ((AndroidDevice*) app->userData)->process_input(app, event);
	}

	static void on_app_cmd(struct android_app* app, s32 cmd)
	{
		((AndroidDevice*) app->userData)->process_command(app, cmd);
	}

public:

	OsEventQueue _queue;
	Thread _main_thread;
	MainThreadArgs _margs;
};

struct WindowAndroid : public Window
{
	WindowAndroid()
	{
	}

	void open(u16 /*x*/, u16 /*y*/, u16 /*width*/, u16 /*height*/, u32 /*parent*/)
	{
	}

	void close()
	{
	}

	void bgfx_setup()
	{
	}

	void show()
	{
	}

	void hide()
	{
	}

	void resize(u16 /*width*/, u16 /*height*/)
	{
	}

	void move(u16 /*x*/, u16 /*y*/)
	{
	}

	void minimize()
	{
	}

	void restore()
	{
	}

	const char* title()
	{
		return NULL;
	}

	void set_title (const char* /*title*/)
	{
	}

	void show_cursor(bool /*show*/)
	{
	}

	void* handle()
	{
		return NULL;
	}
};

namespace window
{
	Window* create(Allocator& a)
	{
		return CE_NEW(a, WindowAndroid)();
	}

	void destroy(Allocator& a, Window& w)
	{
		CE_DELETE(a, &w);
	}
} // namespace window

struct DisplayAndroid : public Display
{
	void modes(Array<DisplayMode>& /*modes*/)
	{
	}

	void set_mode(u32 /*id*/)
	{
	}
};

namespace display
{
	Display* create(Allocator& a)
	{
		return CE_NEW(a, DisplayAndroid)();
	}

	void destroy(Allocator& a, Display& d)
	{
		CE_DELETE(a, &d);
	}
} // namespace display

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

	DeviceOptions opts(0, NULL);
	opts._asset_manager = app->activity->assetManager;

	crown::s_advc.run(app, opts);
	memory_globals::shutdown();
}

#endif // CROWN_PLATFORM_ANDROID
