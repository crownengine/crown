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

#include "X11Mouse.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "Exceptions.h"
#include "X11InputManager.h"

namespace Crown
{

//-----------------------------------------------------------------------------
X11Mouse::X11Mouse(InputManager* creator) :
	Mouse(creator),
	mIsCursorVisible(true),
	mDisplay(NULL),
	mXWindow(0),
	mHiddenCursor(0)
{
	// Connect to the X Server
	mDisplay = XOpenDisplay(NULL);

	if (!mDisplay)
	{
		throw InvalidOperationException("X11Mouse::X11Mouse: Unable to connect to the X Server.");
	}

	if (mCreator)
	{
		mXWindow = static_cast<X11InputManager*>(mCreator)->GetXWindow();
	}

	// We want to track motion and button pressed/released events
	if (XSelectInput(mDisplay, mXWindow, ButtonPressMask | ButtonReleaseMask | PointerMotionMask) == BadWindow)
	{
		throw InvalidOperationException("X11Mouse::X11Mouse: Unable to select input, bad window.");
	}

	// Build hidden cursor
	Pixmap bm_no;
	XColor black, dummy;
	Colormap colormap;
	static char no_data[] = { 0,0,0,0,0,0,0,0 };

	colormap = DefaultColormap(mDisplay, DefaultScreen(mDisplay));
	XAllocNamedColor(mDisplay, colormap, "black", &black, &dummy);
	bm_no = XCreateBitmapFromData(mDisplay, mXWindow, no_data, 8, 8);
	mHiddenCursor = XCreatePixmapCursor(mDisplay, bm_no, bm_no, &black, &black, 0, 0);

	static_cast<X11InputManager*>(mCreator)->SetMouseAvailable(true);
}

//-----------------------------------------------------------------------------
X11Mouse::~X11Mouse()
{
	if (mCreator)
	{
		static_cast<X11InputManager*>(mCreator)->SetMouseAvailable(false);
	}

	if (!IsCursorVisible())
	{
		SetCursorVisible(true);
	}

	if (mHiddenCursor)
	{
		XFreeCursor(mDisplay, mHiddenCursor);
	}

	if (mDisplay)
	{
		XCloseDisplay(mDisplay);
	}
}

//-----------------------------------------------------------------------------
bool X11Mouse::IsCursorVisible() const
{
	return mIsCursorVisible;
}

//-----------------------------------------------------------------------------
void X11Mouse::SetCursorVisible(bool visible)
{
	if (visible)
	{
		XUndefineCursor(mDisplay, mXWindow);
	}
	else
	{
		XDefineCursor(mDisplay, mXWindow, mHiddenCursor);
	}

	mIsCursorVisible = visible;
}

//-----------------------------------------------------------------------------
Point2 X11Mouse::GetCursorXY() const
{
	Window unused;
	int x, y, dummy;
	uint dummy2;

	XQueryPointer(mDisplay, mXWindow, &unused, &unused, &dummy, &dummy, &x, &y, &dummy2);

	return Point2(x, y);
}

//-----------------------------------------------------------------------------
void X11Mouse::SetCursorXY(const Point2& position)
{
	XWindowAttributes attribs;
	XGetWindowAttributes(mDisplay, mXWindow, &attribs);

	XWarpPointer(mDisplay, None, mXWindow, 0, 0, attribs.width, attribs.height, position.x, position.y);

	XFlush(mDisplay);
}

//-----------------------------------------------------------------------------
Vec2 X11Mouse::GetCursorRelativeXY() const
{
	XWindowAttributes attribs;
	XGetWindowAttributes(mDisplay, mXWindow, &attribs);

	Vec2 pos = GetCursorXY().ToVec2();

	pos.x = pos.x / (float) attribs.width;
	pos.y = pos.y / (float) attribs.height;

	return pos;
}

//-----------------------------------------------------------------------------
void X11Mouse::SetCursorRelativeXY(const Vec2& position)
{
	XWindowAttributes attribs;
	XGetWindowAttributes(mDisplay, mXWindow, &attribs);

	SetCursorXY(Point2((int)(position.x * (float) attribs.width), (int)(position.y * (float) attribs.height)));
}

//-----------------------------------------------------------------------------
void X11Mouse::EventLoop()
{
	XEvent event;
	MouseEvent mouseEvent;

	while (XPending(mDisplay))
	{
		XNextEvent(mDisplay, &event);

		switch (event.type)
		{
			case ButtonPress:
			case ButtonRelease:
			{
				switch (event.xbutton.button)
				{
					case Button1:
					{
						mouseEvent.button = MB_LEFT;
						break;
					}
					case Button2:
					{
						mouseEvent.button = MB_MIDDLE;
						break;
					}
					case Button3:
					{
						mouseEvent.button = MB_RIGHT;
						break;
					}
				}

				mouseEvent.x = event.xbutton.x;
				mouseEvent.y = event.xbutton.y;

				if (mListener)
				{
					if (event.type == ButtonPress)
					{
						mListener->ButtonPressed(mouseEvent);
					}
					else if (event.type == ButtonRelease)
					{
						mListener->ButtonReleased(mouseEvent);
					}
				}

				break;
			}
			case MotionNotify:
			{
				mouseEvent.x = event.xbutton.x;
				mouseEvent.y = event.xbutton.y;

				if (mListener)
				{
					mListener->CursorMoved(mouseEvent);
				}

				break;
			}
		}
	}
}

} // namespace Crown

