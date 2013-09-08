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

#include "Config.h"
#include "Assert.h"
#include "GLContext.h"
#include "Log.h"

namespace crown
{

extern ANativeWindow* g_android_window;

//-----------------------------------------------------------------------------
static const char* egl_error_to_string(EGLint error)
{
	switch (error)
	{
		case EGL_NOT_INITIALIZED: return "EGL_NOT_INITIALIZED";
		case EGL_BAD_ACCESS: return "EGL_BAD_ACCESS";
		case EGL_BAD_ALLOC: return "EGL_BAD_ALLOC";
		case EGL_BAD_ATTRIBUTE: return "EGL_BAD_ATTRIBUTE";
		case EGL_BAD_CONTEXT: return "EGL_BAD_CONTEXT";
		case EGL_BAD_CONFIG: return "EGL_BAD_CONFIG";
		case EGL_BAD_CURRENT_SURFACE: return "EGL_BAD_CURRENT_SURFACE";
		case EGL_BAD_DISPLAY: return "EGL_BAD_DISPLAY";
		case EGL_BAD_SURFACE: return "EGL_BAD_SURFACE";
		case EGL_BAD_MATCH: return "EGL_BAD_MATCH";
		case EGL_BAD_PARAMETER: return "EGL_BAD_PARAMETER";
		case EGL_BAD_NATIVE_PIXMAP: return "EGL_BAD_NATIVE_PIXMAP";
		case EGL_BAD_NATIVE_WINDOW: return "EGL_BAD_NATIVE_WINDOW";
		case EGL_CONTEXT_LOST: return "EGL_CONTEXT_LOST";
		default: return "UNKNOWN_EGL_ERROR";
	}
}

//-----------------------------------------------------------------------------
#if defined(CROWN_DEBUG) || defined(CROWN_DEVELOPMENT)
	#define EGL_CHECK(function)\
		function;\
		do { EGLint error; CE_ASSERT((error = eglGetError()) == EGL_SUCCESS,\
				"EGL error: %s", egl_error_to_string(error)); } while (0)
#else
	#define EGL_CHECK(function)\
		function;
#endif

//-----------------------------------------------------------------------------
GLContext::GLContext()
	: display(EGL_NO_DISPLAY), surface(EGL_NO_SURFACE), context(EGL_NO_CONTEXT), num_configs(0)
{
}

//-----------------------------------------------------------------------------
void GLContext::create_context()
{
    const EGLint attrib_list[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_NONE
    };

	EGLint attributes[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };

	display = EGL_CHECK(eglGetDisplay(EGL_DEFAULT_DISPLAY));
	CE_ASSERT(display != EGL_NO_DISPLAY, "Failed to obtain EGL display connection");

	EGLint egl_major, egl_minor;
	EGLBoolean init_success = EGL_CHECK(eglInitialize(display, &egl_major, &egl_minor));
	CE_ASSERT(init_success == EGL_TRUE, "Failed to initialize EGL");

	Log::d("EGL Initialized: major = %d, minor = %d", egl_major, egl_minor);

	EGLBoolean bind_success = EGL_CHECK(eglBindAPI(EGL_OPENGL_ES_API));
	CE_ASSERT(bind_success != EGL_FALSE, "Failed to bind OpenGL|ES API");

	EGLBoolean cfg_success = EGL_CHECK(eglChooseConfig(display, attrib_list, &config, 1, &num_configs));
	CE_ASSERT(cfg_success == EGL_TRUE, "Failed to choose EGL configuration");

	EGLint format;
	EGL_CHECK(eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format));

	ANativeWindow_setBuffersGeometry(g_android_window, 0, 0, format);

	context = EGL_CHECK(eglCreateContext(display, config, EGL_NO_CONTEXT, attributes));
	CE_ASSERT(context != EGL_NO_CONTEXT, "Oops, failed to create EGL context");

	surface = EGL_CHECK(eglCreateWindowSurface(display, config, (EGLNativeWindowType)g_android_window, NULL));
	CE_ASSERT(surface != EGL_NO_SURFACE, "Oops, failed to create EGL surface");

	EGL_CHECK(eglMakeCurrent(display, surface, surface, context));

	Log::d("EGL context created");
}

//-----------------------------------------------------------------------------
void GLContext::destroy_context()
{
	EGL_CHECK(eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT));
	EGL_CHECK(eglDestroyContext(display, context));
	EGL_CHECK(eglDestroySurface(display, surface));
	EGL_CHECK(eglTerminate(display));

	Log::i("EGL context destroyed");
}

//-----------------------------------------------------------------------------
void GLContext::swap_buffers()
{
	EGL_CHECK(eglMakeCurrent(display, surface, surface, context));
	EGL_CHECK(eglSwapBuffers(display, surface));
}

}
