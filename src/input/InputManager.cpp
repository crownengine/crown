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

#include "InputManager.h"
#include "OS.h"

namespace crown
{

//-----------------------------------------------------------------------------
InputManager::InputManager()
{
//	mMouse = new Mouse();
//	mKeyboard = new Keyboard();
//	mTouch = new Touch();
}

//-----------------------------------------------------------------------------
InputManager::~InputManager()
{
}

//-----------------------------------------------------------------------------
void InputManager::EventLoop()
{
	os::OSEvent event;

	while (1)
	{
		event = os::pop_event();

		if (event.type == os::OSET_NONE)
		{
			return;
		}

		os::printf("OS Event: %d, %d, %d, %d, %d\n", event.type, event.data_a, event.data_b, event.data_c, event.data_d);

		switch (event.type)
		{
			case os::OSET_BUTTON_PRESS:
			case os::OSET_BUTTON_RELEASE:
			{
				MouseEvent mouse_event;
				mouse_event.x = event.data_a;
				mouse_event.y = event.data_b;
				mouse_event.button = event.data_c == 0 ? MB_LEFT : event.data_c == 1 ? MB_MIDDLE : MB_RIGHT;
				mouse_event.wheel = 0.0f;

				if (event.type == os::OSET_BUTTON_PRESS)
				{
					mEventDispatcher.ButtonPressed(mouse_event);
				}
				else
				{
					mEventDispatcher.ButtonReleased(mouse_event);
				}

				break;
			}
			case os::OSET_KEY_PRESS:
			case os::OSET_KEY_RELEASE:
			{
				KeyboardEvent keyboard_event;
				keyboard_event.key = event.data_a;

				if (event.type == os::OSET_KEY_PRESS)
				{
					mEventDispatcher.KeyPressed(keyboard_event);
				}
				else
				{
					mEventDispatcher.KeyReleased(keyboard_event);
				}

				break;
			}
			default:
			{
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
InputManager inputManager;
InputManager* GetInputManager()
{
	return &inputManager;
}

} // namespace crown

