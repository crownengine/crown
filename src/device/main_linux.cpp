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
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-compose.h>
#include <sys/mman.h> // mmap
#include <linux/input-event-codes.h>
#define STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_NOUNALIGNED
#include <stb_sprintf.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>

struct wl_display;
struct wl_proxy;
struct wl_interface;

#define WAYLAND_IMPORT()                                                                                                                                   \
	DL_IMPORT_FUNC(wl_display_connect,          struct wl_display *, (const char *name));                                                                  \
	DL_IMPORT_FUNC(wl_display_dispatch,         int,                 (struct wl_display *));                                                               \
	DL_IMPORT_FUNC(wl_display_roundtrip,        int,                 (struct wl_display *));                                                               \
	DL_IMPORT_FUNC(wl_proxy_add_listener,       int,                 (struct wl_proxy *, void (**implementation)(void), void *));                          \
	DL_IMPORT_FUNC(wl_proxy_get_version,        uint32_t,            (struct wl_proxy *));                                                                 \
	DL_IMPORT_FUNC(wl_proxy_marshal_flags,      struct wl_proxy *,   (struct wl_proxy *, uint32_t, const struct wl_interface *, uint32_t, uint32_t, ...)); \
	DL_IMPORT_FUNC(wl_display_dispatch_pending, int,                 (struct wl_display *));                                                               \
	DL_IMPORT_FUNC(wl_display_get_fd,           int,                 (struct wl_display *));                                                               \
	DL_IMPORT_FUNC(wl_display_read_events,      int,                 (struct wl_display *));                                                               \
	DL_IMPORT_FUNC(wl_display_prepare_read,     int,                 (struct wl_display *));                                                               \
	DL_IMPORT_FUNC(wl_display_flush,            int,                 (struct wl_display *));

