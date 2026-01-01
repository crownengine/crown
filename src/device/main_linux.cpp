/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#if CROWN_PLATFORM_LINUX
#include "core/command_line.h"
#include "core/containers/array.inl"
#include "core/debug/callstack.h"
#include "core/guid.h"
#include "core/memory/globals.h"
#include "core/memory/memory.inl"
#include "core/option.inl"
#include "core/os.h"
#include "core/profiler.h"
#include "core/thread/spsc_queue.inl"
#include "core/thread/thread.h"
#include "core/unit_tests.h"
#include "device/device.h"
#include "device/device_event_queue.inl"
#include "device/display.h"
#include "device/window.h"
#include "resource/data_compiler.h"
#include <fcntl.h>  // O_RDONLY, ...
#include <stdlib.h>
#include <string.h> // memset
#include <unistd.h> // close
#include <X11/cursorfont.h>
#include <X11/extensions/Xrandr.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#define STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_NOUNALIGNED
#include <stb_sprintf.h>
#include <signal.h>
#include <errno.h>

namespace crown
{
static KeyboardButton::Enum x11_translate_key(KeySym x11_key)
{
	switch (x11_key) {
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
	struct AxisData
	{
		s16 left[3];
		s16 right[3];
	};

	DeviceEventQueue *_queue;
	int _fd[CROWN_MAX_JOYPADS];
	AxisData _axis[CROWN_MAX_JOYPADS];

	explicit Joypad(DeviceEventQueue &queue)
		: _queue(&queue)
	{
		memset(&_fd, 0, sizeof(_fd));
		memset(&_axis, 0, sizeof(_axis));
	}

	void open()
	{
		char jspath[] = "/dev/input/jsX";
		char *num = strchr(jspath, 'X');

		for (u32 ii = 0; ii < CROWN_MAX_JOYPADS; ++ii) {
			*num = '0' + ii;
			_fd[ii] = ::open(jspath, O_RDONLY);
			_queue->push_status_event(InputDeviceType::JOYPAD, ii, _fd[ii] >= 0);
		}
	}

	void close()
	{
		for (u32 ii = 0; ii < CROWN_MAX_JOYPADS; ++ii) {
			if (_fd[ii] != -1) {
				::close(_fd[ii]);
				_fd[ii] = -1;
				_queue->push_status_event(InputDeviceType::JOYPAD, ii, false);
			}
		}
	}

	void process_events(u32 joypad_id, const JoypadEvent *events, u32 num_events)
	{
		for (u32 ii = 0; ii < num_events; ++ii) {
			JoypadEvent ev = events[ii];

			switch (ev.type &= ~JS_EVENT_INIT) {
			case JS_EVENT_AXIS: {
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
				s16 value = ev.value;
				if (ev.number == 2 || ev.number == 5)
					value = (ev.value + INT16_MAX) >> 1;

				s16 *values = ev.number > 2 ? _axis[joypad_id].right : _axis[joypad_id].left;
				values[axis_idx[ev.number]] = value;

				if (ev.number == 2 || ev.number == 5) {
					_queue->push_axis_event(InputDeviceType::JOYPAD
						, joypad_id
						, axis_map[ev.number]
						, 0
						, 0
						, values[2]
						);
				} else if (ev.number < countof(axis_map)) {
					_queue->push_axis_event(InputDeviceType::JOYPAD
						, joypad_id
						, axis_map[ev.number]
						, values[0]
						, -values[1]
						, 0
						);
				}
				break;
			}

			case JS_EVENT_BUTTON:
				if (ev.number < countof(s_button)) {
					_queue->push_button_event(InputDeviceType::JOYPAD
						, joypad_id
						, s_button[ev.number]
						, ev.value == 1
						);
				}
				break;

			default:
				break;
			}
		}
	}

	void update(fd_set *fdset)
	{
		for (u8 ii = 0; ii < CROWN_MAX_JOYPADS; ++ii) {
			if (_fd[ii] == -1 || !FD_ISSET(_fd[ii], fdset))
				continue;

			// Read all events.
			JoypadEvent events[64];
			ssize_t num_bytes = read(_fd[ii], &events, sizeof(events));

			if (num_bytes > 0) {
				process_events(ii, events, num_bytes/ssize_t(sizeof(events[0])));
			} else {
				::close(_fd[ii]);
				_fd[ii] = -1;
				_queue->push_status_event(InputDeviceType::JOYPAD, ii, false);
			}
		}
	}
};

static bool s_exit = false;
static int exit_pipe[2];
static Cursor _x11_cursors[MouseCursor::COUNT];
static bool push_event(const OsEvent &ev);

#define X11_IMPORT()                                                                                                                                                                                          \
	DL_IMPORT_FUNC(XCloseDisplay,              int,         (::Display *));                                                                                                                                   \
	DL_IMPORT_FUNC(XCloseIM,                   Status,      (XIM));                                                                                                                                           \
	DL_IMPORT_FUNC(XCreateBitmapFromData,      Pixmap,      (::Display *, Drawable, _Xconst char *, unsigned int, unsigned int));                                                                             \
	DL_IMPORT_FUNC(XCreateFontCursor,          Cursor,      (::Display *, unsigned int));                                                                                                                     \
	DL_IMPORT_FUNC(XCreateIC,                  XIC,         (XIM, ...));                                                                                                                                      \
	DL_IMPORT_FUNC(XCreatePixmapCursor,        Cursor,      (::Display *, Pixmap, Pixmap, XColor *, XColor *, unsigned int, unsigned int));                                                                   \
	DL_IMPORT_FUNC(XCreateWindow,              ::Window,    (::Display *, ::Window, int, int, unsigned int, unsigned int, unsigned int, int, unsigned int, Visual *, unsigned long, XSetWindowAttributes *)); \
	DL_IMPORT_FUNC(XDefineCursor,              int,         (::Display *, ::Window, Cursor));                                                                                                                 \
	DL_IMPORT_FUNC(XDestroyIC,                 void,        (XIC));                                                                                                                                           \
	DL_IMPORT_FUNC(XDestroyWindow,             int,         (::Display *, ::Window));                                                                                                                         \
	DL_IMPORT_FUNC(XEventsQueued,              int,         (::Display *, int));                                                                                                                              \
	DL_IMPORT_FUNC(XFetchName,                 int,         (::Display *, ::Window, char **));                                                                                                                \
	DL_IMPORT_FUNC(XFlush,                     int,         (::Display *));                                                                                                                                   \
	DL_IMPORT_FUNC(XFree,                      int,         (void *));                                                                                                                                        \
	DL_IMPORT_FUNC(XFreeCursor,                int,         (::Display *, Cursor));                                                                                                                           \
	DL_IMPORT_FUNC(XFreePixmap,                int,         (::Display *, Pixmap));                                                                                                                           \
	DL_IMPORT_FUNC(XGetWindowAttributes,       Status,      (::Display *, ::Window, XWindowAttributes *));                                                                                                    \
	DL_IMPORT_FUNC(XGrabPointer,               int,         (::Display *, ::Window, Bool, unsigned int, int, int, ::Window, Cursor, Time));                                                                   \
	DL_IMPORT_FUNC(XIconifyWindow,             Status,      (::Display *, ::Window, int));                                                                                                                    \
	DL_IMPORT_FUNC(XInitThreads,               Status,      (void));                                                                                                                                          \
	DL_IMPORT_FUNC(XInternAtom,                Atom,        (::Display *, _Xconst char *, Bool));                                                                                                             \
	DL_IMPORT_FUNC(XLookupKeysym,              Status,      (XKeyEvent *, int));                                                                                                                              \
	DL_IMPORT_FUNC(XMapRaised,                 int,         (::Display *, ::Window));                                                                                                                         \
	DL_IMPORT_FUNC(XMoveWindow,                int,         (::Display *, ::Window, int, int));                                                                                                               \
	DL_IMPORT_FUNC(XNextEvent,                 int,         (::Display *, XEvent *));                                                                                                                         \
	DL_IMPORT_FUNC(XOpenDisplay,               ::Display *, (_Xconst char *));                                                                                                                                \
	DL_IMPORT_FUNC(XOpenIM,                    XIM,         (::Display *, struct _XrmHashBucketRec *, char *, char *));                                                                                       \
	DL_IMPORT_FUNC(XRefreshKeyboardMapping,    int,         (XMappingEvent *));                                                                                                                               \
	DL_IMPORT_FUNC(XResizeWindow,              int,         (::Display *, ::Window, unsigned int, unsigned int));                                                                                             \
	DL_IMPORT_FUNC(XSendEvent,                 Status,      (::Display *, ::Window, Bool, long, XEvent *));                                                                                                   \
	DL_IMPORT_FUNC(XSetWMProtocols,            Status,      (::Display *, ::Window, Atom *, int));                                                                                                            \
	DL_IMPORT_FUNC(XStoreName,                 int,         (::Display *, ::Window, _Xconst char *));                                                                                                         \
	DL_IMPORT_FUNC(XUngrabPointer,             int,         (::Display *, Time));                                                                                                                             \
	DL_IMPORT_FUNC(XUnmapWindow,               int,         (::Display *, ::Window));                                                                                                                         \
	DL_IMPORT_FUNC(XWarpPointer,               int,         (::Display *, ::Window, ::Window, int, int, unsigned int, unsigned int, int, int));                                                               \
	DL_IMPORT_FUNC(XkbSetDetectableAutoRepeat, Bool,        (::Display *, Bool, Bool *));                                                                                                                     \
	DL_IMPORT_FUNC(Xutf8LookupString,          int,         (XIC, XKeyPressedEvent *, char *, int, KeySym *, Status *))

#define DL_IMPORT_FUNC(func_name, return_type, params) \
	typedef return_type (*PROTO_ ## func_name)params;  \
	static PROTO_ ## func_name func_name

X11_IMPORT();

#undef DL_IMPORT_FUNC

#define XRR_IMPORT()                                                                                                 \
	DL_IMPORT_FUNC(XRRConfigCurrentConfiguration, SizeID,                   (XRRScreenConfiguration *, Rotation *)); \
	DL_IMPORT_FUNC(XRRConfigSizes,                XRRScreenSize *,          (XRRScreenConfiguration *, int *));      \
	DL_IMPORT_FUNC(XRRFreeScreenConfigInfo,       void,                     (XRRScreenConfiguration *));             \
	DL_IMPORT_FUNC(XRRGetScreenInfo,              XRRScreenConfiguration *, (::Display *, ::Window));                \
	DL_IMPORT_FUNC(XRRSetScreenConfig,            Status,                   (::Display *, XRRScreenConfiguration *, Drawable, int, Rotation, Time))

#define DL_IMPORT_FUNC(func_name, return_type, params) \
	typedef return_type (*PROTO_ ## func_name)params;  \
	static PROTO_ ## func_name func_name

XRR_IMPORT();

#undef DL_IMPORT_FUNC

struct LinuxDevice
{
	::Display *_x11_display;
	void *_x11_lib;
	void *_xrandr_lib;
	Atom _wm_delete_window;
	Atom _net_wm_state;
	Atom _net_wm_state_maximized_horz;
	Atom _net_wm_state_maximized_vert;
	Atom _net_wm_state_fullscreen;
	Cursor _x11_hidden_cursor;
	bool _x11_detectable_autorepeat;
	XRRScreenConfiguration *_screen_config;
	SPSCQueue<OsEvent, CROWN_MAX_OS_EVENTS> _events;
	DeviceEventQueue _queue;
	Joypad _joypad;
	::Window _x11_window;
	s16 _mouse_last_x;
	s16 _mouse_last_y;
	CursorMode::Enum _cursor_mode;

	explicit LinuxDevice(Allocator &a)
		: _x11_display(NULL)
		, _x11_lib(NULL)
		, _xrandr_lib(NULL)
		, _wm_delete_window(None)
		, _net_wm_state(None)
		, _net_wm_state_maximized_horz(None)
		, _net_wm_state_maximized_vert(None)
		, _net_wm_state_fullscreen(None)
		, _x11_hidden_cursor(None)
		, _x11_detectable_autorepeat(false)
		, _screen_config(NULL)
		, _events(a)
		, _queue(push_event)
		, _joypad(_queue)
		, _x11_window(None)
		, _mouse_last_x(INT16_MAX)
		, _mouse_last_y(INT16_MAX)
		, _cursor_mode(CursorMode::NORMAL)
	{
	}

	int run(DeviceOptions *opts)
	{
		int err = pipe(exit_pipe);
		CE_ASSERT(err != -1, "pipe: errno = %d", errno);
		CE_UNUSED(err);

		_x11_lib = os::library_open("libX11.so.6");
#define DL_IMPORT_FUNC(func_name, return_type, params)                          \
	func_name = (PROTO_ ## func_name)os::library_symbol(_x11_lib, # func_name); \
	CE_ENSURE(func_name != NULL);

		X11_IMPORT();

#undef DL_IMPORT_FUNC

		_xrandr_lib = os::library_open("libXrandr.so.2");
#define DL_IMPORT_FUNC(func_name, return_type, params)                             \
	func_name = (PROTO_ ## func_name)os::library_symbol(_xrandr_lib, # func_name); \
	CE_ENSURE(func_name != NULL);

		XRR_IMPORT();

#undef DL_IMPORT_FUNC

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

		_wm_delete_window = XInternAtom(_x11_display, "WM_DELETE_WINDOW", False);
		_net_wm_state = XInternAtom(_x11_display, "_NET_WM_STATE", False);
		_net_wm_state_maximized_horz = XInternAtom(_x11_display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
		_net_wm_state_maximized_vert = XInternAtom(_x11_display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
		_net_wm_state_fullscreen = XInternAtom(_x11_display, "_NET_WM_STATE_FULLSCREEN", False);

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

		// Create hidden cursor
		Pixmap bitmap;
		const char data[8] = { 0 };
		XColor dummy;
		bitmap = XCreateBitmapFromData(_x11_display, root_window, data, 8, 8);
		_x11_hidden_cursor = XCreatePixmapCursor(_x11_display, bitmap, bitmap, &dummy, &dummy, 0, 0);

		// Create standard cursors
		_x11_cursors[MouseCursor::ARROW]               = XCreateFontCursor(_x11_display, XC_top_left_arrow);
		_x11_cursors[MouseCursor::HAND]                = XCreateFontCursor(_x11_display, XC_hand2);
		_x11_cursors[MouseCursor::TEXT_INPUT]          = XCreateFontCursor(_x11_display, XC_xterm);
		_x11_cursors[MouseCursor::CORNER_TOP_LEFT]     = XCreateFontCursor(_x11_display, XC_top_left_corner);
		_x11_cursors[MouseCursor::CORNER_TOP_RIGHT]    = XCreateFontCursor(_x11_display, XC_top_right_corner);
		_x11_cursors[MouseCursor::CORNER_BOTTOM_LEFT]  = XCreateFontCursor(_x11_display, XC_bottom_left_corner);
		_x11_cursors[MouseCursor::CORNER_BOTTOM_RIGHT] = XCreateFontCursor(_x11_display, XC_bottom_right_corner);
		_x11_cursors[MouseCursor::SIZE_HORIZONTAL]     = XCreateFontCursor(_x11_display, XC_sb_h_double_arrow);
		_x11_cursors[MouseCursor::SIZE_VERTICAL]       = XCreateFontCursor(_x11_display, XC_sb_v_double_arrow);
		_x11_cursors[MouseCursor::WAIT]                = XCreateFontCursor(_x11_display, XC_watch);

		// Start main thread
		Thread main_thread;
		main_thread.start([](void *user_data) {
				int ec = crown::main_runtime(*((DeviceOptions *)user_data));
				s_exit = true;
				// Write something just to unlock the listening select().
				write(exit_pipe[1], &s_exit, sizeof(s_exit));
				return ec;
			}
			, opts
			);

		_joypad.open();

		// Input events loop.
		fd_set fdset;
		int x11_fd = ConnectionNumber(_x11_display);

		while (!s_exit) {
			FD_ZERO(&fdset);
			FD_SET(x11_fd, &fdset);
			FD_SET(exit_pipe[0], &fdset);
			int maxfd = max(x11_fd, exit_pipe[0]);

			for (int i = 0; i < CROWN_MAX_JOYPADS; ++i) {
				if (_joypad._fd[i] != -1) {
					FD_SET(_joypad._fd[i], &fdset);
					maxfd = max(maxfd, _joypad._fd[i]);
				}
			}

			if (select(maxfd + 1, &fdset, NULL, NULL, NULL) <= 0)
				continue;

			if (FD_ISSET(exit_pipe[0], &fdset)) {
				break;
			} else if (FD_ISSET(x11_fd, &fdset)) {
				while (XEventsQueued(_x11_display, QueuedAfterFlush) > 0) {
					XEvent event;
					XNextEvent(_x11_display, &event);

					switch (event.type) {
					case EnterNotify:
						_mouse_last_x = (s16)event.xcrossing.x;
						_mouse_last_y = (s16)event.xcrossing.y;
						_queue.push_axis_event(InputDeviceType::MOUSE
							, 0
							, MouseAxis::CURSOR
							, event.xcrossing.x
							, event.xcrossing.y
							, 0
							);
						break;

					case ClientMessage:
						if ((Atom)event.xclient.data.l[0] == _wm_delete_window)
							_queue.push_exit_event();
						break;

					case ConfigureNotify:
						_queue.push_resolution_event(event.xconfigure.width
							, event.xconfigure.height
							);
						break;

					case ButtonPress:
					case ButtonRelease: {
						if (event.xbutton.button == Button4 || event.xbutton.button == Button5) {
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
						switch (event.xbutton.button) {
						case Button1: mb = MouseButton::LEFT; break;
						case Button2: mb = MouseButton::MIDDLE; break;
						case Button3: mb = MouseButton::RIGHT; break;
						default: mb = MouseButton::COUNT; break;
						}

						if (mb != MouseButton::COUNT) {
							_queue.push_button_event(InputDeviceType::MOUSE
								, 0
								, mb
								, event.type == ButtonPress
								);
						}
						break;
					}

					case MotionNotify: {
						const s32 mx = event.xmotion.x;
						const s32 my = event.xmotion.y;
						s16 deltax = mx - _mouse_last_x;
						s16 deltay = my - _mouse_last_y;
						if (_cursor_mode == CursorMode::DISABLED) {
							XWindowAttributes window_attribs;
							XGetWindowAttributes(_x11_display, _x11_window, &window_attribs);
							unsigned width = window_attribs.width;
							unsigned height = window_attribs.height;
							if (mx != (s32)width/2 || my != (s32)height/2) {
								_queue.push_axis_event(InputDeviceType::MOUSE
									, 0
									, MouseAxis::CURSOR_DELTA
									, deltax
									, deltay
									, 0
									);
								XWarpPointer(_x11_display
									, None
									, _x11_window
									, 0
									, 0
									, 0
									, 0
									, width/2
									, height/2
									);
								XFlush(_x11_display);
							}
						} else if (_cursor_mode == CursorMode::NORMAL) {
							_queue.push_axis_event(InputDeviceType::MOUSE
								, 0
								, MouseAxis::CURSOR_DELTA
								, deltax
								, deltay
								, 0
								);
						}
						_queue.push_axis_event(InputDeviceType::MOUSE
							, 0
							, MouseAxis::CURSOR
							, (s16)mx
							, (s16)my
							, 0
							);
						_mouse_last_x = (s16)mx;
						_mouse_last_y = (s16)my;
						break;
					}

					case KeyPress:
					case KeyRelease: {
						KeySym keysym = XLookupKeysym(&event.xkey, 0);

						KeyboardButton::Enum kb = x11_translate_key(keysym);
						if (kb != KeyboardButton::COUNT) {
							_queue.push_button_event(InputDeviceType::KEYBOARD
								, 0
								, kb
								, event.type == KeyPress
								);
						}

						if (event.type == KeyPress) {
							Status status = 0;
							u8 utf8[4] = { 0 };
							int len = Xutf8LookupString(ic
								, &event.xkey
								, (char *)utf8
								, sizeof(utf8)
								, NULL
								, &status
								);

							if (status == XLookupChars || status == XLookupBoth) {
								if (len)
									_queue.push_text_event(len, utf8);
							}
						}
						break;
					}
					case KeymapNotify:
						XRefreshKeyboardMapping(&event.xmapping);
						break;

					default:
						break;
					}
				}
			} else {
				_joypad.update(&fdset);
			}
		}

		_joypad.close();

		main_thread.stop();

		// Free standard cursors
		XFreeCursor(_x11_display, _x11_cursors[MouseCursor::WAIT]);
		XFreeCursor(_x11_display, _x11_cursors[MouseCursor::SIZE_VERTICAL]);
		XFreeCursor(_x11_display, _x11_cursors[MouseCursor::SIZE_HORIZONTAL]);
		XFreeCursor(_x11_display, _x11_cursors[MouseCursor::CORNER_BOTTOM_RIGHT]);
		XFreeCursor(_x11_display, _x11_cursors[MouseCursor::CORNER_BOTTOM_LEFT]);
		XFreeCursor(_x11_display, _x11_cursors[MouseCursor::CORNER_TOP_RIGHT]);
		XFreeCursor(_x11_display, _x11_cursors[MouseCursor::CORNER_TOP_LEFT]);
		XFreeCursor(_x11_display, _x11_cursors[MouseCursor::TEXT_INPUT]);
		XFreeCursor(_x11_display, _x11_cursors[MouseCursor::HAND]);
		XFreeCursor(_x11_display, _x11_cursors[MouseCursor::ARROW]);

		// Free hidden cursor
		XFreeCursor(_x11_display, _x11_hidden_cursor);
		XFreePixmap(_x11_display, bitmap);

		XDestroyIC(ic);
		XCloseIM(im);

		// Restore previous screen configuration
		Rotation rr_rot;
		const SizeID rr_sizeid = XRRConfigCurrentConfiguration(_screen_config, &rr_rot);

		if (rr_rot != rr_old_rot || rr_sizeid != rr_old_sizeid) {
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

		os::library_close(_xrandr_lib);
		os::library_close(_x11_lib);

		::close(exit_pipe[0]);
		::close(exit_pipe[1]);
		return main_thread.exit_code();
	}
};

static LinuxDevice *s_linux_device;

struct WindowX11 : public Window
{
	WindowX11()
	{
	}

	void open(u16 x, u16 y, u16 width, u16 height, u32 parent) override
	{
		int screen = DefaultScreen(s_linux_device->_x11_display);
		::Window root_window = RootWindow(s_linux_device->_x11_display, screen);
		::Window parent_window = (parent == 0) ? root_window : (::Window)parent;

		// Create main window
		XSetWindowAttributes win_attribs;
		win_attribs.background_pixmap = 0;
		win_attribs.border_pixel = 0;
		win_attribs.event_mask = FocusChangeMask
			| StructureNotifyMask
			;

		if (!parent) {
			win_attribs.event_mask |= KeyPressMask
				| KeyReleaseMask
				| ButtonPressMask
				| ButtonReleaseMask
				| PointerMotionMask
				| EnterWindowMask
				;
		}

		s_linux_device->_x11_window = XCreateWindow(s_linux_device->_x11_display
			, parent_window
			, x
			, y
			, width
			, height
			, 0
			, CopyFromParent
			, InputOutput
			, CopyFromParent
			, CWBorderPixel | CWEventMask
			, &win_attribs
			);
		CE_ASSERT(s_linux_device->_x11_window != None, "XCreateWindow: error");

		XSetWMProtocols(s_linux_device->_x11_display, s_linux_device->_x11_window, &s_linux_device->_wm_delete_window, 1);
	}

	void close() override
	{
		XDestroyWindow(s_linux_device->_x11_display, s_linux_device->_x11_window);
	}

	void show() override
	{
		XMapRaised(s_linux_device->_x11_display, s_linux_device->_x11_window);
	}

	void hide() override
	{
		XUnmapWindow(s_linux_device->_x11_display, s_linux_device->_x11_window);
	}

	void resize(u16 width, u16 height) override
	{
		XResizeWindow(s_linux_device->_x11_display, s_linux_device->_x11_window, width, height);
		XFlush(s_linux_device->_x11_display);
	}

	void move(u16 x, u16 y) override
	{
		XMoveWindow(s_linux_device->_x11_display, s_linux_device->_x11_window, x, y);
	}

	void maximize_or_restore(bool maximize)
	{
		XEvent xev;
		xev.type = ClientMessage;
		xev.xclient.window = s_linux_device->_x11_window;
		xev.xclient.message_type = s_linux_device->_net_wm_state;
		xev.xclient.format = 32;
		xev.xclient.data.l[0] = maximize ? 1 : 0; // 0 = remove property, 1 = set property
		xev.xclient.data.l[1] = s_linux_device->_net_wm_state_maximized_horz;
		xev.xclient.data.l[2] = s_linux_device->_net_wm_state_maximized_vert;
		XSendEvent(s_linux_device->_x11_display
			, DefaultRootWindow(s_linux_device->_x11_display)
			, False
			, SubstructureNotifyMask | SubstructureRedirectMask
			, &xev
			);
	}

	void minimize() override
	{
		XIconifyWindow(s_linux_device->_x11_display, s_linux_device->_x11_window, DefaultScreen(s_linux_device->_x11_display));
	}

	void maximize() override
	{
		maximize_or_restore(true);
	}

	void restore() override
	{
		maximize_or_restore(false);
	}

	const char *title() override
	{
		static char buf[512];
		memset(buf, 0, sizeof(buf));
		char *name;
		XFetchName(s_linux_device->_x11_display, s_linux_device->_x11_window, &name);
		strncpy(buf, name, sizeof(buf) - 1);
		XFree(name);
		return buf;
	}

	void set_title(const char *title) override
	{
		XStoreName(s_linux_device->_x11_display, s_linux_device->_x11_window, title);
	}

	void show_cursor(bool show) override
	{
		XDefineCursor(s_linux_device->_x11_display
			, s_linux_device->_x11_window
			, show ? None : s_linux_device->_x11_hidden_cursor
			);
	}

	void set_fullscreen(bool full) override
	{
		XEvent xev;
		xev.xclient.type = ClientMessage;
		xev.xclient.window = s_linux_device->_x11_window;
		xev.xclient.message_type = s_linux_device->_net_wm_state;
		xev.xclient.format = 32;
		xev.xclient.data.l[0] = full ? 1 : 0;
		xev.xclient.data.l[1] = s_linux_device->_net_wm_state_fullscreen;
		XSendEvent(s_linux_device->_x11_display, DefaultRootWindow(s_linux_device->_x11_display), False, SubstructureNotifyMask | SubstructureRedirectMask, &xev);
		XFlush(s_linux_device->_x11_display);
	}

	void set_cursor(MouseCursor::Enum cursor) override
	{
		XDefineCursor(s_linux_device->_x11_display, s_linux_device->_x11_window, _x11_cursors[cursor]);
	}

	void set_cursor_mode(CursorMode::Enum mode) override
	{
		if (mode == s_linux_device->_cursor_mode)
			return;

		s_linux_device->_cursor_mode = mode;

		if (mode == CursorMode::DISABLED) {
			XWindowAttributes window_attribs;
			XGetWindowAttributes(s_linux_device->_x11_display, s_linux_device->_x11_window, &window_attribs);
			unsigned width = window_attribs.width;
			unsigned height = window_attribs.height;
			s_linux_device->_mouse_last_x = width/2;
			s_linux_device->_mouse_last_y = height/2;

			XWarpPointer(s_linux_device->_x11_display
				, None
				, s_linux_device->_x11_window
				, 0
				, 0
				, 0
				, 0
				, width/2
				, height/2
				);
			XGrabPointer(s_linux_device->_x11_display
				, s_linux_device->_x11_window
				, True
				, ButtonPressMask | ButtonReleaseMask | PointerMotionMask
				, GrabModeAsync
				, GrabModeAsync
				, s_linux_device->_x11_window
				, s_linux_device->_x11_hidden_cursor
				, CurrentTime
				);
			XFlush(s_linux_device->_x11_display);
		} else if (mode == CursorMode::NORMAL) {
			XUngrabPointer(s_linux_device->_x11_display, CurrentTime);
			XFlush(s_linux_device->_x11_display);
		}
	}

	void *native_handle() override
	{
		return (void *)(uintptr_t)s_linux_device->_x11_window;
	}

	void *native_display() override
	{
		return s_linux_device->_x11_display;
	}
};

namespace window
{
	Window *create(Allocator &a)
	{
		return CE_NEW(a, WindowX11)();
	}

	void destroy(Allocator &a, Window &w)
	{
		CE_DELETE(a, &w);
	}

} // namespace window

struct DisplayXRandr : public Display
{
	void modes(Array<DisplayMode> &modes) override
	{
		int num = 0;
		XRRScreenSize *sizes = XRRConfigSizes(s_linux_device->_screen_config, &num);

		if (!sizes)
			return;

		for (int i = 0; i < num; ++i) {
			DisplayMode dm;
			dm.id     = (u32)i;
			dm.width  = sizes[i].width;
			dm.height = sizes[i].height;
			array::push_back(modes, dm);
		}
	}

	void set_mode(u32 id) override
	{
		int num = 0;
		XRRScreenSize *sizes = XRRConfigSizes(s_linux_device->_screen_config, &num);

		if (!sizes || (int)id >= num)
			return;

		XRRSetScreenConfig(s_linux_device->_x11_display
			, s_linux_device->_screen_config
			, RootWindow(s_linux_device->_x11_display, DefaultScreen(s_linux_device->_x11_display))
			, (int)id
			, RR_Rotate_0
			, CurrentTime
			);
	}
};

namespace display
{
	Display *create(Allocator &a)
	{
		return CE_NEW(a, DisplayXRandr)();
	}

	void destroy(Allocator &a, Display &d)
	{
		CE_DELETE(a, &d);
	}

} // namespace display

static bool push_event(const OsEvent &ev)
{
	return s_linux_device->_events.push(ev);
}

bool next_event(OsEvent &ev)
{
	return s_linux_device->_events.pop(ev);
}

struct InitGlobals
{
	InitGlobals()
	{
		memory_globals::init();
		profiler_globals::init();
		guid_globals::init();
	}

	~InitGlobals()
	{
		guid_globals::shutdown();
		profiler_globals::shutdown();
		memory_globals::shutdown();
	}
};

void at_exit()
{
	debug::callstack_shutdown();
}

} // namespace crown

int main(int argc, char **argv)
{
	using namespace crown;

	if (debug::callstack_init() != 0)
		return EXIT_FAILURE;
	if (atexit(at_exit) != 0) {
		debug::callstack_shutdown();
		return EXIT_FAILURE;
	}

	struct sigaction act;
	// code-format off
	act.sa_handler = [](int signum) {
			switch (signum)
			{
			case SIGINT:
			case SIGTERM:
				if (device())
					device()->quit();
				break;

			case SIGABRT:
			case SIGBUS:
			case SIGFPE:
			case SIGILL:
			case SIGPIPE:
			case SIGSEGV:
			case SIGSYS:
				error::abort("Signal %d", signum);
				break;

			default:
				error::abort("Unhandled signal %d", signum);
				break;
			}
		};
	// code-format on
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGABRT, &act, NULL);
	sigaction(SIGBUS, &act, NULL);
	sigaction(SIGFPE, &act, NULL);
	sigaction(SIGILL, &act, NULL);
	sigaction(SIGPIPE, &act, NULL);
	sigaction(SIGSEGV, &act, NULL);
	sigaction(SIGSYS, &act, NULL);

#if CROWN_BUILD_UNIT_TESTS
	CommandLine cl(argc, (const char **)argv);
	if (cl.has_option("run-unit-tests")) {
		return main_unit_tests();
	}
#endif

	InitGlobals m;
	CE_UNUSED(m);

	DeviceOptions opts(default_allocator(), argc, (const char **)argv);
	bool quit = false;
	int ec = opts.parse(&quit);

	if (quit)
		return ec;

#if CROWN_CAN_COMPILE
	if (ec == EXIT_SUCCESS && (opts._do_compile || opts._server)) {
		ec = main_data_compiler(opts);
		if (!opts._do_continue)
			return ec;
	}
#endif

	if (ec == EXIT_SUCCESS) {
		s_linux_device = CE_NEW(default_allocator(), LinuxDevice)(default_allocator());
		ec = s_linux_device->run(&opts);
		CE_DELETE(default_allocator(), s_linux_device);
	}

	return ec;
}

#endif // if CROWN_PLATFORM_LINUX
