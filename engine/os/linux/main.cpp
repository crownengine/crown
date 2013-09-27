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

#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include "Crown.h"
#include "Device.h"
#include "OsTypes.h"
#include "EventQueue.h"
#include "Config.h"
#include "BundleCompiler.h"

namespace crown
{

extern void set_x11_display_and_window(Display* dpy, Window win);

//-----------------------------------------------------------------------------
void init()
{
	crown::memory::init();
	crown::os::init_os();
}

//-----------------------------------------------------------------------------
void shutdown()
{
	crown::memory::shutdown();
}

//-----------------------------------------------------------------------------
static KeyboardButton::Enum x11_translate_key(KeySym x11_key)
{
	if ((x11_key > 0x40 && x11_key < 0x5B) || (x11_key > 0x60 && x11_key < 0x7B) || (x11_key > 0x2F && x11_key < 0x3A))
	{
		return (KeyboardButton::Enum) x11_key;
	}

	switch (x11_key)
	{
		case XK_BackSpace:	return KeyboardButton::BACKSPACE;
		case XK_Tab:		return KeyboardButton::TAB;
		case XK_space:		return KeyboardButton::SPACE;
		case XK_Escape:		return KeyboardButton::ESCAPE;
		case XK_Return:		return KeyboardButton::ENTER;
		case XK_F1:			return KeyboardButton::F1;
		case XK_F2:			return KeyboardButton::F2;
		case XK_F3:			return KeyboardButton::F3;
		case XK_F4:			return KeyboardButton::F4;
		case XK_F5:			return KeyboardButton::F5;
		case XK_F6:			return KeyboardButton::F6;
		case XK_F7:			return KeyboardButton::F7;
		case XK_F8:			return KeyboardButton::F8;
		case XK_F9:			return KeyboardButton::F9;
		case XK_F10:		return KeyboardButton::F10;
		case XK_F11:		return KeyboardButton::F11;
		case XK_F12:		return KeyboardButton::F12;
		case XK_Home:		return KeyboardButton::HOME;
		case XK_Left:		return KeyboardButton::LEFT;
		case XK_Up:			return KeyboardButton::UP;
		case XK_Right:		return KeyboardButton::RIGHT;
		case XK_Down:		return KeyboardButton::DOWN;
		case XK_Page_Up:	return KeyboardButton::PAGE_UP;
		case XK_Page_Down:	return KeyboardButton::PAGE_DOWN;
		case XK_Shift_L:	return KeyboardButton::LSHIFT;
		case XK_Shift_R:	return KeyboardButton::RSHIFT;
		case XK_Control_L:	return KeyboardButton::LCONTROL;
		case XK_Control_R:	return KeyboardButton::RCONTROL;
		case XK_Caps_Lock:	return KeyboardButton::CAPS_LOCK;
		case XK_Alt_L:		return KeyboardButton::LALT;
		case XK_Alt_R:		return KeyboardButton::RALT;
		case XK_Super_L:	return KeyboardButton::LSUPER;
		case XK_Super_R:	return KeyboardButton::RSUPER;
		case XK_KP_0:		return KeyboardButton::KP_0;
		case XK_KP_1:		return KeyboardButton::KP_1;
		case XK_KP_2:		return KeyboardButton::KP_2;
		case XK_KP_3:		return KeyboardButton::KP_3;
		case XK_KP_4:		return KeyboardButton::KP_4;
		case XK_KP_5:		return KeyboardButton::KP_5;
		case XK_KP_6:		return KeyboardButton::KP_6;
		case XK_KP_7:		return KeyboardButton::KP_7;
		case XK_KP_8:		return KeyboardButton::KP_8;
		case XK_KP_9:		return KeyboardButton::KP_9;
		default:			return KeyboardButton::NONE;
	}
}

struct MainArgs
{
	int argc;
	char** argv;
	class LinuxDevice* device;
};

class LinuxDevice : public Device
{
public:

	//-----------------------------------------------------------------------------
	LinuxDevice()
		: m_x11_display(NULL)
		, m_x11_window(None)
		, m_x11_parent_window(None)
		, m_x11_hidden_cursor(None)
		, m_exit(false)
		, m_x(0)
		, m_y(0)
		, m_width(1000)
		, m_height(625)
		, m_parent_window_handle(0)
		, m_fullscreen(0)
		, m_compile(0)
		, m_continue(0)
		, m_alloc(m_event_buffer, 1024 * 4)
		, m_queue(m_alloc)
	{
		for (uint32_t i = 0; i < 1024; i++)
		{
			m_event_buffer[i] = 'a';
		}
	}

