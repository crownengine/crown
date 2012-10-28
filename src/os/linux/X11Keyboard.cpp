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

#include "X11Keyboard.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <X11/XKBlib.h>
#include "Exceptions.h"
#include "X11InputManager.h"
#include "Log.h"
#include "OS.h"

namespace Crown
{

//-----------------------------------------------------------------------------
X11Keyboard::X11Keyboard(InputManager* creator) :
	Keyboard(creator),
	mModifierMask(0),
	mDisplay(NULL),
	mXWindow(0),
	mDetectableAutoRepeat(false)
{
	for (int i = 0; i < MAX_KEYCODES; i++)
	{
		mKeyState[i] = false;
	}

	// Connect to the X Server
	mDisplay = XOpenDisplay(NULL);

	if (!mDisplay)
	{
		throw InvalidOperationException("X11Keyboard::X11Keyboard: Unable to connect to the X Server.");
	}

	if (mCreator)
	{
		mXWindow = static_cast<X11InputManager*>(mCreator)->GetXWindow();
	}

	// We want to track motion and button pressed/released events
	if (XSelectInput(mDisplay, mXWindow, KeyPressMask | KeyReleaseMask) == BadWindow)
	{
		throw InvalidOperationException("X11Keyboard::X11Keyboard: Unable to select input, bad window.");
	}

	Bool detectable;
	if ((mDetectableAutoRepeat = (bool) XkbSetDetectableAutoRepeat(mDisplay, true, &detectable)) == False)
	{
		Log::I("X11Keyboard::X11Keyboard: Detectable auto-repeat not supported.");
	}

	static_cast<X11InputManager*>(mCreator)->SetKeyboardAvailable(true);
}

//-----------------------------------------------------------------------------
X11Keyboard::~X11Keyboard()
{
	if (mCreator)
	{
		static_cast<X11InputManager*>(mCreator)->SetKeyboardAvailable(false);
	}

	if (mDisplay)
	{
		XCloseDisplay(mDisplay);
	}
}

//-----------------------------------------------------------------------------
bool X11Keyboard::IsModifierPressed(ModifierKey modifier) const
{
	return (mModifierMask & modifier) == modifier;
}

//-----------------------------------------------------------------------------
bool X11Keyboard::IsKeyPressed(KeyCode key) const
{
	return mKeyState[key] == true;
}

//-----------------------------------------------------------------------------
bool X11Keyboard::IsKeyReleased(KeyCode key) const
{
	return mKeyState[key] == false;
}

//-----------------------------------------------------------------------------
void X11Keyboard::EventLoop()
{
	XEvent event;
	KeyboardEvent keyboardEvent;

	while (XPending(mDisplay))
	{
		XNextEvent(mDisplay, &event);

		switch (event.type)
		{
			case KeyPress:
			case KeyRelease:
			{
				char string[4] = {0, 0, 0, 0};
				int len = -1;
				KeySym key;

				len = XLookupString(&event.xkey, string, 4, &key, NULL);

				Key kc = TranslateKey(key);

				// Check if any modifier key is pressed or released
				if (kc == KC_LSHIFT || kc == KC_RSHIFT)
				{
					(event.type == KeyPress) ? mModifierMask |= MK_SHIFT : mModifierMask &= ~MK_SHIFT;
				}
				else if (kc == KC_LCONTROL || kc == KC_RCONTROL)
				{
					(event.type == KeyPress) ? mModifierMask |= MK_CTRL : mModifierMask &= ~MK_CTRL;
				}
				else if (kc == KC_LALT || kc == KC_RALT)
				{
					(event.type == KeyPress) ? mModifierMask |= MK_ALT : mModifierMask &= ~MK_ALT;
				}

				mKeyState[kc] = (event.type == KeyPress) ? true : false;
				keyboardEvent.key = kc;

				if (mListener)
				{
					if (event.type == KeyPress)
					{
						mListener->KeyPressed(keyboardEvent);
					}
					else if (event.type == KeyRelease)
					{
						mListener->KeyReleased(keyboardEvent);
					}
				}

				if (event.type == KeyPress)
				{
				push_event(OS::OSET_KEYBOARD, 1, 2, 3, 4);
				}
				else if (event.type == KeyRelease)
				{
				push_event(OS::OSET_KEYBOARD, 55, 2, 3, 4);
				}


				// Text input part
				if (event.type == KeyPress && len > 0)
				{
					//crownEvent.event_type = ET_TEXT;
					//crownEvent.text.type = TET_TEXT_INPUT;
					strncpy(keyboardEvent.text, string, 4);

					if (mListener)
					{
						mListener->TextInput(keyboardEvent);
					}
				}

				break;
			}
			case KeymapNotify:
			{
				XRefreshKeyboardMapping(&event.xmapping);

				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
Key X11Keyboard::TranslateKey(int x11Key)
{
	if ((x11Key > 0x40 && x11Key < 0x5B) || (x11Key > 0x60 && x11Key < 0x7B) || (x11Key > 0x2F && x11Key < 0x3A))
	{
		return (Key)x11Key;
	}

	switch (x11Key)
	{
		case XK_BackSpace:	return KC_BACKSPACE;
		case XK_Tab:		return KC_TAB;
		case XK_space:		return KC_SPACE;
		case XK_Escape:		return KC_ESCAPE;
		case XK_Return:		return KC_ENTER;
		case XK_F1:			return KC_F1;
		case XK_F2:			return KC_F2;
		case XK_F3:			return KC_F3;
		case XK_F4:			return KC_F4;
		case XK_F5:			return KC_F5;
		case XK_F6:			return KC_F6;
		case XK_F7:			return KC_F7;
		case XK_F8:			return KC_F8;
		case XK_F9:			return KC_F9;
		case XK_F10:		return KC_F10;
		case XK_F11:		return KC_F11;
		case XK_F12:		return KC_F12;
		case XK_Home:		return KC_HOME;
		case XK_Left:		return KC_LEFT;
		case XK_Up:			return KC_UP;
		case XK_Right:		return KC_RIGHT;
		case XK_Down:		return KC_DOWN;
		case XK_Page_Up:	return KC_PAGE_UP;
		case XK_Page_Down:	return KC_PAGE_DOWN;
		case XK_Shift_L:	return KC_LSHIFT;
		case XK_Shift_R:	return KC_RSHIFT;
		case XK_Control_L:	return KC_LCONTROL;
		case XK_Control_R:	return KC_RCONTROL;
		case XK_Caps_Lock:	return KC_CAPS_LOCK;
		case XK_Alt_L:		return KC_LALT;
		case XK_Alt_R:		return KC_RALT;
		case XK_Super_L:	return KC_LSUPER;
		case XK_Super_R:	return KC_RSUPER;
		case XK_KP_0:		return KC_KP_0;
		case XK_KP_1:		return KC_KP_1;
		case XK_KP_2:		return KC_KP_2;
		case XK_KP_3:		return KC_KP_3;
		case XK_KP_4:		return KC_KP_4;
		case XK_KP_5:		return KC_KP_5;
		case XK_KP_6:		return KC_KP_6;
		case XK_KP_7:		return KC_KP_7;
		case XK_KP_8:		return KC_KP_8;
		case XK_KP_9:		return KC_KP_9;
		default:			return KC_NOKEY;
	}
}

} // namespace Crown

