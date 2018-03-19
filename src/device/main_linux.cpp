/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "config.h"

#if CROWN_PLATFORM_LINUX

#include "core/command_line.h"
#include "core/containers/array.h"
#include "core/os.h"
#include "core/thread/thread.h"
#include "core/unit_tests.h"
#include "device/device.h"
#include "device/device_event_queue.h"
#include "device/display.h"
#include "device/window.h"
#include "resource/data_compiler.h"
#include <bgfx/platform.h>
#include <fcntl.h>  // O_RDONLY, ...
#include <stdlib.h>
#include <string.h> // memset
#include <unistd.h> // close
#include <X11/extensions/Xrandr.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

namespace crown
{
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
	case XK_Insert:       return KeyboardButton::INS;
	case XK_Delete:       return KeyboardButton::DEL;
	case XK_End:          return KeyboardButton::END;
	case XK_Shift_L:      return KeyboardButton::SHIFT_LEFT;
	case XK_Shift_R:      return KeyboardButton::SHIFT_RIGHT;
	case XK_Control_L:    return KeyboardButton::CTRL_LEFT;
	case XK_Control_R:    return KeyboardButton::CTRL_RIGHT;
	case XK_Caps_Lock:    return KeyboardButton::CAPS_LOCK;
	case XK_Alt_L:        return KeyboardButton::ALT_LEFT;
	case XK_Alt_R:        return KeyboardButton::ALT_RIGHT;
	case XK_Super_L:      return KeyboardButton::SUPER_LEFT;
	case XK_Super_R:      return KeyboardButton::SUPER_RIGHT;
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

#define JS_EVENT_BUTTON 0x01 /* button pressed/released */
#define JS_EVENT_AXIS   0x02 /* joystick moved */
#define JS_EVENT_INIT   0x80 /* initial state of device */

static u8 s_button[] =
{
	JoypadButton::A,
	JoypadButton::B,
	JoypadButton::X,
	JoypadButton::Y,
	JoypadButton::SHOULDER_LEFT,
	JoypadButton::SHOULDER_RIGHT,
	JoypadButton::BACK,
	JoypadButton::START,
	JoypadButton::GUIDE,
	JoypadButton::THUMB_LEFT,
	JoypadButton::THUMB_RIGHT,
	JoypadButton::UP, // FIXME (reported as axis...)
	JoypadButton::DOWN,
	JoypadButton::LEFT,
	JoypadButton::RIGHT
};

struct JoypadEvent
{
	u32 time;  /* event timestamp in milliseconds */
	s16 value; /* value */
	u8 type;   /* event type */
	u8 number; /* axis/button number */
};

struct Joypad
{
	int _fd[CROWN_MAX_JOYPADS];
	bool _connected[CROWN_MAX_JOYPADS];

	struct AxisData
	{
		s16 left[3];
		s16 right[3];
	};

	AxisData _axis[CROWN_MAX_JOYPADS];

	void init()
	{
		char jspath[] = "/dev/input/jsX";
		char* num = strchr(jspath, 'X');

		for (u8 i = 0; i < CROWN_MAX_JOYPADS; ++i)
		{
			*num = '0' + i;
			_fd[i] = open(jspath, O_RDONLY | O_NONBLOCK);
		}

		memset(_connected, 0, sizeof(_connected));
		memset(_axis, 0, sizeof(_axis));
	}

	void shutdown()
	{
		for (u8 i = 0; i < CROWN_MAX_JOYPADS; ++i)
		{
			if (_fd[i] != -1)
				close(_fd[i]);
		}
	}

