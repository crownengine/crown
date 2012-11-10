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
#include "AndroidRenderWindow.h"
#include "Log.h"
#include "Types.h"
#include "Config.h"
#include <android_native_app_glue.h>

namespace crown
{

AndroidRenderWindow::AndroidRenderWindow() :
	mEGLDisplay(EGL_NO_DISPLAY),
	mEGLContext(EGL_NO_CONTEXT),
	mEGLWindow(EGL_NO_SURFACE)
{
	mANativeWindow = GetDevice()->_GetAndroidApp()->window;
	mEGLDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	eglInitialize(mEGLDisplay, NULL, NULL);
}

AndroidRenderWindow::~AndroidRenderWindow()
{
	if (mEGLDisplay != EGL_NO_DISPLAY)
	{
		Log::D("AndroidRenderWindow::Destroy: Releasing context...");
		if (mEGLContext != EGL_NO_CONTEXT)
		{
			eglMakeCurrent(mEGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
			eglDestroyContext(mEGLDisplay, mEGLContext);
		}
		Log::D("AndroidRenderWindow::Destroy: Context released.");

		if (mEGLWindow != EGL_NO_SURFACE)
		{
			eglDestroySurface(mEGLDisplay, mEGLWindow);
		}

		mEGLWindow = EGL_NO_SURFACE;
		Log::D("AndroidRenderWindow::Destroy: Window Destroyed.");

		eglTerminate(mEGLDisplay);
	}
}

bool AndroidRenderWindow::Create(uint x, uint y, uint width, uint height, uint depth, bool /*fullscreen*/)
{
	Log::D("AndroidRenderWindow::Create: Creating window...");
	if (!width || !height)
	{
		Log::E("Width and height must differ from 0.");
		return false;
	}

	if (!mEGLDisplay)
	{
		Log::E("Unable to open a display");
		return false;
	}

	uint bpp			= depth / 4;

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
		Log::E("Unable to choose config.");
		return false;
	}

	EGLint format;
    eglGetConfigAttrib(mEGLDisplay, ecfg, EGL_NATIVE_VISUAL_ID, &format);

	// Reconfigure ANativeWindow buffer
    ANativeWindow_setBuffersGeometry(mANativeWindow, 0, 0, format);

	mEGLWindow = eglCreateWindowSurface(mEGLDisplay, ecfg, mANativeWindow, NULL);
	if (mEGLWindow == EGL_NO_SURFACE)
	{
		Log::E("Unable to create window surface.");
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
		Log::E("Unable to create context: " + Str(eglGetError()));
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

	Log::D("AndroidRenderWindow::Create: Window created.");

	return true;
}

void AndroidRenderWindow::Destroy()
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

void AndroidRenderWindow::SetVisible(bool visible)
{
	mVisible = visible;
}

void AndroidRenderWindow::Move(uint x, uint y)
{
}

void AndroidRenderWindow::Resize(uint width, uint height)
{
}

void AndroidRenderWindow::SetFullscreen(bool full)
{
}

void AndroidRenderWindow::Bind()
{
	eglMakeCurrent(mEGLDisplay, mEGLWindow, mEGLWindow, mEGLContext);
}

void AndroidRenderWindow::Unbind()
{
	eglMakeCurrent(mEGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

void AndroidRenderWindow::Update()
{
	eglSwapBuffers(mEGLDisplay, mEGLWindow);
}

void AndroidRenderWindow::EventLoop()
{
}

void AndroidRenderWindow::_NotifyMetricsChange(uint x, uint y, uint width, uint height)
{
}

void AndroidRenderWindow::_SetTitleAndAdditionalTextToWindow()
{
}

} // namespace crown

