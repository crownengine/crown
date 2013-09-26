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

#include "GLContext.h"
#include "Assert.h"
#include "Types.h"
#include "Log.h"

namespace crown
{

static HWND g_handle_window;

//-----------------------------------------------------------------------------
void set_win_handle_window(HWND win)
{
	g_handle_window = win;

	CE_ASSERT(g_handle_window != 0, "Unable to set Windows handle");
}

//-----------------------------------------------------------------------------
GLContext::GLContext() :
	m_win_context(NULL),
	m_hdc(0)
{
	
}

//-----------------------------------------------------------------------------
void GLContext::create_context()
{
	/* get the device context (DC) */
	m_hdc = GetDC(g_handle_window);
	CE_ASSERT(m_hdc != NULL, "Fail to retrieves window device context");

	Log::i("no windows.h");

	/* set the pixel format for the DC */
	PIXELFORMATDESCRIPTOR pfd;
	int32_t pixel_format;
	ZeroMemory(&pfd, sizeof (pfd));
	pfd.nSize = sizeof (pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 24;
	pfd.iLayerType = PFD_MAIN_PLANE;

	pixel_format = ChoosePixelFormat(m_hdc, &pfd);
	CE_ASSERT(pixel_format != 0, "Pixel format not supported");

	bool pf_set = SetPixelFormat(m_hdc, pixel_format, &pfd);
	CE_ASSERT(pf_set, "Unable to set the pixel format, altough it seems to be supported");

	m_win_context = wglCreateContext(m_hdc);

	CE_ASSERT(m_win_context != NULL, "Unable to create a render context. errno: %d", GetLastError());

	BOOL result = wglMakeCurrent(m_hdc, m_win_context);

	CE_ASSERT(result == TRUE , "Unable to make current WGL context.");	
}

//-----------------------------------------------------------------------------
void GLContext::destroy_context()
{
	wglMakeCurrent(NULL, NULL);
}

//-----------------------------------------------------------------------------
void GLContext::swap_buffers()
{
	if (g_handle_window != NULL)
	{
		wglMakeCurrent(m_hdc, m_win_context);
		SwapBuffers(m_hdc);
	}
}

} // namespace crown