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

static Display* s_x11_display = NULL;
static Window s_x11_window = None;

//-----------------------------------------------------------------------------
void set_x11_display_and_window(Display* dpy, Window win)
{
	s_x11_display = dpy;
	s_x11_window = win;
}

//-----------------------------------------------------------------------------
GLContext::GLContext() :
	m_glx_context(NULL)
{
}

//-----------------------------------------------------------------------------
void GLContext::create_context()
{
	// Color index buffer not supported - deprecated
	static int fb_attribs[] =
	{
		GLX_RENDER_TYPE,		static_cast<int>(GLX_RGBA_BIT),
		GLX_DRAWABLE_TYPE,		static_cast<int>(GLX_WINDOW_BIT),
		GLX_DOUBLEBUFFER,		static_cast<int>(True),
		GLX_RED_SIZE,			8,
		GLX_GREEN_SIZE,			8,
		GLX_BLUE_SIZE,			8,
		GLX_DEPTH_SIZE,			24,
		static_cast<int>(None)
	};

	int fb_count;
	GLXFBConfig* fb_config = glXChooseFBConfig(s_x11_display, DefaultScreen(s_x11_display), fb_attribs, &fb_count);
	ce_assert(fb_config != NULL, "Unable to find a matching frame buffer configuration");

	XVisualInfo* vi = glXGetVisualFromFBConfig(s_x11_display, fb_config[0]);
	ce_assert(vi != NULL, "Unable to find a matching visual for frame buffer configuration.");

	m_glx_context = glXCreateContext(s_x11_display, vi, 0, True);
	ce_assert(m_glx_context != NULL, "Unable to create GLX context");

	glXMakeCurrent(s_x11_display, s_x11_window, m_glx_context);

	XFlush(s_x11_display);

	XFree(vi);
	XFree(fb_config);
}

//-----------------------------------------------------------------------------
void GLContext::destroy_context()
{
	if (s_x11_display != NULL)
	{
		if (m_glx_context != None)
		{
			glXMakeCurrent(s_x11_display, None, None);
			glXDestroyContext(s_x11_display, m_glx_context);
		}
	}
}

//-----------------------------------------------------------------------------
void GLContext::swap_buffers()
{
	glXSwapBuffers(s_x11_display, s_x11_window);
}

} // namespace crown