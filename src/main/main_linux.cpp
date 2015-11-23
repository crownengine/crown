/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "config.h"

#if CROWN_PLATFORM_LINUX

#include "device.h"
#include "memory.h"
#include "os_event_queue.h"
#include "thread.h"
#include "command_line.h"
#include "bundle_compiler.h"
#include "console_server.h"
#include "device.h"
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/extensions/Xrandr.h>
#include <bgfxplatform.h>
#include <bgfx.h>

namespace crown
{

// void display_modes(Array<DisplayMode>& modes)
// {
// 	int num_rrsizes = 0;
// 	XRRScreenSize* rrsizes = XRRConfigSizes(m_screen_config, &num_rrsizes);

// 	for (int i = 0; i < num_rrsizes; i++)
// 	{
// 		DisplayMode dm;
// 		dm.id = (uint32_t) i;
// 		dm.width = rrsizes[i].width;
// 		dm.height = rrsizes[i].height;
// 		array::push_back(modes, dm);
// 	}
// }

// void set_display_mode(uint32_t id)
// {
// 	// Check if id is valid
// 	int num_rrsizes = 0;
// 	XRRScreenSize* rrsizes = XRRConfigSizes(m_screen_config, &num_rrsizes);
// 	(void) rrsizes;

// 	if ((int) id >= num_rrsizes)
// 		return;

// 	XRRSetScreenConfig(m_x11_display,
// 		m_screen_config,
// 		RootWindow(m_x11_display, DefaultScreen(m_x11_display)),
// 		(int) id,
// 		RR_Rotate_0,
// 		CurrentTime);
// }

// void set_fullscreen(bool full)
// {
// 	XEvent e;
// 	e.xclient.type = ClientMessage;
// 	e.xclient.window = m_x11_window;
// 	e.xclient.message_type = XInternAtom(m_x11_display, "_NET_WM_STATE", False );
// 	e.xclient.format = 32;
// 	e.xclient.data.l[0] = full ? 1 : 0;
// 	e.xclient.data.l[1] = XInternAtom(m_x11_display, "_NET_WM_STATE_FULLSCREEN", False);
// 	XSendEvent(m_x11_display, DefaultRootWindow(m_x11_display), False, SubstructureNotifyMask, &e);
// }

static KeyboardButton::Enum x11_translate_key(KeySym x11_key)
{
	switch (x11_key)
	{
		case XK_BackSpace:    return KeyboardButton::BACKSPACE;
		case XK_Tab:          return KeyboardButton::TAB;
		case XK_space:        return KeyboardButton::SPACE;
		case XK_Escape:       return KeyboardButton::ESCAPE;
		case XK_Return:       return KeyboardButton::ENTER;
		case XK_F1:           return KeyboardButton::F1;
		case XK_F2:           return KeyboardButton::F2;
		case XK_F3:           return KeyboardButton::F3;
		case XK_F4:           return KeyboardButton::F4;
		case XK_F5:           return KeyboardButton::F5;
		case XK_F6:           return KeyboardButton::F6;
		case XK_F7:           return KeyboardButton::F7;
		case XK_F8:           return KeyboardButton::F8;
		case XK_F9:           return KeyboardButton::F9;
		case XK_F10:          return KeyboardButton::F10;
		case XK_F11:          return KeyboardButton::F11;
		case XK_F12:          return KeyboardButton::F12;
		case XK_Home:         return KeyboardButton::HOME;
		case XK_Left:         return KeyboardButton::LEFT;
		case XK_Up:           return KeyboardButton::UP;
		case XK_Right:        return KeyboardButton::RIGHT;
		case XK_Down:         return KeyboardButton::DOWN;
		case XK_Page_Up:      return KeyboardButton::PAGE_UP;
		case XK_Page_Down:    return KeyboardButton::PAGE_DOWN;
		case XK_Delete:       return KeyboardButton::DELETE;
		case XK_End:          return KeyboardButton::END;
		case XK_Shift_L:      return KeyboardButton::LEFT_SHIFT;
		case XK_Shift_R:      return KeyboardButton::RIGHT_SHIFT;
		case XK_Control_L:    return KeyboardButton::LEFT_CTRL;
		case XK_Control_R:    return KeyboardButton::RIGHT_CTRL;
		case XK_Caps_Lock:    return KeyboardButton::CAPS_LOCK;
		case XK_Alt_L:        return KeyboardButton::LEFT_ALT;
		case XK_Alt_R:        return KeyboardButton::RIGHT_ALT;
		case XK_Super_L:      return KeyboardButton::LEFT_SUPER;
		case XK_Super_R:      return KeyboardButton::RIGHT_SUPER;
		case XK_Num_Lock:     return KeyboardButton::NUM_LOCK;
		case XK_KP_Enter:     return KeyboardButton::NUMPAD_ENTER;
		case XK_KP_Delete:    return KeyboardButton::NUMPAD_DELETE;
		case XK_KP_Multiply:  return KeyboardButton::NUMPAD_MULTIPLY;
		case XK_KP_Add:       return KeyboardButton::NUMPAD_ADD;
		case XK_KP_Subtract:  return KeyboardButton::NUMPAD_SUBTRACT;
		case XK_KP_Divide:    return KeyboardButton::NUMPAD_DIVIDE;
		case XK_KP_Insert:
		case XK_KP_0:         return KeyboardButton::NUMPAD_0;
		case XK_KP_End:
		case XK_KP_1:         return KeyboardButton::NUMPAD_1;
		case XK_KP_Down:
		case XK_KP_2:         return KeyboardButton::NUMPAD_2;
		case XK_KP_Page_Down: // or XK_KP_Next
		case XK_KP_3:         return KeyboardButton::NUMPAD_3;
		case XK_KP_Left:
		case XK_KP_4:         return KeyboardButton::NUMPAD_4;
		case XK_KP_Begin:
		case XK_KP_5:         return KeyboardButton::NUMPAD_5;
		case XK_KP_Right:
		case XK_KP_6:         return KeyboardButton::NUMPAD_6;
		case XK_KP_Home:
		case XK_KP_7:         return KeyboardButton::NUMPAD_7;
		case XK_KP_Up:
		case XK_KP_8:         return KeyboardButton::NUMPAD_8;
		case XK_KP_Page_Up:   // or XK_KP_Prior
		case XK_KP_9:         return KeyboardButton::NUMPAD_9;
		case '0':             return KeyboardButton::NUMBER_0;
		case '1':             return KeyboardButton::NUMBER_1;
		case '2':             return KeyboardButton::NUMBER_2;
		case '3':             return KeyboardButton::NUMBER_3;
		case '4':             return KeyboardButton::NUMBER_4;
		case '5':             return KeyboardButton::NUMBER_5;
		case '6':             return KeyboardButton::NUMBER_6;
		case '7':             return KeyboardButton::NUMBER_7;
		case '8':             return KeyboardButton::NUMBER_8;
		case '9':             return KeyboardButton::NUMBER_9;
		case 'a':             return KeyboardButton::A;
		case 'b':             return KeyboardButton::B;
		case 'c':             return KeyboardButton::C;
		case 'd':             return KeyboardButton::D;
		case 'e':             return KeyboardButton::E;
		case 'f':             return KeyboardButton::F;
		case 'g':             return KeyboardButton::G;
		case 'h':             return KeyboardButton::H;
		case 'i':             return KeyboardButton::I;
		case 'j':             return KeyboardButton::J;
		case 'k':             return KeyboardButton::K;
		case 'l':             return KeyboardButton::L;
		case 'm':             return KeyboardButton::M;
		case 'n':             return KeyboardButton::N;
		case 'o':             return KeyboardButton::O;
		case 'p':             return KeyboardButton::P;
		case 'q':             return KeyboardButton::Q;
		case 'r':             return KeyboardButton::R;
		case 's':             return KeyboardButton::S;
		case 't':             return KeyboardButton::T;
		case 'u':             return KeyboardButton::U;
		case 'v':             return KeyboardButton::V;
		case 'w':             return KeyboardButton::W;
		case 'x':             return KeyboardButton::X;
		case 'y':             return KeyboardButton::Y;
		case 'z':             return KeyboardButton::Z;
		default:              return KeyboardButton::COUNT;
	}
}

static bool s_exit = false;

struct MainThreadArgs
{
	DeviceOptions* opts;
};

int32_t func(void* data)
{
	MainThreadArgs* args = (MainThreadArgs*)data;
	crown::init(*args->opts);
	crown::update();
	crown::shutdown();
	s_exit = true;
	return EXIT_SUCCESS;
}

struct LinuxDevice
{
	LinuxDevice()
		: _x11_display(NULL)
		, _x11_window(None)
		, _x11_hidden_cursor(None)
		, _screen_config(NULL)
	{
	}

