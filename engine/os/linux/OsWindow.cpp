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

#include "OsWindow.h"
#include "Assert.h"
#include "Keyboard.h"
#include "OS.h"
#include "GLContext.h"
#include "StringUtils.h"
#include "EventBuffer.h"

namespace crown
{

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
OsWindow::OsWindow(uint32_t width, uint32_t height, uint32_t parent) :
	m_x11_display(NULL),
	m_x11_window(None),
	m_x11_parent_window(None),
	m_x(0),
	m_y(0),
	m_width(width),
	m_height(height),
	m_x11_detectable_autorepeat(false),
	m_x11_hidden_cursor(None)
{
	CE_ASSERT(width != 0 || height != 0, "Width and height must differ from zero");

	m_x11_display = XOpenDisplay(NULL);

	CE_ASSERT(m_x11_display != NULL, "Unable to open X11 display");

	int screen = DefaultScreen(m_x11_display);
	int depth = DefaultDepth(m_x11_display, screen);
	Visual* visual = DefaultVisual(m_x11_display, screen);

	if (parent != 0)
	{
		m_x11_parent_window = (Window) parent;
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
				   width, height,
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

	set_x11_display_and_window(m_x11_display, m_x11_window);
}

//-----------------------------------------------------------------------------
OsWindow::~OsWindow()
{
	if (m_x11_display)
	{
		if (m_x11_window != None)
		{
			XDestroyWindow(m_x11_display, m_x11_window);
		}

		XCloseDisplay(m_x11_display);
	}
}

//-----------------------------------------------------------------------------
void OsWindow::show()
{
	XMapRaised(m_x11_display, m_x11_window);
}

//-----------------------------------------------------------------------------
void OsWindow::hide()
{
	XUnmapWindow(m_x11_display, m_x11_window);
}

//-----------------------------------------------------------------------------
void OsWindow::get_size(uint32_t& width, uint32_t& height)
{
	width = m_width;
	height = m_height;
}

//-----------------------------------------------------------------------------
void OsWindow::get_position(uint32_t& x, uint32_t& y)
{
	x = m_x;
	y = m_y;
}

//-----------------------------------------------------------------------------
void OsWindow::resize(uint32_t width, uint32_t height)
{
	XResizeWindow(m_x11_display, m_x11_window, width, height);
}

//-----------------------------------------------------------------------------
void OsWindow::move(uint32_t x, uint32_t y)
{
	XMoveWindow(m_x11_display, m_x11_window, x, y);
}

//-----------------------------------------------------------------------------
void OsWindow::show_cursor()
{
	XDefineCursor(m_x11_display, m_x11_window, None);
}

//-----------------------------------------------------------------------------
void OsWindow::hide_cursor()
{
	XDefineCursor(m_x11_display, m_x11_window, m_x11_hidden_cursor);
}

//-----------------------------------------------------------------------------
void OsWindow::get_cursor_xy(int32_t& x, int32_t& y)
{
	Window unused;
	int32_t pointer_x, pointer_y, dummy;
	uint32_t dummy2;

	XQueryPointer(m_x11_display, m_x11_window, &unused, &unused, &dummy, &dummy, &pointer_x, &pointer_y, &dummy2);

	x = pointer_x;
	y = pointer_y;
}

//-----------------------------------------------------------------------------
void OsWindow::set_cursor_xy(int32_t x, int32_t y)
{
	XWarpPointer(m_x11_display, None, m_x11_window, 0, 0, m_width, m_height, x, y);

	XFlush(m_x11_display);
}

//-----------------------------------------------------------------------------
char* OsWindow::title()
{
	static char title[1024];

	char* tmp_title;
	XFetchName(m_x11_display, m_x11_window, &tmp_title);

	string::strncpy(title, tmp_title, 1024);
	XFree(tmp_title);

	return title;
}

//-----------------------------------------------------------------------------
void OsWindow::set_title(const char* title)
{
	XStoreName(m_x11_display, m_x11_window, title);
}

//-----------------------------------------------------------------------------
void OsWindow::frame()
{
	XEvent event;

	OsMouseEvent mouse_event;
	OsKeyboardEvent keyboard_event;

	while (XPending(m_x11_display))
	{
		XNextEvent(m_x11_display, &event);

		switch (event.type)
		{
			case ConfigureNotify:
			{
				m_x = event.xconfigure.x;
				m_y = event.xconfigure.y;
				m_width = event.xconfigure.width;
				m_height = event.xconfigure.height;
				break;
			}
			case ButtonPress:
			case ButtonRelease:
			{
				OsEventType oset_type = event.type == ButtonPress ? OSET_BUTTON_PRESS : OSET_BUTTON_RELEASE;

				mouse_event.x = event.xbutton.x;
				mouse_event.y = event.xbutton.y;

				switch (event.xbutton.button)
				{
					case Button1:
					{
						mouse_event.button = 0;
						os_event_buffer()->push_event(oset_type, &mouse_event, sizeof(OsMouseEvent));
						break;
					}
					case Button2:
					{
						mouse_event.button = 1;
						os_event_buffer()->push_event(oset_type, &mouse_event, sizeof(OsMouseEvent));
						break;
					}
					case Button3:
					{
						mouse_event.button = 2;
						os_event_buffer()->push_event(oset_type, &mouse_event, sizeof(OsMouseEvent));
						break;
					}
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

				OsEventType oset_type = event.type == KeyPress ? OSET_KEY_PRESS : OSET_KEY_RELEASE;

				keyboard_event.key = ((int32_t)kc);
				keyboard_event.modifier = modifier_mask;

				os_event_buffer()->push_event(oset_type, &keyboard_event, sizeof(OsKeyboardEvent));

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

} // namespace crown