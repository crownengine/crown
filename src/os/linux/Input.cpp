/*
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

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include "Keyboard.h"
#include "OS.h"
#include "Log.h"

namespace crown
{

namespace os
{

//-----------------------------------------------------------------------------
extern Display*		display;
extern Window		window;
extern uint32_t		window_x;
extern uint32_t		window_y;
extern uint32_t		window_width;
extern uint32_t		window_height;

static bool			x11_detectable_autorepeat = false;
static Cursor		x11_hidden_cursor = None;

//-----------------------------------------------------------------------------
static void x11_create_hidden_cursor()
{
	// Build hidden cursor
	Pixmap bm_no;
	XColor black, dummy;
	Colormap colormap;
	static char no_data[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

	colormap = DefaultColormap(display, DefaultScreen(display));
	XAllocNamedColor(display, colormap, "black", &black, &dummy);
	bm_no = XCreateBitmapFromData(display, window, no_data, 8, 8);
	x11_hidden_cursor = XCreatePixmapCursor(display, bm_no, bm_no, &black, &black, 0, 0);
}

//-----------------------------------------------------------------------------
static Key x11_translate_key(int32_t x11_key)
{
	if ((x11_key > 0x40 && x11_key < 0x5B) || (x11_key > 0x60 && x11_key < 0x7B) || (x11_key > 0x2F && x11_key < 0x3A))
	{
		return (Key)x11_key;
	}

	switch (x11_key)
	{
		case XK_BackSpace:	return KC_BACKSPACE;
		case XK_Tab:		return KC_TAB;
		case XK_space:		return KC_SPACE;
		case XK_Escape:		return KC_ESCAPE;
		case XK_Return:		return KC_ENTER;
		case XK_F1:			return KC_F1;
		case XK_F2:			return KC_F2;
		case XK_F3:			return KC_F3;
		case XK_F4:			return KC_F4;
		case XK_F5:			return KC_F5;
		case XK_F6:			return KC_F6;
		case XK_F7:			return KC_F7;
		case XK_F8:			return KC_F8;
		case XK_F9:			return KC_F9;
		case XK_F10:		return KC_F10;
		case XK_F11:		return KC_F11;
		case XK_F12:		return KC_F12;
		case XK_Home:		return KC_HOME;
		case XK_Left:		return KC_LEFT;
		case XK_Up:			return KC_UP;
		case XK_Right:		return KC_RIGHT;
		case XK_Down:		return KC_DOWN;
		case XK_Page_Up:	return KC_PAGE_UP;
		case XK_Page_Down:	return KC_PAGE_DOWN;
		case XK_Shift_L:	return KC_LSHIFT;
		case XK_Shift_R:	return KC_RSHIFT;
		case XK_Control_L:	return KC_LCONTROL;
		case XK_Control_R:	return KC_RCONTROL;
		case XK_Caps_Lock:	return KC_CAPS_LOCK;
		case XK_Alt_L:		return KC_LALT;
		case XK_Alt_R:		return KC_RALT;
		case XK_Super_L:	return KC_LSUPER;
		case XK_Super_R:	return KC_RSUPER;
		case XK_KP_0:		return KC_KP_0;
		case XK_KP_1:		return KC_KP_1;
		case XK_KP_2:		return KC_KP_2;
		case XK_KP_3:		return KC_KP_3;
		case XK_KP_4:		return KC_KP_4;
		case XK_KP_5:		return KC_KP_5;
		case XK_KP_6:		return KC_KP_6;
		case XK_KP_7:		return KC_KP_7;
		case XK_KP_8:		return KC_KP_8;
		case XK_KP_9:		return KC_KP_9;
		default:			return KC_NOKEY;
	}
}

//-----------------------------------------------------------------------------
void init_input()
{
	// We want to track keyboard and mouse events
	XSelectInput(display, window, KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask);

	// Check presence of detectable autorepeat
	Bool detectable;
	x11_detectable_autorepeat = (bool) XkbSetDetectableAutoRepeat(display, true, &detectable);

	x11_create_hidden_cursor();
}

//-----------------------------------------------------------------------------
void get_cursor_xy(int32_t& x, int32_t& y)
{
	Window unused;
	int32_t pointer_x, pointer_y, dummy;
	uint32_t dummy2;

	XQueryPointer(display, window, &unused, &unused, &dummy, &dummy, &pointer_x, &pointer_y, &dummy2);

	x = pointer_x;
	y = pointer_y;
}

//-----------------------------------------------------------------------------
void set_cursor_xy(int32_t x, int32_t y)
{
	uint32_t width;
	uint32_t height;

	get_render_window_metrics(width, height);

	XWarpPointer(display, None, window, 0, 0, width, height, x, y);

	XFlush(display);
}

//-----------------------------------------------------------------------------
void hide_cursor()
{
	XDefineCursor(display, window, x11_hidden_cursor);
}

//-----------------------------------------------------------------------------
void show_cursor()
{
	XDefineCursor(display, window, None);
}

//-----------------------------------------------------------------------------
void event_loop()
{
	XEvent event;

	OSEventParameter data_button[4] = {0, 0, 0, 0};
	OSEventParameter data_key[4] = {0, 0, 0, 0};

	while (XPending(display))
	{
		XNextEvent(display, &event);

		switch (event.type)
		{
			case ConfigureNotify:
			{
				window_x = event.xconfigure.x;
				window_y = event.xconfigure.y;
				window_width = event.xconfigure.width;
				window_height = event.xconfigure.height;

				break;
			}
			case ButtonPress:
			case ButtonRelease:
			{
				OSEventType oset_type = event.type == ButtonPress ? OSET_BUTTON_PRESS : OSET_BUTTON_RELEASE;

				data_button[0].int_value = event.xbutton.x;
				data_button[1].int_value = event.xbutton.y;

				switch (event.xbutton.button)
				{
					case Button1:
					{
						data_button[2].int_value = 0;
						push_event(oset_type, data_button[0], data_button[1], data_button[2], data_button[3]);
						break;
					}
					case Button2:
					{
						data_button[2].int_value = 1;
						push_event(oset_type, data_button[0], data_button[1], data_button[2], data_button[3]);
						break;
					}
					case Button3:
					{
						data_button[2].int_value = 2;
						push_event(oset_type, data_button[0], data_button[1], data_button[2], data_button[3]);
						break;
					}
				}

				break;
			}
			case MotionNotify:
			{
				push_event(OSET_MOTION_NOTIFY, data_button[0], data_button[1], data_button[2], data_button[3]);
				break;
			}
			case KeyPress:
			case KeyRelease:
			{
				char string[4] = {0, 0, 0, 0};
				int32_t len = -1;
				KeySym key;

				len = XLookupString(&event.xkey, string, 4, &key, NULL);

				Key kc = x11_translate_key(key);

				// Check if any modifier key is pressed or released
				int32_t modifier_mask = 0;

				if (kc == KC_LSHIFT || kc == KC_RSHIFT)
				{
					(event.type == KeyPress) ? modifier_mask |= MK_SHIFT : modifier_mask &= ~MK_SHIFT;
				}
				else if (kc == KC_LCONTROL || kc == KC_RCONTROL)
				{
					(event.type == KeyPress) ? modifier_mask |= MK_CTRL : modifier_mask &= ~MK_CTRL;
				}
				else if (kc == KC_LALT || kc == KC_RALT)
				{
					(event.type == KeyPress) ? modifier_mask |= MK_ALT : modifier_mask &= ~MK_ALT;
				}

				OSEventType oset_type = event.type == KeyPress ? OSET_KEY_PRESS : OSET_KEY_RELEASE;

				data_key[0].int_value = ((int32_t)kc);
				data_key[1].int_value = modifier_mask;

				push_event(oset_type, data_key[0], data_key[1], data_key[2], data_key[3]);

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
}


} // namespace os

} // namespace crown

