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

#pragma once

#include "RenderWindow.h"
#include "Types.h"
#include <EGL/egl.h>

namespace crown
{

class AndroidRenderWindow : public RenderWindow
{

public:

	//! See RenderWindow
	AndroidRenderWindow();

	//! See RenderWindow
	~AndroidRenderWindow();

	//! See RenderWindow
	virtual bool Create(uint x, uint y, uint width, uint height, uint depth, bool fullscreen);

	//! See RenderWindow
	virtual void Destroy();

	//! See RenderWindow
	virtual void SetVisible(bool visible);

	//! See RenderWindow
	virtual void Move(uint x, uint y);

	//! See RenderWindow
	virtual void Resize(uint width, uint height);

	//! See RenderWindow
	virtual void SetFullscreen(bool full);

	virtual void Bind();

	virtual void Unbind();

	//! See RenderWindow
	virtual void Update();

	virtual void EventLoop();

	inline EGLSurface GetEGLWindow() const
	{
		return mEGLWindow;
	}

private:

	//! See RenderWindow
	void _NotifyMetricsChange(uint x, uint y, uint width, uint height);

	//! Sets the window's title plus the additional title text in the window
	virtual void _SetTitleAndAdditionalTextToWindow();

    ANativeWindow* mANativeWindow;
	EGLDisplay mEGLDisplay;
	EGLContext mEGLContext;
	EGLSurface mEGLWindow;
};

} // namespace crown

