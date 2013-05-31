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
#include <GL/glx.h>
#include "Types.h"
#include "Log.h"
#include "OS.h"

namespace crown
{

namespace os
{

Display*		display = NULL;
Window			window = None;
GLXContext		glx_context = NULL;
GLXDrawable		glx_window = None;
uint32_t		window_x = 0;
uint32_t		window_y = 0;
uint32_t		window_width = 0;
uint32_t		window_height = 0;

//-----------------------------------------------------------------------------
bool create_render_window(uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool fullscreen)
{
	(void)fullscreen;

	CE_ASSERT(width != 0 && height != 0, "Width and height must be != 0");

	display = XOpenDisplay(NULL);

	if (display == NULL)
	{
		Log::e("Unable to open a display");
		return false;
	}

	Window defRoot = DefaultRootWindow(display);

	// Color index buffer not supported - deprecated
	int32_t fbAttribs[] =
	{
		GLX_DOUBLEBUFFER,		static_cast<int32_t>(True),
		GLX_RED_SIZE,			8,
		GLX_GREEN_SIZE,			8,
		GLX_BLUE_SIZE,			8,
		GLX_ALPHA_SIZE,			8,
		GLX_DEPTH_SIZE,			24,
		GLX_STENCIL_SIZE,		0,
		GLX_ACCUM_RED_SIZE,		0,
		GLX_ACCUM_GREEN_SIZE,	0,
		GLX_ACCUM_BLUE_SIZE,	0,
		GLX_ACCUM_ALPHA_SIZE,	0,
		GLX_RENDER_TYPE,		static_cast<int32_t>(GLX_RGBA_BIT),
		GLX_DRAWABLE_TYPE,		static_cast<int32_t>(GLX_WINDOW_BIT),
		GLX_X_RENDERABLE,		static_cast<int32_t>(True),
		GLX_CONFIG_CAVEAT,		static_cast<int32_t>(GLX_DONT_CARE),
		GLX_TRANSPARENT_TYPE,	static_cast<int32_t>(GLX_NONE),
		static_cast<int32_t>(None)
	};

	int32_t fbCount;
	GLXFBConfig* fbConfig = glXChooseFBConfig(display, XDefaultScreen(display), fbAttribs, &fbCount);

	if (!fbConfig)
	{
		Log::e("Unable to find a matching FrameBuffer configuration.");
		return false;
	}

	XVisualInfo* visualInfo = glXGetVisualFromFBConfig(display, fbConfig[0]);

	if (!visualInfo)
	{
		Log::e("Unable to find a matching Visual for the FrameBuffer configuration.");
		XFree(fbConfig);
		return false;
	}

	Colormap cmap;
	cmap = XCreateColormap(display, defRoot, visualInfo->visual, AllocNone);
	XSetWindowAttributes winAttribs;
	winAttribs.colormap = cmap;
	winAttribs.event_mask = FocusChangeMask | StructureNotifyMask;

	window = XCreateWindow(
				   display,
				   defRoot,
				   x, y,
				   width, height,
				   0,
				   visualInfo->depth,
				   InputOutput,
				   visualInfo->visual,
				   CWColormap | CWEventMask,
				   &winAttribs
			   );

	if (!window)
	{
		Log::e("Unable to create the X Window.");
		return false;
	}

	XMapRaised(display, window);
	glx_window = glXCreateWindow(display, fbConfig[0], window, 0);

	glx_context = glXCreateNewContext(display, fbConfig[0], GLX_RGBA_TYPE, NULL, True);
	glXMakeContextCurrent(display, glx_window, glx_window, glx_context);

	XFreeColormap(display, cmap);
	XFree(visualInfo);
	XFree(fbConfig);
	XFlush(display);

	window_x = x;
	window_y = y;
	window_width = width;
	window_height = height;

	return true;
}

//-----------------------------------------------------------------------------
bool destroy_render_window()
{
	if (display)
	{
		if (glx_window)
		{
			glXDestroyWindow(display, glx_window);
		}

		if (glx_context)
		{
			glXMakeContextCurrent(display, None, None, NULL);
			glXDestroyContext(display, glx_context);
		}

		if (window)
		{
			XDestroyWindow(display, window);
		}

		XCloseDisplay(display);
	}

	return true;
}

//-----------------------------------------------------------------------------
void swap_buffers()
{
	glXSwapBuffers(display, glx_window);
}

//-----------------------------------------------------------------------------
void get_render_window_metrics(uint32_t& width, uint32_t& height)
{
	XWindowAttributes attribs;
	XGetWindowAttributes(display, window, &attribs);

	XFlush(display);

	window_width = attribs.width;
	window_height = attribs.height;

	width = window_width;
	height = window_height;
}

////-----------------------------------------------------------------------------
//void GLXRenderWindow::Move(uint32_t x, uint32_t y)
//{
//	if (x == mX && y == mY)
//	{
//		return;
//	}

//	XMoveWindow(display, window, x, y);
//}

////-----------------------------------------------------------------------------
//void GLXRenderWindow::Resize(uint32_t width, uint32_t height)
//{
//	if (!width || !height)
//	{
//		return;
//	}

//	if (width == mWidth && height == mHeight)
//	{
//		return;
//	}

//	XResizeWindow(display, window, width, height);
//}

////-----------------------------------------------------------------------------
//void GLXRenderWindow::SetFullscreen(bool full)
//{
//	mFull = full;
//	XEvent xEvent;
//	Atom wmState = XInternAtom(display, "_NET_WM_STATE", False);
//	Atom fullscreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);
//	xEvent.type = ClientMessage;
//	xEvent.xclient.window = window;
//	xEvent.xclient.message_type = wmState;
//	xEvent.xclient.format = 32;
//	xEvent.xclient.data.l[0] = (mFull ? 1 : 0);
//	xEvent.xclient.data.l[1] = fullscreen;
//	xEvent.xclient.data.l[2] = 0;
//	XSendEvent(display, DefaultRootWindow(display), False, SubstructureNotifyMask, &xEvent);
//}

} // namespace os

} // namespace crown

