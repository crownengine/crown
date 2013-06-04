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

#pragma once

#include "Mouse.h"
#include <windows.h>

namespace crown
{

/**
	Implementation of Mouse int32_terface for Windows.
*/
class WinMouse : public Mouse
{

public:

	/** @copydoc Mouse::Mouse() */
	WinMouse(InputManager* creator);

	/** @copydoc Mouse::~Mouse() */
	~WinMouse();

	/** @copydoc Mouse::IsVisible() */
	virtual bool IsCursorVisible() const;

	/** @copydoc Mouse::SetVisible() */
	virtual void SetCursorVisible(bool visible);

	/** @copydoc Mouse::GetCursorXY() */
	virtual Point32_t2 GetCursorXY() const;

	/** @copydoc Mouse::SetCursorXY() */
	virtual void SetCursorXY(const Point32_t2& position);

	/** @copydoc Mouse::GetCursorRelativeXY() */
	virtual Vec2 GetCursorRelativeXY() const;

	/** @copydoc Mouse::SetCursorRelativeXY() */
	virtual void SetCursorRelativeXY(const Vec2& position);

	/** @copydoc Mouse::EventLoop() */
	virtual void EventLoop();

private:

	bool mIsCursorVisible;

	// Win related
	HWND mWindowHandle;
};

} // namespace crown

