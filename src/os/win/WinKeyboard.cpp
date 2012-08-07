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

#include "WinKeyboard.h"
#include "Exceptions.h"
#include "WinInputManager.h"

// Undef functions to avoid conflicts
#undef MK_SHIFT
#undef MK_ALT

namespace Crown
{

//-----------------------------------------------------------------------------
WinKeyboard::WinKeyboard(InputManager* creator) :
	Keyboard(creator),
	mModifierMask(0),
	mWindowHandle(0)
{
	for (int i = 0; i < MAX_KEYCODES; i++)
	{
		mKeyState[i] = false;
	}

	if (mCreator)
	{
		mWindowHandle = (HWND)(static_cast<WinInputManager*>(mCreator)->GetWindowHandle());
	}

	static_cast<WinInputManager*>(mCreator)->SetKeyboardAvailable(true);
}

//-----------------------------------------------------------------------------
WinKeyboard::~WinKeyboard()
{
	if (mCreator)
	{
		static_cast<WinInputManager*>(mCreator)->SetKeyboardAvailable(false);
	}
}

//-----------------------------------------------------------------------------
bool WinKeyboard::IsModifierPressed(ModifierKey modifier) const
{
	return (mModifierMask & modifier) == modifier;
}

//-----------------------------------------------------------------------------
bool WinKeyboard::IsKeyPressed(KeyCode key) const
{
	return mKeyState[key] == true;
}

//-----------------------------------------------------------------------------
bool WinKeyboard::IsKeyReleased(KeyCode key) const
{
	return mKeyState[key] == false;
}

//-----------------------------------------------------------------------------
void WinKeyboard::EventLoop()
{
	MSG msg;
	while (PeekMessage(&msg, mWindowHandle, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		switch (msg.message)
		{
			case WM_KEYDOWN:
			case WM_KEYUP:
			{
				KeyboardEvent keyboardEvent;
				Key kc = TranslateKey(msg.wParam);

				// Check if any modifier key is pressed or released
				if (kc == KC_LSHIFT || kc == KC_RSHIFT)
				{
					(msg.message == WM_KEYDOWN) ? mModifierMask |= MK_SHIFT : mModifierMask &= ~MK_SHIFT;
				}
				else if (kc == KC_LCONTROL || kc == KC_RCONTROL)
				{
					(msg.message == WM_KEYDOWN) ? mModifierMask |= MK_CTRL : mModifierMask &= ~MK_CTRL;
				}
				else if (kc == KC_LALT || kc == KC_RALT)
				{
					(msg.message == WM_KEYDOWN) ? mModifierMask |= MK_ALT : mModifierMask &= ~MK_ALT;
				}

				mKeyState[kc] = (msg.message == WM_KEYDOWN) ? true : false;
				keyboardEvent.key = kc;

				if (mListener)
				{
					if (msg.message == WM_KEYDOWN)
					{
						mListener->KeyPressed(keyboardEvent);
					}
					else if (msg.message == WM_KEYUP)
					{
						mListener->KeyReleased(keyboardEvent);
					}
				}
			}break;
			case WM_CHAR:
			{
				KeyboardEvent keyboardEvent;
				keyboardEvent.text[0] = msg.wParam;
				keyboardEvent.text[1] = '\0';

				if (mListener)
				{
					mListener->TextInput(keyboardEvent);
				}
			}break;
		}
	}
}

//-----------------------------------------------------------------------------
Key WinKeyboard::TranslateKey(int winKey)
{
	if ((winKey > 0x40 && winKey < 0x5B) || (winKey > 0x60 && winKey < 0x7B) || (winKey > 0x2F && winKey < 0x3A))
	{
		return (Key)winKey;
	}

	switch (winKey)
	{
		case VK_BACK:		return KC_BACKSPACE;
		case VK_TAB:		return KC_TAB;
		case VK_SPACE:		return KC_SPACE;
		case VK_ESCAPE:		return KC_ESCAPE;
		case VK_RETURN:		return KC_ENTER;
		case VK_F1:			return KC_F1;
		case VK_F2:			return KC_F2;
		case VK_F3:			return KC_F3;
		case VK_F4:			return KC_F4;
		case VK_F5:			return KC_F5;
		case VK_F6:			return KC_F6;
		case VK_F7:			return KC_F7;
		case VK_F8:			return KC_F8;
		case VK_F9:			return KC_F9;
		case VK_F10:		return KC_F10;
		case VK_F11:		return KC_F11;
		case VK_F12:		return KC_F12;
		case VK_HOME:		return KC_HOME;
		case VK_LEFT:		return KC_LEFT;
		case VK_UP:			return KC_UP;
		case VK_RIGHT:		return KC_RIGHT;
		case VK_DOWN:		return KC_DOWN;
		case VK_PRIOR:		return KC_PAGE_UP;
		case VK_NEXT:		return KC_PAGE_DOWN;
		case VK_LSHIFT:		return KC_LSHIFT;
		case VK_RSHIFT:		return KC_RSHIFT;
		case VK_LCONTROL:	return KC_LCONTROL;
		case VK_RCONTROL:	return KC_RCONTROL;
		case VK_CAPITAL:	return KC_CAPS_LOCK;
		case VK_LMENU:		return KC_LALT;
		case VK_RMENU:		return KC_RALT;
		case VK_LWIN:		return KC_LSUPER;
		case VK_RWIN:		return KC_RSUPER;
		case VK_NUMPAD0:	return KC_KP_0;
		case VK_NUMPAD1:	return KC_KP_1;
		case VK_NUMPAD2:	return KC_KP_2;
		case VK_NUMPAD3:	return KC_KP_3;
		case VK_NUMPAD4:	return KC_KP_4;
		case VK_NUMPAD5:	return KC_KP_5;
		case VK_NUMPAD6:	return KC_KP_6;
		case VK_NUMPAD7:	return KC_KP_7;
		case VK_NUMPAD8:	return KC_KP_8;
		case VK_NUMPAD9:	return KC_KP_9;
		default:			return KC_NOKEY;
	}
}

} // namespace Crown