	//-----------------------------------------------------------------------------
	void init(int argc, char** argv)
	{
		parse_command_line(argc, argv);
		check_preferred_settings();

		#if defined(CROWN_DEBUG) || defined(CROWN_DEVELOPMENT)
			if (m_compile == 1)
			{
				m_bundle_compiler = CE_NEW(default_allocator(), BundleCompiler);
				if (!m_bundle_compiler->compile(m_bundle_dir, m_source_dir))
				{
					CE_DELETE(default_allocator(), m_bundle_compiler);
					Log::e("Exiting.");
					exit(EXIT_FAILURE);
				}

				if (!m_continue)
				{
					CE_DELETE(default_allocator(), m_bundle_compiler);
					exit(EXIT_SUCCESS);
				}
			}
		#endif
	}

	//-----------------------------------------------------------------------------
	void shutdown()
	{
		#if defined(CROWN_DEBUG) || defined(CROWN_DEVELOPMENT)
			CE_DELETE(default_allocator(), m_bundle_compiler);
		#endif
	}

	//-----------------------------------------------------------------------------
	int32_t run(int argc, char** argv)
	{
		init(argc, argv);

		XInitThreads();
		CE_ASSERT(m_width != 0 || m_height != 0, "Width and height must differ from zero");

		m_x11_display = XOpenDisplay(NULL);

		CE_ASSERT(m_x11_display != NULL, "Unable to open X11 display");

		int screen = DefaultScreen(m_x11_display);
		int depth = DefaultDepth(m_x11_display, screen);
		Visual* visual = DefaultVisual(m_x11_display, screen);

		if (m_parent_window_handle != 0)
		{
			m_x11_parent_window = (Window) m_parent_window_handle;
		}
		else
		{
			m_x11_parent_window = RootWindow(m_x11_display, screen);
		}

		// We want to track keyboard and mouse events
		XSetWindowAttributes win_attribs;
		win_attribs.background_pixmap = 0;
		win_attribs.border_pixel = 0;
		win_attribs.event_mask = FocusChangeMask | StructureNotifyMask | KeyPressMask | 
			KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask;

		m_x11_window = XCreateWindow(
					   m_x11_display,
					   m_x11_parent_window,
					   0, 0,
					   m_width, m_height,
					   0,
					   depth,
					   InputOutput,
					   visual,
					   CWBorderPixel | CWEventMask,
					   &win_attribs
				   );

		CE_ASSERT(m_x11_window != None, "Unable to create X window");

		// Check presence of detectable autorepeat
		Bool detectable;
		m_x11_detectable_autorepeat = (bool) XkbSetDetectableAutoRepeat(m_x11_display, true, &detectable);

		// Build hidden cursor
		Pixmap bm_no;
		XColor black, dummy;
		Colormap colormap;
		static char no_data[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

		colormap = XDefaultColormap(m_x11_display, screen);
		XAllocNamedColor(m_x11_display, colormap, "black", &black, &dummy);
		bm_no = XCreateBitmapFromData(m_x11_display, m_x11_window, no_data, 8, 8);
		m_x11_hidden_cursor = XCreatePixmapCursor(m_x11_display, bm_no, bm_no, &black, &black, 0, 0);

		m_wm_delete_message = XInternAtom(m_x11_display, "WM_DELETE_WINDOW", False);
		XSetWMProtocols(m_x11_display, m_x11_window, &m_wm_delete_message, 1);

		XMapRaised(m_x11_display, m_x11_window);

		set_x11_display_and_window(m_x11_display, m_x11_window);

		OsThread game_thread("game-thread");
		game_thread.start(main_loop, (void*)this);

		while (!m_exit)
		{
			while (XPending(m_x11_display))
			{
				LinuxDevice::pump_events();
			}
		}

		Log::d("Stopping game thread");

		game_thread.stop();

		XDestroyWindow(m_x11_display, m_x11_window);
		XCloseDisplay(m_x11_display);

		return EXIT_SUCCESS;
	}

	//-----------------------------------------------------------------------------
	int32_t loop()
	{
		Device::init();

		while(!process_events() && is_running())
		{
			Device::frame();
		}

		Device::shutdown();

		m_exit = true;

		return 0;
	}

	//-----------------------------------------------------------------------------
	static int32_t main_loop(void* thiz)
	{
		return ((LinuxDevice*)thiz)->loop();
	}

	//-----------------------------------------------------------------------------
	bool process_events()
	{
		OsEvent* event;
		do
		{
			event = (OsEvent*)m_queue.get_event();

			if (event != NULL)
			{
				switch (event->type)
				{
					case OsEvent::MOUSE:
					{
						m_mouse->set_button_state(event->mouse.button, event->mouse.pressed);
						break;
					}
					case OsEvent::KEYBOARD:
					{
						m_keyboard->set_button_state(event->keyboard.button, event->keyboard.pressed);
						break;
					}
					case OsEvent::EXIT:
					{
						return true;
					}
					default:
					{
						Log::d("Unmanaged");
						//CE_FATAL("Oops, unknown Os event");
						break;
					}
				}
			}
		}
		while (event != 0);

		return false;
	}

	//-----------------------------------------------------------------------------
	void pump_events()
	{
		XEvent event;
		XNextEvent(m_x11_display, &event);

		switch (event.type)
		{
			case ClientMessage:
			{
				if ((Atom)event.xclient.data.l[0] == m_wm_delete_message)
				{
					OsEvent* os_event = new OsEvent;
					os_event->type = OsEvent::EXIT;
					m_queue.push_event(os_event);
				}
				break;
			}
			// case ConfigureNotify:
			// {
			// 	m_x = event.xconfigure.x;
			// 	m_y = event.xconfigure.y;
			// 	m_width = event.xconfigure.width;
			// 	m_height = event.xconfigure.height;
			// 	break;
			// }
			case ButtonPress:
			case ButtonRelease:
			{
				OsEvent* os_event = new OsEvent;
				OsMouseEvent& mouse_event = os_event->mouse;
				os_event->type = OsEvent::MOUSE;

				MouseButton::Enum mb;
				switch (event.xbutton.button)
				{
					case Button1: mb = MouseButton::LEFT; break;
					case Button2: mb = MouseButton::MIDDLE; break;
					case Button3: mb = MouseButton::RIGHT; break;
					default: mb = MouseButton::COUNT; break;
				}

				if (mb != MouseButton::COUNT)
				{
					mouse_event.button = mb;
					mouse_event.x = event.xbutton.x;
					mouse_event.y = event.xbutton.y;
					mouse_event.pressed = event.type == ButtonPress;
					m_queue.push_event(os_event);
				}

				break;
			}
			// case MotionNotify:
			// {
			// 	push_event(OSET_MOTION_NOTIFY, data_button[0], data_button[1], data_button[2], data_button[3]);
			// 	break;
			// }
			case KeyPress:
			case KeyRelease:
			{
				char string[4] = {0, 0, 0, 0};
				KeySym key;

				XLookupString(&event.xkey, string, 4, &key, NULL);

				KeyboardButton::Enum kb = x11_translate_key(key);

				// Check if any modifier key is pressed or released
				int32_t modifier_mask = 0;

				if (kb == KeyboardButton::LSHIFT || kb == KeyboardButton::RSHIFT)
				{
					(event.type == KeyPress) ? modifier_mask |= ModifierButton::SHIFT : modifier_mask &= ~ModifierButton::SHIFT;
				}
				else if (kb == KeyboardButton::LCONTROL || kb == KeyboardButton::RCONTROL)
				{
					(event.type == KeyPress) ? modifier_mask |= ModifierButton::CTRL : modifier_mask &= ~ModifierButton::CTRL;
				}
				else if (kb == KeyboardButton::LALT || kb == KeyboardButton::RALT)
				{
					(event.type == KeyPress) ? modifier_mask |= ModifierButton::ALT : modifier_mask &= ~ModifierButton::ALT;
				}

				OsEvent* os_event = new OsEvent;
				OsKeyboardEvent& keyboard_event = os_event->keyboard;
				os_event->type = OsEvent::KEYBOARD;

				keyboard_event.button = kb;
				keyboard_event.modifier = modifier_mask;
				keyboard_event.pressed = event.type == KeyPress;

				m_queue.push_event(os_event);

	//				// Text input part
	//				if (event.type == KeyPress && len > 0)
	//				{
	//					//crownEvent.event_type = ET_TEXT;
	//					//crownEvent.text.type = TET_TEXT_INPUT;
	//					strncpy(keyboardEvent.text, string, 4);

	//					if (mListener)
	//					{
	//						mListener->TextInput(keyboardEvent);
	//					}
	//				}

				break;
			}
			case KeymapNotify:
			{
				XRefreshKeyboardMapping(&event.xmapping);
				break;
			}
			default:
			{
				break;
			}
		}
	}

	//-----------------------------------------------------------------------------
	void parse_command_line(int argc, char** argv)
	{
		static const char* help_message =
			"Usage: crown [options]\n"
			"Options:\n\n"

			"All of the following options take precedence over\n"
			"environment variables and configuration files.\n\n"

			"  --help                     Show this help.\n"
			"  --bundle-dir <path>        Use <path> as the source directory for compiled resources.\n"
			"  --width <width>            Set the <width> of the main window.\n"
			"  --height <width>           Set the <height> of the main window.\n"
			"  --fullscreen               Start in fullscreen.\n"
			"  --parent-window <handle>   Set the parent window <handle> of the main window.\n"
			"                             Used only by tools.\n"

			"\nAvailable only in debug and development builds:\n\n"

			"  --source-dir <path>        Use <path> as the source directory for resource compilation.\n"
			"  --compile                  Run the engine as resource compiler.\n"
			"  --continue                 Do a full compile of the resources and continue the execution.\n";

		static ArgsOption options[] = 
		{
			{ "help",             AOA_NO_ARGUMENT,       NULL,        'i' },
			{ "source-dir",       AOA_REQUIRED_ARGUMENT, NULL,        's' },
			{ "bundle-dir",       AOA_REQUIRED_ARGUMENT, NULL,        'b' },
			{ "compile",          AOA_NO_ARGUMENT,       &m_compile,   1 },
			{ "continue",         AOA_NO_ARGUMENT,       &m_continue,  1 },
			{ "width",            AOA_REQUIRED_ARGUMENT, NULL,        'w' },
			{ "height",           AOA_REQUIRED_ARGUMENT, NULL,        'h' },
			{ "fullscreen",       AOA_NO_ARGUMENT,       &m_fullscreen, 1 },
			{ "parent-window",    AOA_REQUIRED_ARGUMENT, NULL,        'p' },
			{ NULL, 0, NULL, 0 }
		};

		Args args(argc, argv, "", options);

		int32_t opt;
		while ((opt = args.getopt()) != -1)
		{
			switch (opt)
			{
				case 0:
				{
					break;
				}
				// Source directory
				case 's':
				{
					string::strncpy(m_source_dir, args.optarg(), MAX_PATH_LENGTH);
					break;
				}
				// Bundle directory
				case 'b':
				{
					string::strncpy(m_bundle_dir, args.optarg(), MAX_PATH_LENGTH);
					break;
				}
				// Window width
				case 'w':
				{
					m_width = atoi(args.optarg());
					break;
				}
				// Window height
				case 'h':
				{
					m_height = atoi(args.optarg());
					break;
				}
				// Parent window
				case 'p':
				{
					m_parent_window_handle = string::parse_uint(args.optarg());
					break;
				}
				case 'i':
				case '?':
				default:
				{
					os::printf(help_message);
					exit(EXIT_FAILURE);
				}
			}
		}
	}

	//-----------------------------------------------------------------------------
	void check_preferred_settings()
	{
		if (m_compile == 1)
		{
			if (string::strcmp(m_source_dir, "") == 0)
			{
				Log::e("You have to specify the source directory when running in compile mode.");
				exit(EXIT_FAILURE);
			}

			if (!os::is_absolute_path(m_source_dir))
			{
				Log::e("The source directory must be absolute.");
				exit(EXIT_FAILURE);
			}
		}

		if (!os::is_absolute_path(m_bundle_dir))
		{
			Log::e("The bundle directory must be absolute.");
			exit(EXIT_FAILURE);
		}

		if (m_width == 0 || m_height == 0)
		{
			Log::e("Window width and height must be greater than zero.");
			exit(EXIT_FAILURE);
		}
	}


private:

	Display* m_x11_display;
	Window m_x11_window;
	Window m_x11_parent_window;
	Cursor m_x11_hidden_cursor;
	Atom m_wm_delete_message;

	bool m_exit;
	uint32_t m_x;
	uint32_t m_y;
	uint32_t m_width;
	uint32_t m_height;
	bool m_x11_detectable_autorepeat;

	uint32_t m_parent_window_handle;
	int32_t m_fullscreen;
	int32_t m_compile;
	int32_t m_continue;

	char m_event_buffer[1024 * 4];
	LinearAllocator m_alloc;
	EventQueue m_queue;
};

} // namespace crown

int main(int argc, char** argv)
{
	crown::init();

	crown::LinuxDevice* engine = CE_NEW(crown::default_allocator(), crown::LinuxDevice)();
	crown::set_device(engine);
	
	int32_t ret = engine->run(argc, argv);

	CE_DELETE(crown::default_allocator(), engine);

	crown::shutdown();
	return ret;
}