	void update(DeviceEventQueue& queue)
	{
		JoypadEvent ev;
		memset(&ev, 0, sizeof(ev));

		for (u8 i = 0; i < CROWN_MAX_JOYPADS; ++i)
		{
			const int fd = _fd[i];
			const bool connected = fd != -1;

			if (connected != _connected[i])
				queue.push_status_event(InputDeviceType::JOYPAD, i, connected);

			_connected[i] = connected;

			if (!connected)
				continue;

			while(read(fd, &ev, sizeof(ev)) != -1)
			{
				s16 val = ev.value;

				switch (ev.type &= ~JS_EVENT_INIT)
				{
				case JS_EVENT_AXIS:
					{
						// Indices into axis.left/right respectively
						const u8 axis_idx[] = { 0, 1, 2, 0, 1, 2 };
						const u8 axis_map[] =
						{
							JoypadAxis::LEFT,
							JoypadAxis::LEFT,
							JoypadAxis::TRIGGER_LEFT,
							JoypadAxis::RIGHT,
							JoypadAxis::RIGHT,
							JoypadAxis::TRIGGER_RIGHT
						};

						// Remap triggers to [0, INT16_MAX]
						if (ev.number == 2 || ev.number == 5)
							val = (val + INT16_MAX) >> 1;

						s16* values = ev.number > 2 ? _axis[i].right : _axis[i].left;
						values[axis_idx[ev.number]] = val;

						if (ev.number == 2 || ev.number == 5)
						{
							queue.push_axis_event(InputDeviceType::JOYPAD
								, i
								, axis_map[ev.number]
								, 0
								, 0
								, values[2]
								);
						}
						else if (ev.number < countof(axis_map))
						{
							queue.push_axis_event(InputDeviceType::JOYPAD
								, i
								, axis_map[ev.number]
								, values[0]
								, -values[1]
								, 0
								);
						}
					}
					break;

				case JS_EVENT_BUTTON:
					if (ev.number < countof(s_button))
					{
						queue.push_button_event(InputDeviceType::JOYPAD
							, i
							, s_button[ev.number]
							, val == 1
							);
					}
					break;

				default:
					break;
				}
			}
		}
	}
};

static bool s_exit = false;

struct MainThreadArgs
{
	DeviceOptions* opts;
};

s32 func(void* data)
{
	MainThreadArgs* args = (MainThreadArgs*)data;
	crown::run(*args->opts);
	s_exit = true;
	return EXIT_SUCCESS;
}

struct LinuxDevice
{
	::Display* _x11_display;
	Atom _wm_delete_message;
	XRRScreenConfiguration* _screen_config;
	bool _x11_detectable_autorepeat;
	DeviceEventQueue _queue;
	Joypad _joypad;

	LinuxDevice()
		: _x11_display(NULL)
		, _screen_config(NULL)
		, _x11_detectable_autorepeat(false)
	{
	}

	int run(DeviceOptions* opts)
	{
		// http://tronche.com/gui/x/xlib/display/XInitThreads.html
		Status xs = XInitThreads();
		CE_ASSERT(xs != 0, "XInitThreads: error");
		CE_UNUSED(xs);

		_x11_display = XOpenDisplay(NULL);
		CE_ASSERT(_x11_display != NULL, "XOpenDisplay: error");

		::Window root_window = RootWindow(_x11_display, DefaultScreen(_x11_display));

		// Do we have detectable autorepeat?
		Bool detectable;
		_x11_detectable_autorepeat = (bool)XkbSetDetectableAutoRepeat(_x11_display, true, &detectable);

		_wm_delete_message = XInternAtom(_x11_display, "WM_DELETE_WINDOW", False);

		// Save screen configuration
		_screen_config = XRRGetScreenInfo(_x11_display, root_window);

		Rotation rr_old_rot;
		const SizeID rr_old_sizeid = XRRConfigCurrentConfiguration(_screen_config, &rr_old_rot);

		XIM im;
		im = XOpenIM(_x11_display, NULL, NULL, NULL);
		CE_ASSERT(im != NULL, "XOpenIM: error");

		XIC ic;
		ic = XCreateIC(im
			, XNInputStyle
			, 0
			| XIMPreeditNothing
			| XIMStatusNothing
			, XNClientWindow
			, root_window
			, NULL
			);
		CE_ASSERT(ic != NULL, "XCreateIC: error");

		// Start main thread
		MainThreadArgs mta;
		mta.opts = opts;

		Thread main_thread;
		main_thread.start(func, &mta);

		_joypad.init();

		while (!s_exit)
		{
			_joypad.update(_queue);
			int pending = XPending(_x11_display);

			if (!pending)
			{
				os::sleep(8);
			}
			else
			{
				XEvent event;
				XNextEvent(_x11_display, &event);

				switch (event.type)
				{
				case EnterNotify:
					_queue.push_axis_event(InputDeviceType::MOUSE
						, 0
						, MouseAxis::CURSOR
						, event.xcrossing.x
						, event.xcrossing.y
						, 0
						);
					break;

				case ClientMessage:
					if ((Atom)event.xclient.data.l[0] == _wm_delete_message)
						_queue.push_exit_event();
					break;

				case ConfigureNotify:
					_queue.push_resolution_event(event.xconfigure.width
						, event.xconfigure.height
						);
					break;

				case ButtonPress:
				case ButtonRelease:
					{
						if (event.xbutton.button == Button4 || event.xbutton.button == Button5)
						{
							_queue.push_axis_event(InputDeviceType::MOUSE
								, 0
								, MouseAxis::WHEEL
								, 0
								, event.xbutton.button == Button4 ? 1 : -1
								, 0
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
							_queue.push_button_event(InputDeviceType::MOUSE
								, 0
								, mb
								, event.type == ButtonPress
								);
						}
					}
					break;

				case MotionNotify:
					_queue.push_axis_event(InputDeviceType::MOUSE
						, 0
						, MouseAxis::CURSOR
						, event.xmotion.x
						, event.xmotion.y
						, 0
						);
					break;

				case KeyPress:
				case KeyRelease:
					{
						KeySym keysym = XLookupKeysym(&event.xkey, 0);

						if (event.type == KeyPress)
						{
							Status status = 0;
							u8 utf8[4] = { 0 };
							int len = Xutf8LookupString(ic
								, &event.xkey
								, (char*)utf8
								, sizeof(utf8)
								, &keysym
								, &status
								);

							if (status == XLookupChars || status == XLookupBoth)
							{
								if (len)
									_queue.push_text_event(len, utf8);
							}
						}

						KeyboardButton::Enum kb = x11_translate_key(keysym);
						if (kb != KeyboardButton::COUNT)
						{
							_queue.push_button_event(InputDeviceType::KEYBOARD
								, 0
								, kb
								, event.type == KeyPress
								);
						}
					}
					break;
				case KeymapNotify:
					XRefreshKeyboardMapping(&event.xmapping);
					break;

				default:
					break;
				}
			}
		}

		_joypad.shutdown();

		main_thread.stop();

		XDestroyIC(ic);
		XCloseIM(im);

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

		XCloseDisplay(_x11_display);
		return EXIT_SUCCESS;
	}
};

static LinuxDevice s_ldvc;

struct WindowX11 : public Window
{
	::Window _x11_window;
	Cursor _x11_hidden_cursor;
	Atom _wm_delete_message;

