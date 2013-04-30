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

#include "Device.h"
#include "GLESSupport.h"
#include "EGLRenderWindow.h"
#include "Log.h"
#include "Types.h"
#include "Config.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>

namespace crown
{

EGLRenderWindow::EGLRenderWindow() :
	mEGLDisplay(EGL_NO_DISPLAY),
	mEGLContext(EGL_NO_CONTEXT),
	mEGLWindow(EGL_NO_SURFACE)
{
	mXDisplay = XOpenDisplay(NULL);
	mEGLDisplay = eglGetDisplay((EGLNativeDisplayType) mXDisplay);

	eglInitialize(mEGLDisplay, NULL, NULL);
}

EGLRenderWindow::~EGLRenderWindow()
{
	if (mEGLDisplay != EGL_NO_DISPLAY)
	{
		Log::d("EGLRenderWindow::Destroy: Releasing context...");
		if (mEGLContext != EGL_NO_CONTEXT)
		{
			eglMakeCurrent(mEGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
			eglDestroyContext(mEGLDisplay, mEGLContext);
		}
		Log::d("EGLRenderWindow::Destroy: Context released.");

		if (mEGLWindow != EGL_NO_SURFACE)
		{
			eglDestroySurface(mEGLDisplay, mEGLWindow);
		}

		mEGLWindow = EGL_NO_SURFACE;
		Log::d("EGLRenderWindow::Destroy: Window Destroyed.");

		eglTerminate(mEGLDisplay);
	}

	if (mXDisplay)
	{
		XCloseDisplay(mXDisplay);
	}
}

bool EGLRenderWindow::Create(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t depth, bool /*fullscreen*/)
{
	Log::d("EGLRenderWindow::Create: Creating window...");
	if (!width || !height)
	{
		Log::e("Width and height must differ from 0.");
		return false;
	}

	if (!mEGLDisplay)
	{
		Log::e("Unable to open a display");
		return false;
	}

	XSetWindowAttributes winAttribs;
	winAttribs.event_mask = FocusChangeMask | StructureNotifyMask;

	mXWindow = XCreateWindow(
				   mXDisplay, DefaultRootWindow(mXDisplay),
				   x, y, width, height, 0,
				   CopyFromParent, InputOutput,
				   CopyFromParent, CWEventMask,
				   &winAttribs);

	if (!mXWindow)
	{
		Log::e("Unable to create the X Window.");
		return false;
	}

	XSetWindowAttributes  xattr;

	xattr.override_redirect = False;
	XChangeWindowAttributes (mXDisplay, mXWindow, CWOverrideRedirect, &xattr);

	XWMHint32_ts hint32_ts;
	hint32_ts.input = True;
	hint32_ts.flags = InputHint32_t;
	XSetWMHint32_ts(mXDisplay, mXWindow, &hint32_ts);

	XMapRaised(mXDisplay, mXWindow);
	XStoreName (mXDisplay, mXWindow, "GL test" );

	uint32_t bpp			= depth / 4;

	const EGLint attribs[] =
	{
		EGL_BUFFER_SIZE, 24,
		EGL_DEPTH_SIZE, 24,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
		EGL_NONE
	};

	EGLConfig  ecfg;
	EGLint     num_config;
	if (!eglChooseConfig(mEGLDisplay, attribs, &ecfg, 1, &num_config))
	{
		Log::e("Unable to choose config.");
		return false;
	}

	mEGLWindow = eglCreateWindowSurface(mEGLDisplay, ecfg, mXWindow, NULL);
	if (mEGLWindow == EGL_NO_SURFACE)
	{
		Log::e("Unable to create window surface.");
		return false;
	}

	EGLint ctxattr[] =
	{
		EGL_CONTEXT_CLIENT_VERSION, 1,
		EGL_NONE
	};

	mEGLContext = eglCreateContext(mEGLDisplay, ecfg, EGL_NO_CONTEXT, ctxattr);
	if (mEGLContext == EGL_NO_CONTEXT)
	{
		Log::e("Unable to create context: " + Str(eglGetError()));
		return false;
	}

	eglMakeCurrent(mEGLDisplay, mEGLWindow, mEGLWindow, mEGLContext);

	EGLint w, h;
	eglQuerySurface(mEGLDisplay, mEGLWindow, EGL_WIDTH, &w);
	eglQuerySurface(mEGLDisplay, mEGLWindow, EGL_HEIGHT, &h);

	mX = x;
	mY = y;
	mWidth = w;
	mHeight = h;

	mCreated = true;

	Log::d("EGLRenderWindow::Create: Window created.");

	return true;
}

void EGLRenderWindow::Destroy()
{
	if (!mCreated)
	{
		return;
	}

	// Main window can not be destroyed
	if (mMain)
	{
		return;
	}

	if (mFull)
	{
		SetFullscreen(false);
	}

	mCreated = false;
}

void EGLRenderWindow::SetVisible(bool visible)
{
	mVisible = visible;
}

void EGLRenderWindow::Move(uint32_t x, uint32_t y)
{
	if (x == mX && y == mY)
	{
		return;
	}

	XMoveWindow(mXDisplay, mXWindow, x, y);
}

void EGLRenderWindow::Resize(uint32_t width, uint32_t height)
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

void EGLRenderWindow::SetFullscreen(bool full)
{
	/* No action */
}

void EGLRenderWindow::Bind()
{
	eglMakeCurrent(mEGLDisplay, mEGLWindow, mEGLWindow, mEGLContext);
}

void EGLRenderWindow::Unbind()
{
	eglMakeCurrent(mEGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

void EGLRenderWindow::Update()
{
	eglSwapBuffers(mEGLDisplay, mEGLWindow);
}

void EGLRenderWindow::EventLoop()
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

void EGLRenderWindow::_NotifyMetricsChange(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	if (x == mX && y == mY && width == mWidth && height == mHeight)
	{
		return;
	}

	XWindowAttributes attribs;
	XGetWindowAttributes(mXDisplay, mXWindow, &attribs);

	if (attribs.x == (int32_t)x && attribs.y == (int32_t)y)
	{
		mX = x;
		mY = y;
	}

	if (attribs.width == (int32_t)width && attribs.height == (int32_t)height)
	{
		mWidth = width;
		mHeight = height;
	}
}

void EGLRenderWindow::_SetTitleAndAdditionalTextToWindow()
{
	Str tmp = GetDisplayedTitle();
	const char* ctitle = tmp.c_str();
	XTextProperty textProperty;
	XStringListToTextProperty((char**)&ctitle, 1, &textProperty);
	XSetWMName(mXDisplay, mXWindow, &textProperty);
	XFree(textProperty.value);
}

} // namespace crown