#define DL_IMPORT_FUNC(func_name, return_type, params) \
	typedef return_type (*PROTO_ ## func_name)params;  \
	extern PROTO_ ## func_name crown_ ## func_name

extern "C"
{
WAYLAND_IMPORT();
}

#undef DL_IMPORT_FUNC

#define wl_display_connect (*crown_wl_display_connect)
#define wl_display_dispatch (*crown_wl_display_dispatch)
#define wl_display_roundtrip (*crown_wl_display_roundtrip)
#define wl_proxy_add_listener (*crown_wl_proxy_add_listener)
#define wl_proxy_get_version (*crown_wl_proxy_get_version)
#define wl_proxy_marshal_flags (*crown_wl_proxy_marshal_flags)
#define wl_display_dispatch_pending (*crown_wl_display_dispatch_pending)
#define wl_display_get_fd (*crown_wl_display_get_fd)
#define wl_display_read_events (*crown_wl_display_read_events)
#define wl_display_prepare_read (*crown_wl_display_prepare_read)
#define wl_display_flush (*crown_wl_display_flush)

#include "wayland/wayland-client-protocol.h"
#include "wayland/wayland-client-protocol.c"
#include "wayland/xdg-shell-client-protocol.h"
#include "wayland/xdg-shell-client-protocol.c"
#include "wayland/relative-pointer-unstable-v1-client-protocol.h"
#include "wayland/relative-pointer-unstable-v1-client-protocol.c"
#include "wayland/pointer-constraints-unstable-v1-client-protocol.h"
#include "wayland/pointer-constraints-unstable-v1-client-protocol.c"

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

static KeyboardButton::Enum evdev_translate_key(uint32_t key)
{
	switch (key) {
	case KEY_BACKSPACE:  return KeyboardButton::BACKSPACE;
	case KEY_TAB:        return KeyboardButton::TAB;
	case KEY_SPACE:      return KeyboardButton::SPACE;
	case KEY_ESC:        return KeyboardButton::ESCAPE;
	case KEY_ENTER:      return KeyboardButton::ENTER;
	case KEY_F1:         return KeyboardButton::F1;
	case KEY_F2:         return KeyboardButton::F2;
	case KEY_F3:         return KeyboardButton::F3;
	case KEY_F4:         return KeyboardButton::F4;
	case KEY_F5:         return KeyboardButton::F5;
	case KEY_F6:         return KeyboardButton::F6;
	case KEY_F7:         return KeyboardButton::F7;
	case KEY_F8:         return KeyboardButton::F8;
	case KEY_F9:         return KeyboardButton::F9;
	case KEY_F10:        return KeyboardButton::F10;
	case KEY_F11:        return KeyboardButton::F11;
	case KEY_F12:        return KeyboardButton::F12;
	case KEY_HOME:       return KeyboardButton::HOME;
	case KEY_LEFT:       return KeyboardButton::LEFT;
	case KEY_UP:         return KeyboardButton::UP;
	case KEY_RIGHT:      return KeyboardButton::RIGHT;
	case KEY_DOWN:       return KeyboardButton::DOWN;
	case KEY_PAGEUP:     return KeyboardButton::PAGE_UP;
	case KEY_PAGEDOWN:   return KeyboardButton::PAGE_DOWN;
	case KEY_INSERT:     return KeyboardButton::INS;
	case KEY_DELETE:     return KeyboardButton::DEL;
	case KEY_END:        return KeyboardButton::END;
	case KEY_LEFTSHIFT:  return KeyboardButton::SHIFT_LEFT;
	case KEY_RIGHTSHIFT: return KeyboardButton::SHIFT_RIGHT;
	case KEY_LEFTCTRL:   return KeyboardButton::CTRL_LEFT;
	case KEY_RIGHTCTRL:  return KeyboardButton::CTRL_RIGHT;
	case KEY_CAPSLOCK:   return KeyboardButton::CAPS_LOCK;
	case KEY_LEFTALT:    return KeyboardButton::ALT_LEFT;
	case KEY_RIGHTALT:   return KeyboardButton::ALT_RIGHT;
	case KEY_LEFTMETA:   return KeyboardButton::SUPER_LEFT;
	case KEY_RIGHTMETA:  return KeyboardButton::SUPER_RIGHT;
	case KEY_NUMLOCK:    return KeyboardButton::NUM_LOCK;
	case KEY_KPENTER:    return KeyboardButton::NUMPAD_ENTER;
	case KEY_KPDOT:      return KeyboardButton::NUMPAD_DELETE;
	case KEY_KPASTERISK: return KeyboardButton::NUMPAD_MULTIPLY;
	case KEY_KPPLUS:     return KeyboardButton::NUMPAD_ADD;
	case KEY_KPMINUS:    return KeyboardButton::NUMPAD_SUBTRACT;
	case KEY_KPSLASH:    return KeyboardButton::NUMPAD_DIVIDE;
	case KEY_KP0:        return KeyboardButton::NUMPAD_0;
	case KEY_KP1:        return KeyboardButton::NUMPAD_1;
	case KEY_KP2:        return KeyboardButton::NUMPAD_2;
	case KEY_KP3:        return KeyboardButton::NUMPAD_3;
	case KEY_KP4:        return KeyboardButton::NUMPAD_4;
	case KEY_KP5:        return KeyboardButton::NUMPAD_5;
	case KEY_KP6:        return KeyboardButton::NUMPAD_6;
	case KEY_KP7:        return KeyboardButton::NUMPAD_7;
	case KEY_KP8:        return KeyboardButton::NUMPAD_8;
	case KEY_KP9:        return KeyboardButton::NUMPAD_9;
	case KEY_0:          return KeyboardButton::NUMBER_0;
	case KEY_1:          return KeyboardButton::NUMBER_1;
	case KEY_2:          return KeyboardButton::NUMBER_2;
	case KEY_3:          return KeyboardButton::NUMBER_3;
	case KEY_4:          return KeyboardButton::NUMBER_4;
	case KEY_5:          return KeyboardButton::NUMBER_5;
	case KEY_6:          return KeyboardButton::NUMBER_6;
	case KEY_7:          return KeyboardButton::NUMBER_7;
	case KEY_8:          return KeyboardButton::NUMBER_8;
	case KEY_9:          return KeyboardButton::NUMBER_9;
	case KEY_A:          return KeyboardButton::A;
	case KEY_B:          return KeyboardButton::B;
	case KEY_C:          return KeyboardButton::C;
	case KEY_D:          return KeyboardButton::D;
	case KEY_E:          return KeyboardButton::E;
	case KEY_F:          return KeyboardButton::F;
	case KEY_G:          return KeyboardButton::G;
	case KEY_H:          return KeyboardButton::H;
	case KEY_I:          return KeyboardButton::I;
	case KEY_J:          return KeyboardButton::J;
	case KEY_K:          return KeyboardButton::K;
	case KEY_L:          return KeyboardButton::L;
	case KEY_M:          return KeyboardButton::M;
	case KEY_N:          return KeyboardButton::N;
	case KEY_O:          return KeyboardButton::O;
	case KEY_P:          return KeyboardButton::P;
	case KEY_Q:          return KeyboardButton::Q;
	case KEY_R:          return KeyboardButton::R;
	case KEY_S:          return KeyboardButton::S;
	case KEY_T:          return KeyboardButton::T;
	case KEY_U:          return KeyboardButton::U;
	case KEY_V:          return KeyboardButton::V;
	case KEY_W:          return KeyboardButton::W;
	case KEY_X:          return KeyboardButton::X;
	case KEY_Y:          return KeyboardButton::Y;
	case KEY_Z:          return KeyboardButton::Z;
	default:             return KeyboardButton::COUNT;
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

	int set_fds(fd_set *fdset, int max_fd)
	{
		for (int i = 0; i < CROWN_MAX_JOYPADS; ++i) {
			if (_fd[i] != -1) {
				FD_SET(_fd[i], fdset);
				max_fd = max(max_fd, _fd[i]);
			}
		}

		return max_fd;
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
	DL_IMPORT_FUNC(XQueryExtension,            Bool,        (::Display *, char *, int *, int *, int *));                                                                                                      \
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

#define XKBCOMMON_IMPORT()                                                                                                                                                                   \
	DL_IMPORT_FUNC(xkb_context_new,            struct xkb_context *, (enum xkb_context_flags flags));                                                                                        \
	DL_IMPORT_FUNC(xkb_keymap_new_from_string, struct xkb_keymap *,  (struct xkb_context *context, const char *string, enum xkb_keymap_format format, enum xkb_keymap_compile_flags flags)); \
	DL_IMPORT_FUNC(xkb_keymap_unref,           void,                 (struct xkb_keymap *keymap));                                                                                           \
	DL_IMPORT_FUNC(xkb_state_key_get_one_sym,  xkb_keysym_t,         (struct xkb_state *state, xkb_keycode_t key));                                                                          \
	DL_IMPORT_FUNC(xkb_state_new,              struct xkb_state *,   (struct xkb_keymap *keymap));                                                                                           \
	DL_IMPORT_FUNC(xkb_state_unref,            void,                 (struct xkb_state *state));                                                                                             \

#define DL_IMPORT_FUNC(func_name, return_type, params) \
	typedef return_type (*PROTO_ ## func_name)params;  \
	static PROTO_ ## func_name func_name

XKBCOMMON_IMPORT();
#undef DL_IMPORT_FUNC

static void registry_handle_global(void *data, wl_registry *registry, uint name, const char *iface, uint ver);
static void registry_handle_global_remove(void *user_data, struct wl_registry *registry, uint32_t name);

static const wl_registry_listener _wl_registry_listener =
{
	registry_handle_global,
	registry_handle_global_remove
};

static void surface_handle_enter(void *user_data, struct wl_surface *surface, struct wl_output *output);
static void surface_handle_leave(void *user_data, struct wl_surface *surface, struct wl_output *output);

static const struct wl_surface_listener surface_listener =
{
	surface_handle_enter,
	surface_handle_leave
};

static void xdg_toplevel_handle_configure(void *user_data, struct xdg_toplevel *toplevel, int32_t width, int32_t height, wl_array *states);
static void xdg_toplevel_handle_close(void *user_data, struct xdg_toplevel *toplevel);

static const struct xdg_toplevel_listener toplevel_listener =
{
	xdg_toplevel_handle_configure,
	xdg_toplevel_handle_close
};

static void xdg_surface_handle_configure(void *user_data, struct xdg_surface *surface, uint32_t serial);

static const xdg_surface_listener xdg_surface_listener =
{
	xdg_surface_handle_configure,
};

struct WindowSystem
{
	enum Enum
	{
		X11,
		WAYLAND,

		COUNT
	};
};

struct System
{
	/// Returns the connection file descriptor or < 0 on error.
	virtual int init() = 0;

	///
	virtual void shutdown() = 0;

	/// Call when events are pending on the connection.
	virtual void handle_events(fd_set *fdset) = 0;

	///
	virtual int set_fds(fd_set *fdset, int max_fd) = 0;
};

struct SystemWayland : public System
{
	void *wl_lib;
	wl_display *display;
	wl_registry *registry;
	wl_compositor *compositor;
	wl_seat *seat;
	xdg_wm_base *wm_base;
	wl_keyboard *keyboard;
	wl_pointer *pointer;
	zwp_relative_pointer_manager_v1 *relative_pointer_manager;
	zwp_relative_pointer_v1 *relative_pointer;
	zwp_pointer_constraints_v1 *pointer_constraints;
	zwp_locked_pointer_v1 *locked_pointer;
	CursorMode::Enum cursor_mode;
	struct wl_surface *surface;
	struct xdg_surface *xdg_surface;
	struct xdg_toplevel *xdg_toplevel;
	DeviceEventQueue *queue;
	int display_fd;

	struct
	{
		void *lib;
		struct xkb_context *context;
		struct xkb_keymap *keymap;
		struct xkb_state *state;
	} xkb;

	explicit SystemWayland(DeviceEventQueue &event_queue)
		: wl_lib(NULL)
		, display(NULL)
		, registry(NULL)
		, compositor(NULL)
		, seat(NULL)
		, wm_base(NULL)
		, keyboard(NULL)
		, pointer(NULL)
		, cursor_mode(CursorMode::NORMAL)
		, queue(&event_queue)
		, xdg_surface(NULL)
		, xdg_toplevel(NULL)
		, display_fd(-1)
	{
	}

	virtual ~SystemWayland()
	{
	}

	int init() override
	{
		wl_lib = os::library_open("libwayland-client.so");
#define DL_IMPORT_FUNC(func_name, return_type, params)                                    \
	::crown_ ## func_name = (PROTO_ ## func_name)os::library_symbol(wl_lib, # func_name); \
	CE_ENSURE(func_name != NULL);
		WAYLAND_IMPORT();
#undef DL_IMPORT_FUNC
		for (int i = 0; i < countof(xdg_shell_types); ++i) {
			if (xdg_shell_types[i] == (void *)1)
				xdg_shell_types[i] = &wl_surface_interface;
			if (xdg_shell_types[i] == (void *)2)
				xdg_shell_types[i] = &wl_seat_interface;
			if (xdg_shell_types[i] == (void *)3)
				xdg_shell_types[i] = &wl_output_interface;
		}

		xkb.lib = os::library_open("libxkbcommon.so");
#define DL_IMPORT_FUNC(func_name, return_type, params)                         \
	func_name = (PROTO_ ## func_name)os::library_symbol(xkb.lib, # func_name); \
	CE_ENSURE(func_name != NULL);
		XKBCOMMON_IMPORT();
#undef DL_IMPORT_FUNC

		xkb.context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
		CE_ASSERT(xkb.context != NULL, "xkb_context_new: error");

		display = wl_display_connect(NULL);
		CE_ASSERT(display != NULL, "wl_display_connect: error");
		display_fd = wl_display_get_fd(display);

		registry = wl_display_get_registry(display);
		wl_registry_add_listener(registry, &_wl_registry_listener, this);

		wl_display_roundtrip(display);
		CE_ENSURE(display != NULL);
		CE_ENSURE(compositor != NULL);
		CE_ENSURE(seat != NULL);
		CE_ENSURE(wm_base != NULL);
		return 0;
	}

	void shutdown()
	{
		if (relative_pointer_manager)
			zwp_relative_pointer_manager_v1_destroy(relative_pointer_manager);
		if (pointer_constraints)
			zwp_pointer_constraints_v1_destroy(pointer_constraints);

		os::library_close(xkb.lib);
		os::library_close(wl_lib);
	}

	void handle_events(fd_set *fdset) override
	{
		if (FD_ISSET(display_fd, fdset)) {
			if (wl_display_prepare_read(display) < 0) {
				wl_display_dispatch_pending(display);
				return;
			}

			if (wl_display_read_events(display) < 0)
				return; // Connection error.

			wl_display_dispatch(display);
		}
	}

	int set_fds(fd_set *fdset, int max_fd)
	{
		FD_SET(display_fd, fdset);
		return max(max_fd, max(display_fd, max_fd));
	}
};

static SystemWayland *_wl;

static void keyboard_handle_keymap(void *user_data
	, wl_keyboard *keyboard
	, uint32_t format
	, int fd
	, uint32_t size
	)
{
	SystemWayland *wl = (SystemWayland *)user_data;

	char *str = (char *)mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (str == MAP_FAILED) {
		close(fd);
		return;
	}

	wl->xkb.keymap = xkb_keymap_new_from_string(wl->xkb.context
		, str
		, XKB_KEYMAP_FORMAT_TEXT_V1
		, XKB_KEYMAP_COMPILE_NO_FLAGS
		);
	CE_ENSURE(wl->xkb.keymap != NULL);
	munmap(str, size);
	close(fd);

	wl->xkb.state = xkb_state_new(wl->xkb.keymap);
	if (!wl->xkb.state) {
		CE_FATAL("xkb_state_new: error");
		xkb_keymap_unref(wl->xkb.keymap);
		return;
	}

	xkb_keymap_unref(wl->xkb.keymap);
	xkb_state_unref(wl->xkb.state);
}

static void keyboard_handle_enter(void *user_data
	, struct wl_keyboard *keyboard
	, uint32_t serial
	, struct wl_surface *surface
	, struct wl_array *keys
	)
{
}

static void keyboard_handle_leave(void *user_data
	, struct wl_keyboard *keyboard
	, uint32_t serial
	, struct wl_surface *surface
	)
{
}

static void keyboard_handle_key(void *user_data
	, wl_keyboard *keyboard
	, uint32_t serial
	, uint32_t time
	, uint32_t key
	, uint32_t state
	)
{
	SystemWayland *wl = (SystemWayland *)user_data;
	DeviceEventQueue &queue = *wl->queue;

	const KeyboardButton::Enum kb = evdev_translate_key(key);
	if (kb != KeyboardButton::COUNT) {
		queue.push_button_event(InputDeviceType::KEYBOARD
			, 0
			, kb
			, state == WL_KEYBOARD_KEY_STATE_PRESSED
			);
	}
}

static void keyboard_handle_modifiers(void *user_data
	, struct wl_keyboard *keyboard
	, uint32_t serial
	, uint32_t mods_depressed
	, uint32_t mods_latched
	, uint32_t mods_locked
	, uint32_t group
	)
{
}

static const struct wl_keyboard_listener keyboard_listener =
{
	keyboard_handle_keymap,
	keyboard_handle_enter,
	keyboard_handle_leave,
	keyboard_handle_key,
	keyboard_handle_modifiers,
};

static void pointer_handle_enter(void *user_data
	, struct wl_pointer *pointer
	, uint32_t serial
	, struct wl_surface *surface
	, wl_fixed_t surface_x
	, wl_fixed_t surface_y
	)
{
	SystemWayland *wl = (SystemWayland *)user_data;
	const s32 mx = wl_fixed_to_int(surface_x);
	const s32 my = wl_fixed_to_int(surface_y);

	wl->queue->push_axis_event(InputDeviceType::MOUSE
		, 0
		, MouseAxis::CURSOR
		, (s16)mx
		, (s16)my
		, 0
		);
}

static void pointer_handle_leave(void *user_data
	, struct wl_pointer *pointer
	, uint32_t serial
	, struct wl_surface *surface
	)
{
}

static void pointer_handle_motion(void *user_data
	, struct wl_pointer *pointer
	, uint32_t time
	, wl_fixed_t surface_x
	, wl_fixed_t surface_y
	)
{
	SystemWayland *wl = (SystemWayland *)user_data;
	DeviceEventQueue &queue = *wl->queue;

	const s32 mx = wl_fixed_to_int(surface_x);
	const s32 my = wl_fixed_to_int(surface_y);

	queue.push_axis_event(InputDeviceType::MOUSE
		, 0
		, MouseAxis::CURSOR
		, (s16)mx
		, (s16)my
		, 0
		);
}

static void pointer_handle_button(void *user_data
	, struct wl_pointer *pointer
	, uint32_t serial
	, uint32_t time
	, uint32_t button
	, uint32_t state
	)
{
	SystemWayland *wl = (SystemWayland *)user_data;
	DeviceEventQueue &queue = *wl->queue;

	MouseButton::Enum mb;
	switch (button) {
	case BTN_LEFT: mb = MouseButton::LEFT; break;
	case BTN_RIGHT: mb = MouseButton::RIGHT; break;
	case BTN_MIDDLE: mb = MouseButton::MIDDLE; break;
	default: mb = MouseButton::COUNT; break;
	}

	if (mb != MouseButton::COUNT) {
		queue.push_button_event(InputDeviceType::MOUSE
			, 0
			, mb
			, state == WL_POINTER_BUTTON_STATE_PRESSED
			);
	}
}

static void pointer_handle_axis(void *user_data
	, struct wl_pointer *pointer
	, uint32_t time
	, uint32_t axis
	, wl_fixed_t value
	)
{
}

static const struct wl_pointer_listener pointer_listener =
{
	pointer_handle_enter,
	pointer_handle_leave,
	pointer_handle_motion,
	pointer_handle_button,
	pointer_handle_axis,
};

static void relative_pointer_handle_relative_motion(void *user_data
	, struct zwp_relative_pointer_v1 *pointer
	, uint32_t time_hi
	, uint32_t time_lo
	, wl_fixed_t dx
	, wl_fixed_t dy
	, wl_fixed_t dx_unaccel
	, wl_fixed_t dy_unaccel
	)
{
	SystemWayland *wl = (SystemWayland *)user_data;
	DeviceEventQueue &queue = *wl->queue;

	queue.push_axis_event(InputDeviceType::MOUSE
		, 0
		, MouseAxis::CURSOR_DELTA
		, wl_fixed_to_int(dx_unaccel)
		, wl_fixed_to_int(dy_unaccel)
		, 0
		);
}

static const struct zwp_relative_pointer_v1_listener relative_pointer_listener =
{
	relative_pointer_handle_relative_motion
};

static void locked_pointer_handle_locked(void *data
	, struct zwp_locked_pointer_v1 *locked_pointer
	)
{
}

static void locked_pointer_handle_unlocked(void *data
	, struct zwp_locked_pointer_v1 *locked_pointer
	)
{
}

static const struct zwp_locked_pointer_v1_listener locked_pointer_listener =
{
	locked_pointer_handle_locked,
	locked_pointer_handle_unlocked
};

static void seat_handle_capabilities(void *data, wl_seat *seat, uint32_t caps)
{
	SystemWayland *wl = (SystemWayland *)data;

	if (caps & WL_SEAT_CAPABILITY_KEYBOARD) {
		wl->keyboard = wl_seat_get_keyboard(seat);
		wl_keyboard_add_listener(wl->keyboard, &keyboard_listener, wl);
	}

	if (caps & WL_SEAT_CAPABILITY_POINTER) {
		wl->pointer = wl_seat_get_pointer(seat);
		wl_pointer_add_listener(wl->pointer, &pointer_listener, wl);
	}
}

static void seat_handle_name(void *user_data
	, struct wl_seat *seat
	, const char *name
	)
{
}

static const wl_seat_listener seat_listener =
{
	seat_handle_capabilities,
	seat_handle_name,
};

static void wm_base_handle_ping(void *user_data, struct xdg_wm_base *wm_base, uint32_t serial)
{
	xdg_wm_base_pong(wm_base, serial);
}

static const struct xdg_wm_base_listener wm_base_listener =
{
	wm_base_handle_ping
};

static void registry_handle_global(void *data, wl_registry *registry, uint name, const char *iface, uint ver)
{
	CE_UNUSED(ver);

	SystemWayland *wl = (SystemWayland *)data;

	if (strcmp(iface, wl_compositor_interface.name) == 0) {
		wl->compositor = (wl_compositor *)wl_registry_bind(registry, name, &wl_compositor_interface, 1);
	} else if (strcmp(iface, wl_seat_interface.name) == 0) {
		wl->seat = (wl_seat *)wl_registry_bind(registry, name, &wl_seat_interface, 1);
		wl_seat_add_listener(wl->seat, &seat_listener, wl);
	} else if (strcmp(iface, xdg_wm_base_interface.name) == 0) {
		wl->wm_base = (xdg_wm_base *)wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
		xdg_wm_base_add_listener(wl->wm_base, &wm_base_listener, wl);
	} else if (strcmp(iface, zwp_relative_pointer_manager_v1_interface.name) == 0) {
		wl->relative_pointer_manager = (zwp_relative_pointer_manager_v1 *)wl_registry_bind(registry, name, &zwp_relative_pointer_manager_v1_interface, 1);
	} else if (strcmp(iface, zwp_pointer_constraints_v1_interface.name) == 0) {
		wl->pointer_constraints = (zwp_pointer_constraints_v1 *)wl_registry_bind(registry, name, &zwp_pointer_constraints_v1_interface, 1);
	}
}

static void registry_handle_global_remove(void *user_data
	, struct wl_registry *registry
	, uint32_t name
	)
{
}

static void surface_handle_enter(void *user_data
	, struct wl_surface *surface
	, struct wl_output *output
	)
{
}

static void surface_handle_leave(void *user_data
	, struct wl_surface *surface
	, struct wl_output *output
	)
{
}

static void xdg_surface_handle_configure(void *user_data
	, struct xdg_surface *surface
	, uint32_t serial
	)
{
	CE_UNUSED(user_data);

	xdg_surface_ack_configure(surface, serial);
}

static void xdg_toplevel_handle_configure(void *user_data
	, struct xdg_toplevel *toplevel
	, int32_t width
	, int32_t height
	, wl_array *states
	)
{
	SystemWayland *wl = (SystemWayland *)user_data;
}

static void xdg_toplevel_handle_close(void *user_data, struct xdg_toplevel *toplevel)
{
	SystemWayland *wl = (SystemWayland *)user_data;
	DeviceEventQueue &queue = *wl->queue;

	queue.push_exit_event();
}

struct SystemX11 : public System
{
	void *x11_lib;
	void *xrandr_lib;
	::Display *display;
	int display_fd;
	Atom wm_delete_window;
	Atom net_wm_state;
	Atom net_wm_state_maximized_horz;
	Atom net_wm_state_maximized_vert;
	Atom net_wm_state_fullscreen;
	Cursor hidden_cursor;
	Cursor cursors[MouseCursor::COUNT];
	bool detectable_autorepeat;
	XRRScreenConfiguration *screen_config;
	::Window window;
	Pixmap bitmap;
	XIM im;
	XIC ic;
	Rotation rr_old_rot;
	SizeID rr_old_sizeid;
	::Window root_window;
	s16 mouse_last_x;
	s16 mouse_last_y;
	CursorMode::Enum cursor_mode;
	bool xwayland;
	bool cursor_inside_window;
	bool motion_received;
	DeviceEventQueue &queue;

	explicit SystemX11(DeviceEventQueue &event_queue)
		: x11_lib(NULL)
		, xrandr_lib(NULL)
		, display(NULL)
		, display_fd(-1)
		, wm_delete_window(None)
		, net_wm_state(None)
		, net_wm_state_maximized_horz(None)
		, net_wm_state_maximized_vert(None)
		, net_wm_state_fullscreen(None)
		, hidden_cursor(None)
		, detectable_autorepeat(false)
		, screen_config(NULL)
		, window(None)
		, mouse_last_x(INT16_MAX)
		, mouse_last_y(INT16_MAX)
		, cursor_mode(CursorMode::NORMAL)
		, xwayland(false)
		, cursor_inside_window(false)
		, motion_received(false)
		, queue(event_queue)
	{
	}

	virtual ~SystemX11()
	{
	}

	int init() override
	{
		x11_lib = os::library_open("libX11.so.6");
#define DL_IMPORT_FUNC(func_name, return_type, params)                         \
	func_name = (PROTO_ ## func_name)os::library_symbol(x11_lib, # func_name); \
	CE_ENSURE(func_name != NULL);

		X11_IMPORT();

#undef DL_IMPORT_FUNC

		xrandr_lib = os::library_open("libXrandr.so.2");
#define DL_IMPORT_FUNC(func_name, return_type, params)                            \
	func_name = (PROTO_ ## func_name)os::library_symbol(xrandr_lib, # func_name); \
	CE_ENSURE(func_name != NULL);

		XRR_IMPORT();

#undef DL_IMPORT_FUNC

		// http://tronche.com/gui/x/xlib/display/XInitThreads.html
		Status xs = XInitThreads();
		CE_ASSERT(xs != 0, "XInitThreads: error");
		CE_UNUSED(xs);

		display = XOpenDisplay(NULL);
		CE_ASSERT(display != NULL, "XOpenDisplay: error");
		display_fd = ConnectionNumber(display);

		int dummy_ret;
		xwayland = XQueryExtension(display, "XWAYLAND", &dummy_ret, &dummy_ret, &dummy_ret);

		root_window = RootWindow(display, DefaultScreen(display));

		// Do we have detectable autorepeat?
		Bool detectable;
		detectable_autorepeat = (bool)XkbSetDetectableAutoRepeat(display, true, &detectable);

		wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
		net_wm_state = XInternAtom(display, "_NET_WM_STATE", False);
		net_wm_state_maximized_horz = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
		net_wm_state_maximized_vert = XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
		net_wm_state_fullscreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);

		// Save screen configuration
		screen_config = XRRGetScreenInfo(display, root_window);

		rr_old_sizeid = XRRConfigCurrentConfiguration(screen_config, &rr_old_rot);

		im = XOpenIM(display, NULL, NULL, NULL);
		CE_ASSERT(im != NULL, "XOpenIM: error");

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
		const char data[8] = { 0 };
		XColor dummy;
		bitmap = XCreateBitmapFromData(display, root_window, data, 8, 8);
		hidden_cursor = XCreatePixmapCursor(display, bitmap, bitmap, &dummy, &dummy, 0, 0);

		// Create standard cursors
		cursors[MouseCursor::ARROW]               = XCreateFontCursor(display, XC_top_left_arrow);
		cursors[MouseCursor::HAND]                = XCreateFontCursor(display, XC_hand2);
		cursors[MouseCursor::TEXT_INPUT]          = XCreateFontCursor(display, XC_xterm);
		cursors[MouseCursor::CORNER_TOP_LEFT]     = XCreateFontCursor(display, XC_top_left_corner);
		cursors[MouseCursor::CORNER_TOP_RIGHT]    = XCreateFontCursor(display, XC_top_right_corner);
		cursors[MouseCursor::CORNER_BOTTOM_LEFT]  = XCreateFontCursor(display, XC_bottom_left_corner);
		cursors[MouseCursor::CORNER_BOTTOM_RIGHT] = XCreateFontCursor(display, XC_bottom_right_corner);
		cursors[MouseCursor::SIZE_HORIZONTAL]     = XCreateFontCursor(display, XC_sb_h_double_arrow);
		cursors[MouseCursor::SIZE_VERTICAL]       = XCreateFontCursor(display, XC_sb_v_double_arrow);
		cursors[MouseCursor::WAIT]                = XCreateFontCursor(display, XC_watch);

		return 0;
	}

	void shutdown() override
	{
		// Free standard cursors.
		for (s32 i = 0; i < countof(cursors); ++i)
			XFreeCursor(display, cursors[i]);

		// Free hidden cursor.
		XFreeCursor(display, hidden_cursor);
		XFreePixmap(display, bitmap);

		XDestroyIC(ic);
		XCloseIM(im);

		// Restore previous screen configuration
		Rotation rr_rot;
		const SizeID rr_sizeid = XRRConfigCurrentConfiguration(screen_config, &rr_rot);

		if (rr_rot != rr_old_rot || rr_sizeid != rr_old_sizeid) {
			XRRSetScreenConfig(display
				, screen_config
				, root_window
				, rr_old_sizeid
				, rr_old_rot
				, CurrentTime
				);
		}
		XRRFreeScreenConfigInfo(screen_config);

		XCloseDisplay(display);

		os::library_close(xrandr_lib);
		os::library_close(x11_lib);
	}

	void handle_events(fd_set *fdset) override
	{
		if (!FD_ISSET(display_fd, fdset))
			return;

		while (XEventsQueued(display, QueuedAfterFlush) > 0) {
			XEvent event;
			XNextEvent(display, &event);

			switch (event.type) {
			case EnterNotify:
				if (xwayland && !motion_received) {
					// XWayland sends spurious EnterNotify events when the pointer is outside the
					// window at start but was inside it in a previous run due to a XWarpPointer()
					// or similar calls. Ignore those events until a MotionNotify is received first.
					break;
				}

				mouse_last_x = (s16)event.xcrossing.x;
				mouse_last_y = (s16)event.xcrossing.y;
				queue.push_axis_event(InputDeviceType::MOUSE
					, 0
					, MouseAxis::CURSOR
					, event.xcrossing.x
					, event.xcrossing.y
					, 0
					);
				break;

			case LeaveNotify:
				cursor_inside_window = false;
				motion_received = false;
				break;

			case ClientMessage:
				if ((Atom)event.xclient.data.l[0] == wm_delete_window)
					queue.push_exit_event();
				break;

			case ConfigureNotify:
				queue.push_resolution_event(event.xconfigure.width
					, event.xconfigure.height
					);
				break;

			case ButtonPress:
			case ButtonRelease: {
				if (event.xbutton.button == Button4 || event.xbutton.button == Button5) {
					queue.push_axis_event(InputDeviceType::MOUSE
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
					queue.push_button_event(InputDeviceType::MOUSE
						, 0
						, mb
						, event.type == ButtonPress
						);
				}
				break;
			}

			case MotionNotify: {
				motion_received = true;
				cursor_inside_window = true;

				const s32 mx = event.xmotion.x;
				const s32 my = event.xmotion.y;
				s16 deltax = mx - mouse_last_x;
				s16 deltay = my - mouse_last_y;
				if (cursor_mode == CursorMode::DISABLED) {
					XWindowAttributes window_attribs;
					XGetWindowAttributes(display, window, &window_attribs);
					unsigned width = window_attribs.width;
					unsigned height = window_attribs.height;
					if (mx != (s32)width/2 || my != (s32)height/2) {
						queue.push_axis_event(InputDeviceType::MOUSE
							, 0
							, MouseAxis::CURSOR_DELTA
							, deltax
							, deltay
							, 0
							);
						XWarpPointer(display
							, None
							, window
							, 0
							, 0
							, 0
							, 0
							, width/2
							, height/2
							);
						XFlush(display);
					}
				} else if (cursor_mode == CursorMode::NORMAL) {
					queue.push_axis_event(InputDeviceType::MOUSE
						, 0
						, MouseAxis::CURSOR_DELTA
						, deltax
						, deltay
						, 0
						);
				}
				queue.push_axis_event(InputDeviceType::MOUSE
					, 0
					, MouseAxis::CURSOR
					, (s16)mx
					, (s16)my
					, 0
					);
				mouse_last_x = (s16)mx;
				mouse_last_y = (s16)my;
				break;
			}

			case KeyPress:
			case KeyRelease: {
				KeySym keysym = XLookupKeysym(&event.xkey, 0);

				KeyboardButton::Enum kb = x11_translate_key(keysym);
				if (kb != KeyboardButton::COUNT) {
					queue.push_button_event(InputDeviceType::KEYBOARD
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
							queue.push_text_event(len, utf8);
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
	}

	int set_fds(fd_set *fdset, int max_fd) override
	{
		FD_SET(display_fd, fdset);
		return max(display_fd, max_fd);
	}
};

static SystemX11 *_x11;

struct LinuxDevice
{
	Allocator *_allocator;
	DeviceOptions *_options;
	SPSCQueue<OsEvent, CROWN_MAX_OS_EVENTS> _events;
	DeviceEventQueue _queue;
	Joypad _joypad;
	System *_system;
	WindowSystem::Enum window_system;

	explicit LinuxDevice(Allocator &a)
		: _allocator(&a)
		, _events(a)
		, _queue(push_event)
		, _joypad(_queue)
		, window_system(WindowSystem::COUNT)
	{
	}

	int run(DeviceOptions *opts)
	{
		int init_ret = -1;
		const char *display = NULL;
		bool disable_wayland = true;

		if (init_ret != 0 && !disable_wayland
			&& (display = getenv("WAYLAND_DISPLAY")) != NULL
			&& strlen32(display) != 0) {
			_system = CE_NEW(*_allocator, SystemWayland)(_queue);

			if ((init_ret = _system->init()) == 0) {
				_wl = (SystemWayland *)_system;
				window_system = WindowSystem::WAYLAND;
			}
		}

		if (init_ret != 0
			&& (display = getenv("DISPLAY")) != NULL
			&& strlen32(display) != 0) {
			_system = CE_NEW(*_allocator, SystemX11)(_queue);

			if ((init_ret = _system->init()) == 0) {
				_x11 = (SystemX11 *)_system;
				window_system = WindowSystem::X11;
			}
		}

		_options = opts;

		int err = pipe(exit_pipe);
		CE_ASSERT(err != -1, "pipe: errno = %d", errno);
		CE_UNUSED(err);

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

		while (!s_exit) {
			FD_ZERO(&fdset);
			FD_SET(exit_pipe[0], &fdset);
			int maxfd = _system->set_fds(&fdset, exit_pipe[0]);
			maxfd = _joypad.set_fds(&fdset, maxfd);

			if (select(maxfd + 1, &fdset, NULL, NULL, NULL) <= 0)
				continue;

			if (FD_ISSET(exit_pipe[0], &fdset)) {
				break;
			} else {
				_system->handle_events(&fdset);
				_joypad.update(&fdset);
			}
		}

		_joypad.close();

		main_thread.stop();

		_system->shutdown();
		CE_DELETE(*_allocator, _system);

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
		int screen = DefaultScreen(_x11->display);
		::Window root_window = RootWindow(_x11->display, screen);
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
				| LeaveWindowMask
				;
		}

		_x11->window = XCreateWindow(_x11->display
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
		CE_ASSERT(_x11->window != None, "XCreateWindow: error");

		XSetWMProtocols(_x11->display, _x11->window, &_x11->wm_delete_window, 1);
	}

	void close() override
	{
		XDestroyWindow(_x11->display, _x11->window);
	}

	void show() override
	{
		XMapRaised(_x11->display, _x11->window);
	}

	void hide() override
	{
		XUnmapWindow(_x11->display, _x11->window);
	}

	void resize(u16 width, u16 height) override
	{
		XResizeWindow(_x11->display, _x11->window, width, height);
		XFlush(_x11->display);
	}

	void move(u16 x, u16 y) override
	{
		XMoveWindow(_x11->display, _x11->window, x, y);
	}

	void maximize_or_restore(bool maximize)
	{
		XEvent xev;
		xev.type = ClientMessage;
		xev.xclient.window = _x11->window;
		xev.xclient.message_type = _x11->net_wm_state;
		xev.xclient.format = 32;
		xev.xclient.data.l[0] = maximize ? 1 : 0; // 0 = remove property, 1 = set property
		xev.xclient.data.l[1] = _x11->net_wm_state_maximized_horz;
		xev.xclient.data.l[2] = _x11->net_wm_state_maximized_vert;
		XSendEvent(_x11->display
			, DefaultRootWindow(_x11->display)
			, False
			, SubstructureNotifyMask | SubstructureRedirectMask
			, &xev
			);
	}

	void minimize() override
	{
		XIconifyWindow(_x11->display, _x11->window, DefaultScreen(_x11->display));
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
		XFetchName(_x11->display, _x11->window, &name);
		strncpy(buf, name, sizeof(buf) - 1);
		XFree(name);
		return buf;
	}

	void set_title(const char *title) override
	{
		XStoreName(_x11->display, _x11->window, title);
	}

	void show_cursor(bool show) override
	{
		XDefineCursor(_x11->display
			, _x11->window
			, show ? None : _x11->hidden_cursor
			);
	}

	void set_fullscreen(bool full) override
	{
		XEvent xev;
		xev.xclient.type = ClientMessage;
		xev.xclient.window = _x11->window;
		xev.xclient.message_type = _x11->net_wm_state;
		xev.xclient.format = 32;
		xev.xclient.data.l[0] = full ? 1 : 0;
		xev.xclient.data.l[1] = _x11->net_wm_state_fullscreen;
		XSendEvent(_x11->display, DefaultRootWindow(_x11->display), False, SubstructureNotifyMask | SubstructureRedirectMask, &xev);
		XFlush(_x11->display);
	}

	void set_cursor(MouseCursor::Enum cursor) override
	{
		XDefineCursor(_x11->display, _x11->window, _x11->cursors[cursor]);
	}

	bool set_cursor_mode(CursorMode::Enum mode) override
	{
		if (mode == _x11->cursor_mode)
			return true;

		if (mode == CursorMode::DISABLED) {
			if (_x11->xwayland && !_x11->cursor_inside_window)
				return false;

			XWindowAttributes window_attribs;
			XGetWindowAttributes(_x11->display, _x11->window, &window_attribs);
			_x11->mouse_last_x = window_attribs.width/2;
			_x11->mouse_last_y = window_attribs.height/2;

			XWarpPointer(_x11->display
				, None
				, _x11->window
				, 0
				, 0
				, 0
				, 0
				, _x11->mouse_last_x
				, _x11->mouse_last_y
				);
			XGrabPointer(_x11->display
				, _x11->window
				, True
				, ButtonPressMask | ButtonReleaseMask | PointerMotionMask
				, GrabModeAsync
				, GrabModeAsync
				, _x11->window
				, _x11->hidden_cursor
				, CurrentTime
				);
			XFlush(_x11->display);
		} else if (mode == CursorMode::NORMAL) {
			XUngrabPointer(_x11->display, CurrentTime);
			XFlush(_x11->display);
		}

		_x11->cursor_mode = mode;
		return true;
	}

	void *native_handle() override
	{
		return (void *)(uintptr_t)_x11->window;
	}

	void *native_handle_type() override
	{
		return (void *)(uintptr_t)bgfx::NativeWindowHandleType::Default;
	}

	void *native_display() override
	{
		return _x11->display;
	}
};

struct WindowWayland : public Window
{
	WindowWayland()
	{
	}

	void open(u16 x, u16 y, u16 width, u16 height, u32 parent) override
	{
		_wl->surface = wl_compositor_create_surface(_wl->compositor);
		CE_ENSURE(_wl->surface != NULL);

		wl_surface_add_listener(_wl->surface, &surface_listener, _wl);
	}

	void close() override
	{
		wl_surface_destroy(_wl->surface);
	}

	void show() override
	{
		get_toplevel_objects();
	}

	void hide() override
	{
		destroy_toplevel_objects();
	}

	void resize(u16 width, u16 height) override
	{
	}

	void move(u16 x, u16 y) override
	{
	}

	void maximize_or_restore(bool maximize)
	{
	}

	void minimize() override
	{
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
		return "";
	}

	void set_title(const char *title) override
	{
	}

	void show_cursor(bool show) override
	{
	}

	void set_fullscreen(bool full) override
	{
	}

	void set_cursor(MouseCursor::Enum cursor) override
	{
	}

	bool set_cursor_mode(CursorMode::Enum mode) override
	{
		if (mode == _wl->cursor_mode)
			return true;

		_wl->cursor_mode = mode;

		if (mode == CursorMode::DISABLED) {
			if (!_wl->relative_pointer_manager)
				return false;

			if (!_wl->pointer_constraints)
				return false;

			_wl->relative_pointer = zwp_relative_pointer_manager_v1_get_relative_pointer(_wl->relative_pointer_manager, _wl->pointer);
			zwp_relative_pointer_v1_add_listener(_wl->relative_pointer, &relative_pointer_listener, _wl);

			_wl->locked_pointer = zwp_pointer_constraints_v1_lock_pointer(_wl->pointer_constraints
				, _wl->surface
				, _wl->pointer
				, NULL
				, ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT
				);
			zwp_locked_pointer_v1_add_listener(_wl->locked_pointer
				, &locked_pointer_listener
				, _wl
				);
		} else if (mode == CursorMode::NORMAL) {
			zwp_relative_pointer_v1_destroy(_wl->relative_pointer);
			_wl->relative_pointer = NULL;

			zwp_locked_pointer_v1_destroy(_wl->locked_pointer);
			_wl->locked_pointer = NULL;
		}

		return true;
	}

	void *native_handle() override
	{
		return (void *)(uintptr_t)_wl->surface;
	}

	void *native_handle_type() override
	{
		return (void *)(uintptr_t)bgfx::NativeWindowHandleType::Wayland;
	}

	void *native_display() override
	{
		return _wl->display;
	}

	void get_toplevel_objects()
	{
		if (_wl->xdg_surface == NULL) {
			_wl->xdg_surface = xdg_wm_base_get_xdg_surface(_wl->wm_base, _wl->surface);
			CE_ENSURE(_wl->xdg_surface != NULL);
			xdg_surface_add_listener(_wl->xdg_surface, &xdg_surface_listener, _wl);
		}

		if (_wl->xdg_toplevel == NULL) {
			_wl->xdg_toplevel = xdg_surface_get_toplevel(_wl->xdg_surface);
			CE_ENSURE(_wl->xdg_toplevel != NULL);
			xdg_toplevel_add_listener(_wl->xdg_toplevel, &toplevel_listener, _wl);
		}

		wl_surface_commit(_wl->surface);
		wl_display_roundtrip(_wl->display);
	}

	void destroy_toplevel_objects()
	{
		if (_wl->xdg_toplevel) {
			xdg_toplevel_destroy(_wl->xdg_toplevel);
			_wl->xdg_toplevel = NULL;
		}

		if (_wl->xdg_surface) {
			xdg_surface_destroy(_wl->xdg_surface);
			_wl->xdg_surface = NULL;
		}

		wl_surface_attach(_wl->surface, NULL, 0, 0);
		wl_surface_commit(_wl->surface);
		wl_display_roundtrip(_wl->display);
	}
};

namespace window
{
	Window *create(Allocator &a)
	{
		if (s_linux_device->window_system == WindowSystem::X11)
			return CE_NEW(a, WindowX11)();
		else
			return CE_NEW(a, WindowWayland)();
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
		XRRScreenSize *sizes = XRRConfigSizes(_x11->screen_config, &num);

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
		XRRScreenSize *sizes = XRRConfigSizes(_x11->screen_config, &num);

		if (!sizes || (int)id >= num)
			return;

		XRRSetScreenConfig(_x11->display
			, _x11->screen_config
			, RootWindow(_x11->display, DefaultScreen(_x11->display))
			, (int)id
			, RR_Rotate_0
			, CurrentTime
			);
	}
};

struct DisplayWayland : public Display
{
	void modes(Array<DisplayMode> &modes) override
	{
	}

	void set_mode(u32 id) override
	{
	}
};

namespace display
{
	Display *create(Allocator &a)
	{
		if (s_linux_device->window_system == WindowSystem::X11)
			return CE_NEW(a, DisplayXRandr)();
		else
			return CE_NEW(a, DisplayWayland)();
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
