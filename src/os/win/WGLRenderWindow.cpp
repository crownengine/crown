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

#include "WGLRenderWindow.h"
#include "Types.h"
#include "Log.h"
#include "resources.h"

namespace crown
{

char WGLRenderWindow::mWindowClassName[20] = "";

WGLRenderWindow::WGLRenderWindow() :
	mWindowHandle(0),
	mRC(0),
	mDC(0),
	mCreated(false)
{
}

WGLRenderWindow::~WGLRenderWindow()
{
	if (mRC)
	{
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(mRC);
	}

	if (mWindowHandle)
	{
		::DestroyWindow(mWindowHandle);
	}
}

bool WGLRenderWindow::Create(uint x, uint y, uint width, uint height,
							 uint depth, bool fullscreen)
{
	if (!width || !height)
	{
		Log::E("Width and height must differ from 0.");
		return false;
	}

	if (fullscreen)
	{
		DEVMODE dmScreenSettings; // Device Mode
		memset(&dmScreenSettings, 0, sizeof (dmScreenSettings)); // Makes Sure Memory's Cleared
		dmScreenSettings.dmSize = sizeof (dmScreenSettings); // Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth = width; // Selected Screen Width
		dmScreenSettings.dmPelsHeight = height; // Selected Screen Height
		dmScreenSettings.dmBitsPerPel = 32; // Selected Bits Per Pixel
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			fullscreen = false;
			Log::I("Fullscreen resolution not supported, switching to windowed mode.");
		}
	}

	strcpy(mWindowClassName, "CrownWindowClass");
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof (WNDCLASSEX);
	wcex.style = CS_OWNDC;
	wcex.lpfnWndProc = (WNDPROC) DefWindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = GetModuleHandle(NULL);
	wcex.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_CROWNICON));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = mWindowClassName;
	wcex.hIconSm = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_CROWNICON));

	if (!RegisterClassEx(&wcex))
	{
		Log::E("Unable to register a Window Class.");
		return false;
	}

	if (fullscreen)
	{
		mWindowHandle = CreateWindowEx(0, mWindowClassName, "", WS_POPUP, 0, 0, width, height, NULL, NULL, GetModuleHandle(NULL), NULL);
	}
	else
	{
		mWindowHandle = CreateWindowEx(0, mWindowClassName, "", WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX, x, y, width, height, NULL, NULL, GetModuleHandle(NULL), NULL);
	}

	if (mWindowHandle == NULL)
	{
		Log::E("Unable to create a Window.");
		return false;
	}

	//Save the WGLRenderWindow pointer to the window's user data
	SetWindowLong(mWindowHandle, GWL_USERDATA, (LONG) this);
	RECT rc;
	rc.left = rc.top = 0;
	rc.right = width;
	rc.bottom = height;
	int style, styleEx;
	style = GetWindowLong(mWindowHandle, GWL_STYLE);
	styleEx = GetWindowLong(mWindowHandle, GWL_EXSTYLE);
	AdjustWindowRectEx(&rc, style, false, styleEx);
	SetWindowPos(mWindowHandle, 0, 0, 0, rc.right-rc.left, rc.bottom-rc.top, SWP_NOMOVE | SWP_NOZORDER);
	PIXELFORMATDESCRIPTOR pfd;
	int iFormat;
	/* get the device context (DC) */
	mDC = GetDC(mWindowHandle);
	/* set the pixel format for the DC */
	ZeroMemory(&pfd, sizeof (pfd));
	pfd.nSize = sizeof (pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = depth;
	pfd.iLayerType = PFD_MAIN_PLANE;
	iFormat = ChoosePixelFormat(mDC, &pfd);

	if (iFormat == 0)
	{
		Log::E("Pixel format not supported.");
		return false;
	}

	if (!SetPixelFormat(mDC, iFormat, &pfd))
	{
		Log::E("Unable to set the pixel format, altough it seems to be supported.");
		return false;
	}

	mRC = wglCreateContext(mDC);


	if (mRC == NULL)
	{
		Log::E("Unable to create a rendering context.");
		return false;
	}

	wglMakeCurrent(mDC, mRC);

	mFull = fullscreen;
	mX = x;
	mY = y;
	mWidth = width;
	mHeight = height;
	mCreated = true;
	SetVisible(true);
	//GetDevice()->GetRenderer()->ResizeRenderTarget(width, height);
	return true;
}

void WGLRenderWindow::Destroy()
{
	if (!mCreated)
	{
		return;
	}

	mCreated = false;
}

void WGLRenderWindow::SetVisible(bool visible)
{
	if (visible)
	{
		ShowWindow(mWindowHandle, SW_SHOW);
	}
	else
	{
		ShowWindow(mWindowHandle, SW_HIDE);
	}
}

void WGLRenderWindow::Move(uint x, uint y)
{
	SetWindowPos(mWindowHandle, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void WGLRenderWindow::Resize(uint width, uint height)
{
	SetWindowPos(mWindowHandle, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
}

void WGLRenderWindow::SetFullscreen(bool full)
{
}

void WGLRenderWindow::_SetTitleAndAdditionalTextToWindow()
{
	SetWindowText(mWindowHandle, GetDisplayedTitle().c_str());
}

void WGLRenderWindow::EventLoop()
{
	//The ultimate lamer trick: do the event loop filtering out the mouse and keyboard events by doing sequential calls with intervals
	EventLoopDo(0, 0);
	//EventLoopDo(1, WM_KEYFIRST-1);
	//EventLoopDo(WM_KEYFIRST, WM_KEYLAST);
	//EventLoopDo(WM_KEYLAST+1, WM_MOUSEFIRST-1);
	//EventLoopDo(WM_MOUSEFIRST, WM_MOUSELAST);
	//EventLoopDo(WM_MOUSELAST+1, WM_USER);
}

void WGLRenderWindow::EventLoopDo(unsigned int minMsg, unsigned int maxMsg)
{
	MSG msg;
	while (PeekMessage(&msg, mWindowHandle, minMsg, maxMsg, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void WGLRenderWindow::Bind()
{
	wglMakeCurrent(mDC, mRC);
}

void WGLRenderWindow::Unbind()
{
	wglMakeCurrent(NULL, NULL);
}

void WGLRenderWindow::Update()
{
	SwapBuffers(mDC);
}

} // namespace crown

