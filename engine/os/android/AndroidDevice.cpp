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
#include <android/sensor.h>
#include <android_native_app_glue.h>
#include "Allocator.h"
#include "Device.h"
#include "Log.h"
#include "OsEventQueue.h"
#include "Touch.h"
#include "OsWindow.h"
#include "OsThread.h"
#include <bgfxplatform.h>

extern "C"
{
#include <android_native_app_glue.c>
}

namespace crown
{

ANativeWindow* g_android_window;
AAssetManager* g_android_asset_manager;

class AndroidDevice : public Device
{
public:

	//-----------------------------------------------------------------------------
	AndroidDevice()
		: m_game_thread("game_thread")
	{
		#if defined(CROWN_DEBUG)
			m_fileserver = 1;
		#endif
	}

	//-----------------------------------------------------------------------------
	void display_modes(Array<DisplayMode>& /*modes*/)
	{
		// Do nothing
	}

	//-----------------------------------------------------------------------------
	void set_display_mode(uint32_t /*id*/)
	{
		// Do nothing
	}

	//-----------------------------------------------------------------------------
	void set_fullscreen(bool /*full*/)
	{
		// Do nothing
	}

	void run(struct android_app* app)
	{
		app->userData = this;
		app->onAppCmd = crown::AndroidDevice::on_app_cmd;
		app->onInputEvent = crown::AndroidDevice::on_input_event;
		g_android_asset_manager = app->activity->assetManager;

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

		m_game_thread.stop();
	}

	//-----------------------------------------------------------------------------
	int32_t run(int, char**)
	{
		return 0;
	}

	//-----------------------------------------------------------------------------
	int32_t loop()
	{
		#if defined(CROWN_DEBUG)
			m_console = CE_NEW(default_allocator(), ConsoleServer)();
			m_console->init(m_console_port, false);
		#endif

		Device::init();

		// Push metrics here since Android does not trigger APP_CMD_WINDOW_RESIZED
		const int32_t width = ANativeWindow_getWidth(g_android_window);
		const int32_t height = ANativeWindow_getHeight(g_android_window);
		m_queue.push_metrics_event(0, 0, width, height);

		while (is_running() && !process_events())
		{
			#if defined(CROWN_DEBUG)
				m_console->update();
			#endif

			Device::frame();
			m_touch->update();
			m_keyboard->update();
		}

		Device::shutdown();

		#if defined(CROWN_DEBUG)
			m_console->shutdown();
			CE_DELETE(default_allocator(), m_console);
		#endif

		exit(EXIT_SUCCESS);
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
					break;
				}
				case OsEvent::METRICS:
				{
					const OsMetricsEvent& ev = event.metrics;
					m_window->m_x = ev.x;
					m_window->m_y = ev.y;
					m_window->m_width = ev.width;
					m_window->m_height = ev.height;
					break;
				}
				case OsEvent::EXIT:
				{
					return true;
				}
				case OsEvent::PAUSE:
				{
					pause();
					break;
				}
				case OsEvent::RESUME:
				{
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
	static void on_app_cmd(struct android_app* app, int32_t cmd)
	{
		((AndroidDevice*) app->userData)->process_command(app, cmd);
	}

	//-----------------------------------------------------------------------------
	void process_command(struct android_app* app, int32_t cmd)
	{
		switch (cmd)
		{
			case APP_CMD_SAVE_STATE:
			{
				// // The system has asked us to save our current state.  Do so.
				// engine->app->savedState = malloc(sizeof(struct saved_state));
				// *((struct saved_state*)engine->app->savedState) = engine->state;
				// engine->app->savedStateSize = sizeof(struct saved_state);
				break;
			}
			case APP_CMD_INIT_WINDOW:
			{
				CE_ASSERT(app->window != NULL, "Android window is NULL");
				g_android_window = app->window;
				bgfx::androidSetWindow(app->window);
				m_game_thread.start(main_loop, (void*)this);
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
				m_queue.push_exit_event(0);
				break;
			}
		}
	}

	//-----------------------------------------------------------------------------
	static int32_t on_input_event(struct android_app* app, AInputEvent* event)
	{
		return ((AndroidDevice*) app->userData)->process_input(app, event);
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
					m_queue.push_touch_event((uint16_t) x, (uint16_t) y, (uint8_t) pointerId, true);
					break;			
				}
				case AMOTION_EVENT_ACTION_UP:
				case AMOTION_EVENT_ACTION_POINTER_UP:
				{
					m_queue.push_touch_event((uint16_t) x, (uint16_t) y, (uint8_t) pointerId, false);
					break;
				}
				case AMOTION_EVENT_ACTION_OUTSIDE:
				case AMOTION_EVENT_ACTION_CANCEL:
				{
					m_queue.push_touch_event((uint16_t) x, (uint16_t) y, (uint8_t) pointerId, false);
					break;			
				}
				case AMOTION_EVENT_ACTION_MOVE:
				{
					for (int index = 0; index < pointerCount; index++)
					{
						const float xx = AMotionEvent_getX(event, index);
						const float yy = AMotionEvent_getY(event, index);
						const int32_t id = AMotionEvent_getPointerId(event, index);
						m_queue.push_touch_event((uint16_t) xx, (uint16_t) yy, (uint8_t) id);
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
				m_queue.push_keyboard_event(0, KeyboardButton::ESCAPE, keyaction == AKEY_EVENT_ACTION_DOWN ? true : false);
			}

			return 1;
		}

		return 0;
	}

private:

	OsEventQueue m_queue;
	OsThread m_game_thread;
};

} // namespace crown

void android_main(struct android_app* app)
{
	// Make sure glue isn't stripped.
	app_dummy();

	crown::memory::init();
	crown::os::init_os();
	crown::AndroidDevice* engine = CE_NEW(crown::default_allocator(), crown::AndroidDevice)();
	crown::set_device(engine);

	engine->run(app);

	CE_DELETE(crown::default_allocator(), engine);
	crown::memory::shutdown();
}
