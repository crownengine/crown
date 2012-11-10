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
#include <X11/Xlib.h>
#include <GL/glx.h>
#include "Types.h"
#include "Log.h"
#include "OS.h"

namespace crown
{

namespace os
{

Display*		mXDisplay = NULL;
Window			mXWindow = None;
GLXContext		mGLXContext = NULL;
GLXDrawable		mGLXWindow = None;

//-----------------------------------------------------------------------------
bool create_render_window(uint x, uint y, uint width, uint height, bool fullscreen)
{
	assert(width != 0 && height != 0);

	mXDisplay = XOpenDisplay(NULL);

	if (mXDisplay == NULL)
	{
		Log::E("Unable to open a display");
		return false;
	}

	Window defRoot = DefaultRootWindow(mXDisplay);

	// Color index buffer not supported - deprecated
	int fbAttribs[] =
	{
		GLX_DOUBLEBUFFER,		True,			// Only double-buffered
		GLX_RED_SIZE,			8,
		GLX_GREEN_SIZE,			8,
		GLX_BLUE_SIZE,			8,
		GLX_ALPHA_SIZE,			8,
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

	return true;
}

//-----------------------------------------------------------------------------
bool destroy_render_window()
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
void event_loop()
{
	XEvent event;

	while (XPending(mXDisplay))
	{
		XNextEvent(mXDisplay, &event);

		switch (event.type)
		{
//			case ConfigureNotify:
//			{
//				_NotifyMetricsChange(event.xconfigure.x, event.xconfigure.y,
//										event.xconfigure.width, event.xconfigure.height);
//				break;
//			}
			case ButtonPress:
			{
				switch (event.xbutton.button)
				{
					case Button1:
					{
						push_event(os::OSET_MOUSE, os::OSMET_LEFT_PRESSED, event.xbutton.x, event.xbutton.y, 0);
						break;
					}
					case Button2:
					{
						push_event(os::OSET_MOUSE, os::OSMET_MIDDLE_PRESSED, event.xbutton.x, event.xbutton.y, 0);
						break;
					}
					case Button3:
					{
						push_event(os::OSET_MOUSE, os::OSMET_RIGHT_PRESSED, event.xbutton.x, event.xbutton.y, 0);
						break;
					}
				}

				break;
			}
			case ButtonRelease:
			{
				switch (event.xbutton.button)
				{
					case Button1:
					{
						push_event(os::OSET_MOUSE, os::OSMET_LEFT_PRESSED, event.xbutton.x, event.xbutton.y, 0);
						break;
					}
					case Button2:
					{
						push_event(os::OSET_MOUSE, os::OSMET_MIDDLE_PRESSED, event.xbutton.x, event.xbutton.y, 0);
						break;
					}
					case Button3:
					{
						push_event(os::OSET_MOUSE, os::OSMET_RIGHT_PRESSED, event.xbutton.x, event.xbutton.y, 0);
						break;
					}
				}

				break;
			}
			case MotionNotify:
			{
				push_event(os::OSET_MOUSE, os::OSMET_CURSOR_MOVED, event.xbutton.x, event.xbutton.y, 0);
				break;
			}
//			case KeyPress:
//			case KeyRelease:
//			{
//				char string[4] = {0, 0, 0, 0};
//				int len = -1;
//				KeySym key;

//				len = XLookupString(&event.xkey, string, 4, &key, NULL);

//				Key kc = TranslateKey(key);

//				// Check if any modifier key is pressed or released
//				if (kc == KC_LSHIFT || kc == KC_RSHIFT)
//				{
//					(event.type == KeyPress) ? mModifierMask |= MK_SHIFT : mModifierMask &= ~MK_SHIFT;
//				}
//				else if (kc == KC_LCONTROL || kc == KC_RCONTROL)
//				{
//					(event.type == KeyPress) ? mModifierMask |= MK_CTRL : mModifierMask &= ~MK_CTRL;
//				}
//				else if (kc == KC_LALT || kc == KC_RALT)
//				{
//					(event.type == KeyPress) ? mModifierMask |= MK_ALT : mModifierMask &= ~MK_ALT;
//				}

//				mKeyState[kc] = (event.type == KeyPress) ? true : false;
//				keyboardEvent.key = kc;

//				if (mListener)
//				{
//					if (event.type == KeyPress)
//					{
//						mListener->KeyPressed(keyboardEvent);
//					}
//					else if (event.type == KeyRelease)
//					{
//						mListener->KeyReleased(keyboardEvent);
//					}
//				}

//				if (event.type == KeyPress)
//				{
//					push_event(os::OSET_KEYBOARD, 1, 2, 3, 4);
//				}
//				else if (event.type == KeyRelease)
//				{
//					push_event(os::OSET_KEYBOARD, 55, 2, 3, 4);
//				}

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

//				break;
//			}
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

//-----------------------------------------------------------------------------
void swap_buffers()
{
	glXSwapBuffers(mXDisplay, mGLXWindow);
}

////-----------------------------------------------------------------------------
//void GLXRenderWindow::Move(uint x, uint y)
//{
//	if (x == mX && y == mY)
//	{
//		return;
//	}

//	XMoveWindow(mXDisplay, mXWindow, x, y);
//}

////-----------------------------------------------------------------------------
//void GLXRenderWindow::Resize(uint width, uint height)
//{
//	if (!width || !height)
//	{
//		return;
//	}

//	if (width == mWidth && height == mHeight)
//	{
//		return;
//	}

//	XResizeWindow(mXDisplay, mXWindow, width, height);
//}

////-----------------------------------------------------------------------------
//void GLXRenderWindow::SetFullscreen(bool full)
//{
//	mFull = full;
//	XEvent xEvent;
//	Atom wmState = XInternAtom(mXDisplay, "_NET_WM_STATE", False);
//	Atom fullscreen = XInternAtom(mXDisplay, "_NET_WM_STATE_FULLSCREEN", False);
//	xEvent.type = ClientMessage;
//	xEvent.xclient.window = mXWindow;
//	xEvent.xclient.message_type = wmState;
//	xEvent.xclient.format = 32;
//	xEvent.xclient.data.l[0] = (mFull ? 1 : 0);
//	xEvent.xclient.data.l[1] = fullscreen;
//	xEvent.xclient.data.l[2] = 0;
//	XSendEvent(mXDisplay, DefaultRootWindow(mXDisplay), False, SubstructureNotifyMask, &xEvent);
//}

} // namespace os

} // namespace crown

