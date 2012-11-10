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

#include <windows.h>
#include "RenderWindow.h"
#include "Types.h"

namespace crown
{

class WGLRenderWindow : public RenderWindow
{

public:

	//! Constructor
	WGLRenderWindow();

	//! Destructor
	~WGLRenderWindow();

	//! Creates the window
	virtual bool Create(uint x, uint y, uint width, uint height,
						uint depth, bool fullscreen);

	//! Destroys the window
	virtual void Destroy();

	//! Sets whether the window is visible
	virtual void SetVisible(bool visible);

	//! Sets the window's position
	virtual void Move(uint x, uint y);

	//! Sets the window's width and height
	virtual void Resize(uint width, uint height);

	//! Sets whether the window is in fullscreen mode
	virtual void SetFullscreen(bool full);

	virtual void Bind();

	virtual void Unbind();

	//! Updates the window's content
	virtual void Update();

	//! Manages basic window's events
	virtual void EventLoop();

	//! Returns the associated X Window
	HWND GetWindowHandle() const
	{
		return mWindowHandle;
	}

	HDC GetDrawingContext() const
	{
		return mDC;
	}

private:

	HWND mWindowHandle;
	HGLRC mRC;
	HDC mDC;

	bool mCreated;

	//! Sets the window's title plus the additional title text in the window
	virtual void _SetTitleAndAdditionalTextToWindow();

	static char mWindowClassName[20];

	void EventLoopDo(unsigned int minMsg, unsigned int maxMsg);
};

} // namespace crown