	WindowX11()
		: _x11_window(None)
		, _x11_hidden_cursor(None)
	{
	}

	void open(u16 x, u16 y, u16 width, u16 height, u32 parent)
	{
		int screen = DefaultScreen(s_ldvc._x11_display);
		int depth = DefaultDepth(s_ldvc._x11_display, screen);
		Visual* visual = DefaultVisual(s_ldvc._x11_display, screen);

		::Window root_window = RootWindow(s_ldvc._x11_display, screen);
		::Window parent_window = (parent == 0) ? root_window : (::Window)parent;

		// Create main window
		XSetWindowAttributes win_attribs;
		win_attribs.background_pixmap = 0;
		win_attribs.border_pixel = 0;
		win_attribs.event_mask = FocusChangeMask
			| StructureNotifyMask
			;

		if (!parent)
		{
			win_attribs.event_mask |= KeyPressMask
				| KeyReleaseMask
				| ButtonPressMask
				| ButtonReleaseMask
				| PointerMotionMask
				| EnterWindowMask
				;
		}
		else
		{
			XWindowAttributes parent_attrs;
			XGetWindowAttributes(s_ldvc._x11_display, parent_window, &parent_attrs);
			depth = parent_attrs.depth;
			visual = parent_attrs.visual;
		}

		_x11_window = XCreateWindow(s_ldvc._x11_display
			, parent_window
			, x
			, y
			, width
			, height
			, 0
			, depth
			, InputOutput
			, visual
			, CWBorderPixel | CWEventMask
			, &win_attribs
			);
		CE_ASSERT(_x11_window != None, "XCreateWindow: error");

		// Build hidden cursor
		Pixmap bm_no;
		XColor black, dummy;
		Colormap colormap;
		static char no_data[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

		colormap = XDefaultColormap(s_ldvc._x11_display, screen);
		XAllocNamedColor(s_ldvc._x11_display, colormap, "black", &black, &dummy);
		bm_no = XCreateBitmapFromData(s_ldvc._x11_display, _x11_window, no_data, 8, 8);
		_x11_hidden_cursor = XCreatePixmapCursor(s_ldvc._x11_display, bm_no, bm_no, &black, &black, 0, 0);

		_wm_delete_message = XInternAtom(s_ldvc._x11_display, "WM_DELETE_WINDOW", False);
		XSetWMProtocols(s_ldvc._x11_display, _x11_window, &_wm_delete_message, 1);

		XMapRaised(s_ldvc._x11_display, _x11_window);
	}

	void close()
	{
		XDestroyWindow(s_ldvc._x11_display, _x11_window);
	}

	void bgfx_setup()
	{
		bgfx::PlatformData pd;
		pd.ndt          = s_ldvc._x11_display;
		pd.nwh          = (void*)(uintptr_t)_x11_window;
		pd.context      = NULL;
		pd.backBuffer   = NULL;
		pd.backBufferDS = NULL;
		bgfx::setPlatformData(pd);
	}

	void show()
	{
		XMapRaised(s_ldvc._x11_display, _x11_window);
	}

	void hide()
	{
		XUnmapWindow(s_ldvc._x11_display, _x11_window);
	}

	void resize(u16 width, u16 height)
	{
		XResizeWindow(s_ldvc._x11_display, _x11_window, width, height);
	}

	void move(u16 x, u16 y)
	{
		XMoveWindow(s_ldvc._x11_display, _x11_window, x, y);
	}

	void minimize()
	{
	}

	void restore()
	{
	}

	const char* title()
	{
		static char buf[512];
		memset(buf, 0, sizeof(buf));
		char* name;
		XFetchName(s_ldvc._x11_display, _x11_window, &name);
		strncpy(buf, name, sizeof(buf));
		XFree(name);
		return buf;
	}

	void set_title (const char* title)
	{
		XStoreName(s_ldvc._x11_display, _x11_window, title);
	}

	void* handle()
	{
		return (void*)(uintptr_t)_x11_window;
	}

	void show_cursor(bool show)
	{
		XDefineCursor(s_ldvc._x11_display
			, _x11_window
			, show ? None : _x11_hidden_cursor
			);
	}

	void set_fullscreen(bool full)
	{
		XEvent e;
		e.xclient.type = ClientMessage;
		e.xclient.window = _x11_window;
		e.xclient.message_type = XInternAtom(s_ldvc._x11_display, "_NET_WM_STATE", False);
		e.xclient.format = 32;
		e.xclient.data.l[0] = full ? 1 : 0;
		e.xclient.data.l[1] = XInternAtom(s_ldvc._x11_display, "_NET_WM_STATE_FULLSCREEN", False);
		XSendEvent(s_ldvc._x11_display, DefaultRootWindow(s_ldvc._x11_display), False, SubstructureNotifyMask, &e);
	}
};

namespace window
{
	Window* create(Allocator& a)
	{
		return CE_NEW(a, WindowX11)();
	}

