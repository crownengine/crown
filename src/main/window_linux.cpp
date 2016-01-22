/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "platform.h"

#if CROWN_PLATFORM_LINUX

#include "memory.h"
#include "window.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <bgfx/bgfxplatform.h>

namespace crown
{

Display* _x11_display = NULL;

void set_x11_display(Display* dpy)
{
	_x11_display = dpy;
}

class WindowX11 : public Window
{
	::Window _x11_window;
	Cursor _x11_hidden_cursor;
	Atom _wm_delete_message;

public:

	WindowX11()
		: _x11_window(None)
		, _x11_hidden_cursor(None)
	{
	}

	void open(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t parent)
	{
		int screen = DefaultScreen(_x11_display);
		int depth = DefaultDepth(_x11_display, screen);
		Visual* visual = DefaultVisual(_x11_display, screen);

		::Window root_window = RootWindow(_x11_display, screen);
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

		_x11_window = XCreateWindow(_x11_display
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

		colormap = XDefaultColormap(_x11_display, screen);
		XAllocNamedColor(_x11_display, colormap, "black", &black, &dummy);
		bm_no = XCreateBitmapFromData(_x11_display, _x11_window, no_data, 8, 8);
		_x11_hidden_cursor = XCreatePixmapCursor(_x11_display, bm_no, bm_no, &black, &black, 0, 0);

		_wm_delete_message = XInternAtom(_x11_display, "WM_DELETE_WINDOW", False);
		XSetWMProtocols(_x11_display, _x11_window, &_wm_delete_message, 1);

		XMapRaised(_x11_display, _x11_window);
	}

	void close()
	{
		XDestroyWindow(_x11_display, _x11_window);
	}

	void bgfx_setup()
	{
		bgfx::x11SetDisplayWindow(_x11_display, _x11_window);
	}

	void show()
	{

	}

	void hide()
	{

	}

	void resize(uint16_t width, uint16_t height)
	{

	}

	void move(uint16_t x, uint16_t y)
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

	}

	void set_title (const char* /*title*/)
	{

	}

	void* handle()
	{
		return (void*)(uintptr_t)_x11_window;
	}
};

Window* Window::create(Allocator& a)
{
	return CE_NEW(a, WindowX11)();
}

void Window::destroy(Allocator& a, Window& w)
{
	CE_DELETE(a, &w);
}

} // namespace crown

#endif // CROWN_PLATFORM_LINUX
