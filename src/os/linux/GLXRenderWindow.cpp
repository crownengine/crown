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

#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include "GLXRenderWindow.h"
#include "Log.h"

namespace Crown
{

//-----------------------------------------------------------------------------
GLXRenderWindow::GLXRenderWindow() :
	mXDisplay(NULL),
	mXWindow(None),
	mGLXContext(NULL),
	mGLXWindow(None)
{
	mXDisplay = XOpenDisplay(NULL);
}

//-----------------------------------------------------------------------------
GLXRenderWindow::~GLXRenderWindow()
{
	if (mXDisplay)
	{
		if (mGLXWindow)
		{
			glXDestroyWindow(mXDisplay, mGLXWindow);
		}

		if (mGLXContext)
		{
			glXMakeContextCurrent(mXDisplay, None, None, NULL);
			glXDestroyContext(mXDisplay, mGLXContext);
		}

		if (mXWindow)
		{
			XDestroyWindow(mXDisplay, mXWindow);
		}

		XCloseDisplay(mXDisplay);
	}
}

//-----------------------------------------------------------------------------
bool GLXRenderWindow::Create(uint x, uint y, uint width, uint height, uint depth, bool /*fullscreen*/)
{
	if (!width || !height)
	{
		Log::E("Width and height must differ from 0.");
		return false;
	}

	if (!mXDisplay)
	{
		Log::E("Unable to open a display");
		return false;
	}

	Window defRoot = DefaultRootWindow(mXDisplay);

	// Color index buffer not supported - deprecated
	int fbAttribs[] =
	{
		GLX_DOUBLEBUFFER,		True,			// Only double-buffered
		GLX_RED_SIZE,			(int)depth / 4,
		GLX_GREEN_SIZE,			(int)depth / 4,
		GLX_BLUE_SIZE,			(int)depth / 4,
		GLX_ALPHA_SIZE,			(int)depth / 4,
		GLX_DEPTH_SIZE,			24,				// Depth buffer size
		GLX_STENCIL_SIZE,		0,				// Stencil buffer size
		GLX_ACCUM_RED_SIZE,		0,
		GLX_ACCUM_GREEN_SIZE,	0,
		GLX_ACCUM_BLUE_SIZE,	0,
		GLX_ACCUM_ALPHA_SIZE,	0,
		GLX_RENDER_TYPE,		GLX_RGBA_BIT,	// The default framebuffer is always RGBA
		GLX_DRAWABLE_TYPE,		GLX_WINDOW_BIT,
		GLX_X_RENDERABLE,		True,
		GLX_CONFIG_CAVEAT,		GLX_DONT_CARE,
		GLX_TRANSPARENT_TYPE,	GLX_NONE,
		None
	};

	int fbCount;
	GLXFBConfig* fbConfig = glXChooseFBConfig(mXDisplay, XDefaultScreen(mXDisplay), fbAttribs, &fbCount);

	if (!fbConfig)
	{
		Log::E("Unable to find a matching FrameBuffer configuration.");
		return false;
	}

	XVisualInfo* visualInfo = glXGetVisualFromFBConfig(mXDisplay, fbConfig[0]);

	if (!visualInfo)
	{
		Log::E("Unable to find a matching Visual for the FrameBuffer configuration.");
		XFree(fbConfig);
		return false;
	}

	Colormap cmap;
	cmap = XCreateColormap(mXDisplay, defRoot, visualInfo->visual, AllocNone);
	XSetWindowAttributes winAttribs;
	winAttribs.colormap = cmap;
	winAttribs.event_mask = FocusChangeMask | StructureNotifyMask;

	mXWindow = XCreateWindow(
				   mXDisplay,
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

	if (!mXWindow)
	{
		Log::E("Unable to create the X Window.");
		return false;
	}

	XMapRaised(mXDisplay, mXWindow);
	mGLXWindow = glXCreateWindow(mXDisplay, fbConfig[0], mXWindow, 0);

	mGLXContext = glXCreateNewContext(mXDisplay, fbConfig[0], GLX_RGBA_TYPE, NULL, True);
	glXMakeContextCurrent(mXDisplay, mGLXWindow, mGLXWindow, mGLXContext);

	XFreeColormap(mXDisplay, cmap);
	XFree(visualInfo);
	XFree(fbConfig);
	XFlush(mXDisplay);

	mX = x;
	mY = y;
	mWidth = width;
	mHeight = height;
	mCreated = true;

	return true;
}

//-----------------------------------------------------------------------------
void GLXRenderWindow::Destroy()
{
	// Main window can not be destroyed
	if (!mCreated || mMain)
	{
		return;
	}

	if (mFull)
	{
		SetFullscreen(false);
	}

	mCreated = false;
}

//-----------------------------------------------------------------------------
void GLXRenderWindow::SetVisible(bool visible)
{
	mVisible = visible;
}

//-----------------------------------------------------------------------------
void GLXRenderWindow::Move(uint x, uint y)
{
	if (x == mX && y == mY)
	{
		return;
	}

	XMoveWindow(mXDisplay, mXWindow, x, y);
}

//-----------------------------------------------------------------------------
void GLXRenderWindow::Resize(uint width, uint height)
{
	if (!width || !height)
	{
		return;
	}

	if (width == mWidth && height == mHeight)
	{
		return;
	}

	XResizeWindow(mXDisplay, mXWindow, width, height);
}

//-----------------------------------------------------------------------------
void GLXRenderWindow::SetFullscreen(bool full)
{
	mFull = full;
	XEvent xEvent;
	Atom wmState = XInternAtom(mXDisplay, "_NET_WM_STATE", False);
	Atom fullscreen = XInternAtom(mXDisplay, "_NET_WM_STATE_FULLSCREEN", False);
	xEvent.type = ClientMessage;
	xEvent.xclient.window = mXWindow;
	xEvent.xclient.message_type = wmState;
	xEvent.xclient.format = 32;
	xEvent.xclient.data.l[0] = (mFull ? 1 : 0);
	xEvent.xclient.data.l[1] = fullscreen;
	xEvent.xclient.data.l[2] = 0;
	XSendEvent(mXDisplay, DefaultRootWindow(mXDisplay), False, SubstructureNotifyMask, &xEvent);
}

//-----------------------------------------------------------------------------
void GLXRenderWindow::Bind()
{
	glXMakeContextCurrent(mXDisplay, mGLXWindow, mGLXWindow, mGLXContext);
}

//-----------------------------------------------------------------------------
void GLXRenderWindow::Unbind()
{
	glXMakeContextCurrent(mXDisplay, None, None, NULL);
}

//-----------------------------------------------------------------------------
void GLXRenderWindow::Update()
{
	glXSwapBuffers(mXDisplay, mGLXWindow);
}

//-----------------------------------------------------------------------------
void GLXRenderWindow::_NotifyMetricsChange(uint x, uint y, uint width, uint height)
{
	if (x == mX && y == mY && width == mWidth && height == mHeight)
	{
		return;
	}

	XWindowAttributes attribs;
	XGetWindowAttributes(mXDisplay, mXWindow, &attribs);

	if (attribs.x == (int)x && attribs.y == (int)y)
	{
		mX = x;
		mY = y;
	}

	if (attribs.width == (int)width && attribs.height == (int)height)
	{
		mWidth = width;
		mHeight = height;
	}
}

//-----------------------------------------------------------------------------
void GLXRenderWindow::EventLoop()
{
	XEvent event;

	while (XPending(mXDisplay))
	{
		XNextEvent(mXDisplay, &event);

		switch (event.type)
		{
			case ConfigureNotify:
			{
				_NotifyMetricsChange(event.xconfigure.x, event.xconfigure.y,
										event.xconfigure.width, event.xconfigure.height);
				break;
			}
			default:
			{
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void GLXRenderWindow::_SetTitleAndAdditionalTextToWindow()
{
	Str tmp = GetDisplayedTitle();
	const char* ctitle = tmp.c_str();
	XTextProperty textProperty;
	XStringListToTextProperty((char**)&ctitle, 1, &textProperty);
	XSetWMName(mXDisplay, mXWindow, &textProperty);
	XFree(textProperty.value);
}

} // namespace Crown