	void destroy(Allocator& a, Window& w)
	{
		CE_DELETE(a, &w);
	}

} // namespace window

struct DisplayXRandr : public Display
{
	void modes(Array<DisplayMode>& modes)
	{
		int num = 0;
		XRRScreenSize* sizes = XRRConfigSizes(s_ldvc._screen_config, &num);

		if (!sizes)
			return;

		for (int i = 0; i < num; ++i)
		{
			DisplayMode dm;
			dm.id     = (u32)i;
			dm.width  = sizes[i].width;
			dm.height = sizes[i].height;
			array::push_back(modes, dm);
		}
	}

	void set_mode(u32 id)
	{
		int num = 0;
		XRRScreenSize* sizes = XRRConfigSizes(s_ldvc._screen_config, &num);

		if (!sizes || (int)id >= num)
			return;

		XRRSetScreenConfig(s_ldvc._x11_display
			, s_ldvc._screen_config
			, RootWindow(s_ldvc._x11_display, DefaultScreen(s_ldvc._x11_display))
			, (int)id
			, RR_Rotate_0
			, CurrentTime
			);
	}
};

namespace display
{
	Display* create(Allocator& a)
	{
		return CE_NEW(a, DisplayXRandr)();
	}

	void destroy(Allocator& a, Display& d)
	{
		CE_DELETE(a, &d);
	}

} // namespace display

bool next_event(OsEvent& ev)
{
	return s_ldvc._queue.pop_event(ev);
}

} // namespace crown

struct InitMemoryGlobals
{
	InitMemoryGlobals()
	{
		crown::memory_globals::init();
	}

	~InitMemoryGlobals()
	{
		crown::memory_globals::shutdown();
	}
};

int main(int argc, char** argv)
{
	using namespace crown;
#if CROWN_BUILD_UNIT_TESTS
	CommandLine cl(argc, (const char**)argv);
	if (cl.has_option("run-unit-tests"))
	{
		return main_unit_tests();
	}
#endif // CROWN_BUILD_UNIT_TESTS
	if (cl.has_option("compile") || cl.has_option("server"))
	{
		if (main_data_compiler(argc, argv) != EXIT_SUCCESS || !cl.has_option("continue"))
			return EXIT_FAILURE;
	}

	InitMemoryGlobals m;
	CE_UNUSED(m);

	DeviceOptions opts(default_allocator(), argc, (const char**)argv);
	int ec = opts.parse();

	if (ec == EXIT_SUCCESS)
		ec = s_ldvc.run(&opts);

	return ec;
}

#endif // CROWN_PLATFORM_LINUX
