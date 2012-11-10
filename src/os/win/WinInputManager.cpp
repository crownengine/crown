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

#include "WinInputManager.h"
#include "WinMouse.h"
#include "WinKeyboard.h"

namespace crown
{

//-----------------------------------------------------------------------------
WinInputManager::WinInputManager() :
	mIsMouseAvailable(false),
	mIsKeyboardAvailable(false),
	mWindowHandle(0)
{
}

//-----------------------------------------------------------------------------
WinInputManager::~WinInputManager()
{
	if (mMouse)
	{
		delete mMouse;
	}

	if (mKeyboard)
	{
		delete mKeyboard;
	}
}

//-----------------------------------------------------------------------------
void WinInputManager::Init(const EventSource& source)
{
	mWindowHandle = (HWND)source.WindowHandle;

	if (!mMouse)
	{
		mMouse = new WinMouse(this);
	}

	if (!mKeyboard)
	{
		mKeyboard = new WinKeyboard(this);
	}
}

} // namespace crown

