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

#include "InputManager.h"

namespace Crown
{

class AndroidInputManager : public InputManager
{

	friend class AndroidTouch;

public:

	/** @copydoc InputManager::InputManager() */
	AndroidInputManager();

	/** @copydoc InputManager::~InputManager() */
	virtual ~AndroidInputManager();

	/** @copydoc InputManager::Init() */
	virtual void Init(const EventSource& source);

	/** @copydoc InputManager::IsMouseAvailable() */
	virtual bool IsMouseAvailable()
	{
		return false;
	}

	/** @copydoc InputManager::IsKeyboardAvailable() */
	virtual bool IsKeyboardAvailable()
	{
		return false;
	}

	/** @copydoc InputManager::IsTouchAvailable() */
	virtual bool IsTouchAvailable()
	{
		return mIsTouchAvailable;
	}

	/**
		Returns the android_app structure.
	@return
		The android_app structure
	*/
	inline struct android_app* GetAndroidApp()
	{
		return mAndroidApp;
	}

private:

	void SetTouchAvailable(bool available) { mIsTouchAvailable = available; }

	bool mIsTouchAvailable : 1;

	// Android related
	struct android_app* mAndroidApp;
};

} // namespace Crown