	int32_t run(DeviceOptions* opts)
	{
		// http://tronche.com/gui/x/xlib/display/XInitThreads.html
		Status xs = XInitThreads();
		CE_ASSERT(xs != 0, "XInitThreads: error");
		CE_UNUSED(xs);

		_x11_display = XOpenDisplay(NULL);
		CE_ASSERT(_x11_display != NULL, "XOpenDisplay: error");

		int screen = DefaultScreen(_x11_display);
		int depth = DefaultDepth(_x11_display, screen);
		Visual* visual = DefaultVisual(_x11_display, screen);

		Window root_window = RootWindow(_x11_display, screen);
		uint32_t pw = opts->parent_window();
		Window parent_window = (pw == 0) ? root_window : (Window)pw;

		// Create main window
		XSetWindowAttributes win_attribs;
		win_attribs.background_pixmap = 0;
		win_attribs.border_pixel = 0;
		win_attribs.event_mask = FocusChangeMask
			| StructureNotifyMask
			| KeyPressMask
			| KeyReleaseMask
			| ButtonPressMask
			| ButtonReleaseMask
			| PointerMotionMask
			| EnterWindowMask;

		_x11_window = XCreateWindow(_x11_display
			, parent_window
			, opts->window_x()
			, opts->window_y()
			, opts->window_width()
			, opts->window_height()
			, 0
			, depth
			, InputOutput
			, visual
			, CWBorderPixel | CWEventMask
			, &win_attribs
			);
		CE_ASSERT(_x11_window != None, "XCreateWindow: error");

		_wm_delete_message = XInternAtom(_x11_display, "WM_DELETE_WINDOW", False);
		XSetWMProtocols(_x11_display, _x11_window, &_wm_delete_message, 1);

		// Do we have detectable autorepeat?
		Bool detectable;
		_x11_detectable_autorepeat = (bool)XkbSetDetectableAutoRepeat(_x11_display, true, &detectable);

		// Build hidden cursor
		Pixmap bm_no;
		XColor black, dummy;
		Colormap colormap;
		static char no_data[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

		colormap = XDefaultColormap(_x11_display, screen);
		XAllocNamedColor(_x11_display, colormap, "black", &black, &dummy);
		bm_no = XCreateBitmapFromData(_x11_display, _x11_window, no_data, 8, 8);
		_x11_hidden_cursor = XCreatePixmapCursor(_x11_display, bm_no, bm_no, &black, &black, 0, 0);

		bgfx::x11SetDisplayWindow(_x11_display, _x11_window);
		XMapRaised(_x11_display, _x11_window);

		// Save screen configuration
		_screen_config = XRRGetScreenInfo(_x11_display, parent_window);

		Rotation rr_old_rot;
		const SizeID rr_old_sizeid = XRRConfigCurrentConfiguration(_screen_config, &rr_old_rot);

		// Start main thread
		MainThreadArgs mta;
		mta.opts = opts;

		Thread main_thread;
		main_thread.start(func, &mta);

		while (!s_exit)
		{
			pump_events();
		}

		main_thread.stop();

		// Restore previous screen configuration
		Rotation rr_rot;
		const SizeID rr_sizeid = XRRConfigCurrentConfiguration(_screen_config, &rr_rot);

		if (rr_rot != rr_old_rot || rr_sizeid != rr_old_sizeid)
		{
			XRRSetScreenConfig(_x11_display
				, _screen_config
				, root_window
				, rr_old_sizeid
				, rr_old_rot
				, CurrentTime
				);
		}
		XRRFreeScreenConfigInfo(_screen_config);

		XDestroyWindow(_x11_display, _x11_window);
		XCloseDisplay(_x11_display);
		return EXIT_SUCCESS;
	}

	void pump_events()
	{
		while (XPending(_x11_display))
		{
			XEvent event;
			XNextEvent(_x11_display, &event);

			switch (event.type)
			{
				case EnterNotify:
				{
					_queue.push_mouse_event(event.xcrossing.x, event.xcrossing.y);
					break;
				}
				case ClientMessage:
				{
					if ((Atom)event.xclient.data.l[0] == _wm_delete_message)
						_queue.push_exit_event(0);

					break;
				}
				case ConfigureNotify:
				{
					_queue.push_metrics_event(event.xconfigure.x
						, event.xconfigure.y
						, event.xconfigure.width
						, event.xconfigure.height
						);
					break;
				}
				case ButtonPress:
				case ButtonRelease:
				{
					if (event.xbutton.button == Button4 || event.xbutton.button == Button5)
					{
						_queue.push_mouse_event(event.xbutton.x
							, event.xbutton.y
							, event.xbutton.button == Button4 ? 1.0f : -1.0f
							);
						break;
					}

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
						_queue.push_mouse_event(event.xbutton.x
							, event.xbutton.y
							, mb
							, event.type == ButtonPress
							);
					}
					break;
				}
				case MotionNotify:
				{
					_queue.push_mouse_event(event.xmotion.x, event.xmotion.y);
					break;
				}
				case KeyPress:
				case KeyRelease:
				{
					KeySym keysym = XLookupKeysym(&event.xkey, 0);
					KeyboardButton::Enum kb = x11_translate_key(keysym);

					if (kb != KeyboardButton::COUNT)
						_queue.push_keyboard_event(kb, event.type == KeyPress);

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
	}

public:

	Display* _x11_display;
	Window _x11_window;
	Cursor _x11_hidden_cursor;
	Atom _wm_delete_message;
	XRRScreenConfiguration* _screen_config;
	bool _x11_detectable_autorepeat;
	OsEventQueue _queue;
};

static LinuxDevice s_ldvc;

bool next_event(OsEvent& ev)
{
	return s_ldvc._queue.pop_event(ev);
}

} // namespace crown

int main(int argc, char** argv)
{
	using namespace crown;
	memory_globals::init();

	DeviceOptions opts(argc, argv);

	console_server_globals::init(opts.console_port(), opts.wait_console());
	bundle_compiler_globals::init(opts.source_dir(), opts.bundle_dir());

	bool do_continue = true;
	int exitcode = EXIT_SUCCESS;

	do_continue = bundle_compiler::main(opts.do_compile(), opts.do_continue(), opts.platform());

	if (do_continue)
		exitcode = crown::s_ldvc.run(&opts);

	bundle_compiler_globals::shutdown();
	console_server_globals::shutdown();
	memory_globals::shutdown();
	return exitcode;
}

#endif // CROWN_PLATFORM_LINUX
