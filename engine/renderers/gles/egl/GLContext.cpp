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

#include <GLES2/gl2.h>

#include "Assert.h"
#include "GLContext.h"
#include "Log.h"

namespace crown
{

static ANativeWindow* awindow = NULL;

//-----------------------------------------------------------------------------
void set_android_window(ANativeWindow* window)
{
    CE_ASSERT_NOT_NULL(window);

	awindow = window;
}

//-----------------------------------------------------------------------------
GLContext::GLContext() :
	num_configs(0)
{
}

//-----------------------------------------------------------------------------
void GLContext::create_context()
{
	EGLint format;

	// Screen format rgbx8888 with no alpha channel,
	// maybe it is wrong but is for testing
	EGLint attrib_list[]= { EGL_RED_SIZE,        8,
                            EGL_GREEN_SIZE,      8,
                            EGL_BLUE_SIZE,       8,
                            EGL_DEPTH_SIZE,      24,
                            EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
                            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                            EGL_NONE};

    EGLint attributes[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };

    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, NULL, NULL);

    eglBindAPI(EGL_OPENGL_ES_API);

    eglChooseConfig(display, attrib_list, &config, 1, &num_configs);

	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

	ANativeWindow_setBuffersGeometry(awindow, 0, 0, format);

    context = eglCreateContext(display, config, EGL_NO_CONTEXT, attributes);

	surface = eglCreateWindowSurface(display, config, (EGLNativeWindowType)awindow, NULL);

    eglMakeCurrent(display, surface, surface, context);

    Log::i("EGL context created");
}

//-----------------------------------------------------------------------------
void GLContext::destroy_context()
{
 	eglDestroyContext(display, context);
 	eglDestroySurface(display, surface);
 	eglTerminate(display);

    Log::i("EGL context destroyed");
}

//-----------------------------------------------------------------------------
void GLContext::swap_buffers()
{
   	eglSwapBuffers(display, surface);
}

}