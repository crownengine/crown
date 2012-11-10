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

#include "WinMouse.h"
#include "Exceptions.h"
#include "WinInputManager.h"

// Undef functions to avoid conflicts
#undef MB_RIGHT

namespace crown
{

//-----------------------------------------------------------------------------
WinMouse::WinMouse(InputManager* creator) :
	Mouse(creator),
	mIsCursorVisible(true),
	mWindowHandle(0)
{
	if (mCreator)
	{
		mWindowHandle = (HWND)(static_cast<WinInputManager*>(mCreator)->GetWindowHandle());
	}

	static_cast<WinInputManager*>(mCreator)->SetMouseAvailable(true);
}

//-----------------------------------------------------------------------------
WinMouse::~WinMouse()
{
	if (mCreator)
	{
		static_cast<WinInputManager*>(mCreator)->SetMouseAvailable(false);
	}

	if (!IsCursorVisible())
	{
		SetCursorVisible(true);
	}
}

//-----------------------------------------------------------------------------
bool WinMouse::IsCursorVisible() const
{
	return mIsCursorVisible;
}

//-----------------------------------------------------------------------------
void WinMouse::SetCursorVisible(bool visible)
{
	mIsCursorVisible = visible;
	ShowCursor(visible);
}

//-----------------------------------------------------------------------------
Point2 WinMouse::GetCursorXY() const
{
	Point2 tmp;
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(mWindowHandle, &p);
	tmp.x = p.x;
	tmp.y = p.y;
	return tmp;
}

//-----------------------------------------------------------------------------
void WinMouse::SetCursorXY(const Point2& position)
{
	uint width, height;
	RECT windowRect;
	GetWindowRect(mWindowHandle, &windowRect);

	width = windowRect.right - windowRect.left;
	height = windowRect.bottom - windowRect.top;

	POINT p = {(LONG)position.x, (LONG)position.y};

	if (p.x > (int)width)
	{
		p.x = width;
	}
	else
	{
		p.x = (LONG)position.x;
	}

	if (p.y > (int)height)
	{
		p.y = height;
	}
	else
	{
		p.y = (LONG)position.y;
	}

	ClientToScreen(mWindowHandle, &p);
	SetCursorPos(p.x, p.y);
}

//-----------------------------------------------------------------------------
Vec2 WinMouse::GetCursorRelativeXY() const
{
	uint width, height;
	RECT windowRect;
	GetWindowRect(mWindowHandle, &windowRect);

	width = windowRect.right - windowRect.left;
	height = windowRect.bottom - windowRect.top;

	Vec2 pos;
	pos = GetCursorXY().ToVec2();
	pos.x = 1.0f / (float)width * pos.x;
	pos.y = 1.0f / (float)height * pos.y;

	return pos;
}

//-----------------------------------------------------------------------------
void WinMouse::SetCursorRelativeXY(const Vec2& position)
{
	uint width, height;
	RECT windowRect;
	GetWindowRect(mWindowHandle, &windowRect);

	width = windowRect.right - windowRect.left;
	height = windowRect.bottom - windowRect.top;

	SetCursorXY(Point2((int)(position.x * width), (int)(position.y * height)));
}

//-----------------------------------------------------------------------------
void WinMouse::EventLoop()
{
	MSG msg;
	while (PeekMessage(&msg, mWindowHandle, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		switch (msg.message)
		{
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
			{
				MouseEvent mouseEvent;

				if (msg.message == WM_LBUTTONDOWN || msg.message == WM_LBUTTONUP)
				{
					mouseEvent.button = MB_LEFT;
				}
				else if (msg.message == WM_RBUTTONDOWN || msg.message == WM_RBUTTONUP)
				{
					mouseEvent.button = MB_RIGHT;
				}
				else if (msg.message == WM_MBUTTONDOWN || msg.message == WM_MBUTTONUP)
				{
					mouseEvent.button = MB_MIDDLE;
				}

				mouseEvent.x = LOWORD(msg.lParam);
				mouseEvent.y = HIWORD(msg.lParam);

				if (mListener)
				{
					if (msg.message == WM_LBUTTONDOWN || msg.message == WM_RBUTTONDOWN || msg.message == WM_MBUTTONDOWN)
					{
						mListener->ButtonPressed(mouseEvent);
					}
					else if (msg.message == WM_LBUTTONUP || msg.message == WM_RBUTTONUP || msg.message == WM_MBUTTONUP)
					{
						mListener->ButtonReleased(mouseEvent);
					}
				}
			}break;
			case WM_MOUSEMOVE:
			{
				MouseEvent mouseEvent;

				mouseEvent.x = LOWORD(msg.lParam);
				mouseEvent.y = HIWORD(msg.lParam);

				if (mListener)
				{
					mListener->CursorMoved(mouseEvent);
				}
			}break;
		}
	}
}

} // namespace crown

