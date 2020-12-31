/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "config.h"

#if CROWN_PLATFORM_ANDROID

#include "core/error/error.inl"
#include "core/guid.h"
#include "core/memory/globals.h"
#include "core/memory/memory.inl"
#include "core/thread/thread.h"
#include "device/device.h"
#include "device/device_event_queue.inl"
#include <android/sensor.h>
#include <android/window.h>
#include <bgfx/platform.h>
#include <jni.h>
#include <stdlib.h>

extern "C"
{
#include <android_native_app_glue.c>
}

namespace crown
{
struct AndroidDevice
{
	DeviceEventQueue _queue;
	Thread _main_thread;
	DeviceOptions* _opts;

	void run(struct android_app* app, DeviceOptions& opts)
	{
		_opts = &opts;

		app->userData = this;
		app->onAppCmd = crown::AndroidDevice::on_app_cmd;
		app->onInputEvent = crown::AndroidDevice::on_input_event;
		ANativeActivity_setWindowFlags(app->activity
			, AWINDOW_FLAG_FULLSCREEN | AWINDOW_FLAG_KEEP_SCREEN_ON
			, 0
			);

		while (app->destroyRequested == 0)
		{
			s32 num;
			android_poll_source* source;
			ALooper_pollAll(-1, NULL, &num, (void**)&source);

			if (source != NULL)
				source->process(app, source);
		}

		_main_thread.stop();
	}

	void process_command(struct android_app* app, s32 cmd)
	{
		switch (cmd)
		{
		case APP_CMD_SAVE_STATE:
			break;

		case APP_CMD_INIT_WINDOW:
			{
				CE_ASSERT(app->window != NULL, "Android window is NULL");

				bgfx::PlatformData pd;
				pd.ndt          = NULL;
				pd.nwh          = app->window;
				pd.context      = NULL;
				pd.backBuffer   = NULL;
				pd.backBufferDS = NULL;
				bgfx::setPlatformData(pd);

				// Push metrics here since Android does not trigger APP_CMD_WINDOW_RESIZED
				const s32 width  = ANativeWindow_getWidth(app->window);
				const s32 height = ANativeWindow_getHeight(app->window);
				_queue.push_resolution_event(width, height);

				if (!_main_thread.is_running())
				{
					_main_thread.start([](void* user_data) {
							crown::run(*((DeviceOptions*)user_data));
							return EXIT_SUCCESS;
						}
						, _opts
						);
				}
			}
			break;

		case APP_CMD_TERM_WINDOW:
			// The window is being hidden or closed, clean it up.
			break;

		case APP_CMD_WINDOW_RESIZED:
			// Not triggered by Android
			break;

		case APP_CMD_GAINED_FOCUS:
			break;

		case APP_CMD_LOST_FOCUS:
			break;

		case APP_CMD_DESTROY:
			_queue.push_exit_event();
			break;
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
				_queue.push_button_event(InputDeviceType::TOUCHSCREEN
					, 0
					, pointer_id
					, true
					);
				break;

			case AMOTION_EVENT_ACTION_UP:
			case AMOTION_EVENT_ACTION_POINTER_UP:
				_queue.push_button_event(InputDeviceType::TOUCHSCREEN
					, 0
					, pointer_id
					, false
					);
				break;

			case AMOTION_EVENT_ACTION_OUTSIDE:
			case AMOTION_EVENT_ACTION_CANCEL:
				_queue.push_button_event(InputDeviceType::TOUCHSCREEN
					, 0
					, pointer_id
					, false
					);
				break;

			case AMOTION_EVENT_ACTION_MOVE:
				for (int index = 0; index < pointer_count; index++)
				{
					const f32 xx = AMotionEvent_getX(event, index);
					const f32 yy = AMotionEvent_getY(event, index);
					const s32 id = AMotionEvent_getPointerId(event, index);
					_queue.push_axis_event(InputDeviceType::TOUCHSCREEN
						, 0
						, id
						, xx
						, yy
						, 0
						);
				}
				break;
			}

			return 1;
		}
		else if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY)
		{
			const s32 keycode   = AKeyEvent_getKeyCode(event);
			const s32 keyaction = AKeyEvent_getAction(event);

			if (keycode == AKEYCODE_BACK)
			{
				_queue.push_button_event(InputDeviceType::KEYBOARD
					, 0
					, KeyboardButton::ESCAPE
					, keyaction == AKEY_EVENT_ACTION_DOWN ? true : false
					);
			}

			return 1;
		}

		return 0;
	}

	static s32 on_input_event(struct android_app* app, AInputEvent* event)
	{
		return static_cast<AndroidDevice*>(app->userData)->process_input(app, event);
	}

	static void on_app_cmd(struct android_app* app, s32 cmd)
	{
		static_cast<AndroidDevice*>(app->userData)->process_command(app, cmd);
	}
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

	void maximize()
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

	void set_fullscreen(bool /*fullscreen*/)
	{
	}

	void set_cursor(MouseCursor::Enum /*cursor*/)
	{
	}

	void set_cursor_mode(CursorMode::Enum /*mode*/)
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

	memory_globals::init();
	guid_globals::init();

	DeviceOptions opts(default_allocator(), 0, NULL);
	opts._asset_manager = app->activity->assetManager;

	crown::s_advc.run(app, opts);
	guid_globals::shutdown();
	memory_globals::shutdown();
}

#endif // CROWN_PLATFORM_ANDROID
