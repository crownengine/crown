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

#include "RenderWindow.h"
#include "Types.h"
#include "Config.h"

#if defined(LINUX) && defined(CROWN_USE_OPENGL)
	#include "GLXRenderWindow.h"
#elif defined(WINDOWS) && defined(CROWN_USE_OPENGL)
	#include "WGLRenderWindow.h"
#elif defined(LINUX) && defined(CROWN_USE_OPENGLES)
	#include "EGLRenderWindow.h"
#elif defined(CROWN_OS_ANDROID)
	#include "AndroidRenderWindow.h"
#endif

namespace Crown
{

RenderWindow* RenderWindow::CreateWindow(uint x, uint y, uint width, uint height, uint depth, bool fullscreen)
{
	RenderWindow* window = NULL;

	#if defined(LINUX) && defined(CROWN_USE_OPENGL)
		window = new GLXRenderWindow;
	#elif defined(WINDOWS) && defined(CROWN_USE_OPENGL)
		window = new WGLRenderWindow;
	#elif defined(LINUX) && defined(CROWN_USE_OPENGLES)
		window = new EGLRenderWindow;
	#elif defined(CROWN_OS_ANDROID)
		window = new AndroidRenderWindow;
	#endif

	if (window)
	{
		window->Create(x, y, width, height, depth, fullscreen);
	}

	return window;
}

void RenderWindow::DestroyWindow(RenderWindow* window)
{
	if (!window)
	{
		throw NullPointerException("RenderWindow::DestroyWindow: window == NULL");
	}

	window->Destroy();

	delete window;
}

RenderWindow::RenderWindow() :
	mX(0), mY(0),
	mWidth(0), mHeight(0),
	mVisible(false),
	mActive(false),
	mFull(false),
	mCreated(false),
	mMain(false),
	mTitle("Crown"),
	mAdditionalTitleText("")
{
}

RenderWindow::~RenderWindow()
{
}

bool RenderWindow::IsMain() const
{
	return mMain;
}

void RenderWindow::_SetMain(bool main)
{
	mMain = main;
}

bool RenderWindow::IsVisible() const
{
	return mVisible;
}

void RenderWindow::GetPosition(uint& x, uint& y)
{
	x = mX;
	y = mY;
}

void RenderWindow::GetMetrics(uint& width, uint& height) const
{
	width = mWidth;
	height = mHeight;
}

bool RenderWindow::IsFullscreen() const
{
	return mFull;
}

const Str& RenderWindow::GetTitle() const
{
	return mTitle;
}

void RenderWindow::SetTitle(const Str& title)
{
	mTitle = title;
	_SetTitleAndAdditionalTextToWindow();
}

const Str& RenderWindow::GetAdditionalTitleText() const
{
	return mAdditionalTitleText;
}

void RenderWindow::SetAdditionalTitleText(const Str& text)
{
	mAdditionalTitleText = text;
	_SetTitleAndAdditionalTextToWindow();
}

Str RenderWindow::GetDisplayedTitle() const
{
	return mTitle + Str(" - ") + mAdditionalTitleText;
}

} // namespace Crown

