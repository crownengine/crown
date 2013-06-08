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

namespace crown
{

static HWND s_handle_window = 0;

//-----------------------------------------------------------------------------
void set_win_handle_window(HWND win)
{
	s_handle_window = win;
}

//-----------------------------------------------------------------------------
GLContext::GLContext() :
	m_win_context(NULL)
{
	
}

//-----------------------------------------------------------------------------
void GLContext::create_context()
{
	m_win_context = wglCreateContext(GetDC(s_handle_window));

	wglMakeCurrent(GetDC(s_handle_window), m_win_context);

	CE_ASSERT(m_win_context != NULL, "Unable to create a rendering context.");
}

//-----------------------------------------------------------------------------
void GLContext::destroy_context()
{
	wglMakeCurrent(NULL, NULL);
}

//-----------------------------------------------------------------------------
void GLContext::swap_buffers()
{
	SwapBuffers(GetDC(s_handle_window));
}

} // namespace crown