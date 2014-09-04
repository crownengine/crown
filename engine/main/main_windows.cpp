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

#include "config.h"

#if CROWN_PLATFORM_WINDOWS

#include <windowsx.h>

#define WM_USER_SET_WINDOW_SIZE     (WM_USER+0)
#define WM_USER_TOGGLE_WINDOW_FRAME (WM_USER+1)
#define WM_USER_MOUSE_LOCK          (WM_USER+2)

#include "device.h"
#include "os_event_queue.h"
#include "os_window_windows.h"
#include "thread.h"
#include <bgfxplatform.h>

#define ENTRY_DEFAULT_WIDTH 1000
#define ENTRY_DEFAULT_HEIGHT 625

namespace crown
{

extern void set_win_handle_window(HWND hwnd);

//-----------------------------------------------------------------------------
static KeyboardButton::Enum win_translate_key(int32_t winKey)
{
	if ((winKey > 0x40 && winKey < 0x5B) || (winKey > 0x60 && winKey < 0x7B) || (winKey > 0x2F && winKey < 0x3A))
	{
		return (KeyboardButton::Enum)winKey;
	}

	switch (winKey)
	{
		case VK_BACK:		return KeyboardButton::BACKSPACE;
		case VK_TAB:		return KeyboardButton::TAB;
		case VK_SPACE:		return KeyboardButton::SPACE;
		case VK_ESCAPE:		return KeyboardButton::ESCAPE;
		case VK_RETURN:		return KeyboardButton::ENTER;
		case VK_F1:			return KeyboardButton::F1;
		case VK_F2:			return KeyboardButton::F2;
		case VK_F3:			return KeyboardButton::F3;
		case VK_F4:			return KeyboardButton::F4;
		case VK_F5:			return KeyboardButton::F5;
		case VK_F6:			return KeyboardButton::F6;
		case VK_F7:			return KeyboardButton::F7;
		case VK_F8:			return KeyboardButton::F8;
		case VK_F9:			return KeyboardButton::F9;
		case VK_F10:		return KeyboardButton::F10;
		case VK_F11:		return KeyboardButton::F11;
		case VK_F12:		return KeyboardButton::F12;
		case VK_HOME:		return KeyboardButton::HOME;
		case VK_LEFT:		return KeyboardButton::LEFT;
		case VK_UP:			return KeyboardButton::UP;
		case VK_RIGHT:		return KeyboardButton::RIGHT;
		case VK_DOWN:		return KeyboardButton::DOWN;
		case VK_PRIOR:		return KeyboardButton::PAGE_UP;
		case VK_NEXT:		return KeyboardButton::PAGE_DOWN;
		case VK_LSHIFT:		return KeyboardButton::LSHIFT;
		case VK_RSHIFT:		return KeyboardButton::RSHIFT;
		case VK_LCONTROL:	return KeyboardButton::LCONTROL;
		case VK_RCONTROL:	return KeyboardButton::RCONTROL;
		case VK_CAPITAL:	return KeyboardButton::CAPS_LOCK;
		case VK_LMENU:		return KeyboardButton::LALT;
		case VK_RMENU:		return KeyboardButton::RALT;
		case VK_LWIN:		return KeyboardButton::LSUPER;
		case VK_RWIN:		return KeyboardButton::RSUPER;
		case VK_NUMPAD0:	return KeyboardButton::KP_0;
		case VK_NUMPAD1:	return KeyboardButton::KP_1;
		case VK_NUMPAD2:	return KeyboardButton::KP_2;
		case VK_NUMPAD3:	return KeyboardButton::KP_3;
		case VK_NUMPAD4:	return KeyboardButton::KP_4;
		case VK_NUMPAD5:	return KeyboardButton::KP_5;
		case VK_NUMPAD6:	return KeyboardButton::KP_6;
		case VK_NUMPAD7:	return KeyboardButton::KP_7;
		case VK_NUMPAD8:	return KeyboardButton::KP_8;
		case VK_NUMPAD9:	return KeyboardButton::KP_9;
		default:			return KeyboardButton::NONE;
	}
}

static bool s_exit = false;

struct WindowsDevice
{
	WindowsDevice()
		: m_hwnd(0)
	{
	}

	//-----------------------------------------------------------------------------
	int32_t	run(int argc, char** argv)
	{
		HINSTANCE instance = (HINSTANCE)GetModuleHandle(NULL);

		WNDCLASSEX wnd;
		memset(&wnd, 0, sizeof(wnd));
		wnd.cbSize = sizeof(wnd);
		wnd.style = CS_HREDRAW | CS_VREDRAW;
		wnd.lpfnWndProc = window_proc;
		wnd.hInstance = instance;
		wnd.hIcon = LoadIcon(instance, IDI_APPLICATION);
		wnd.hCursor = LoadCursor(instance, IDC_ARROW);
		wnd.lpszClassName = "crown";
		wnd.hIconSm = LoadIcon(instance, IDI_APPLICATION);
		RegisterClassExA(&wnd);

		m_hwnd = CreateWindowA(
					"crown",
					"Crown",
					WS_OVERLAPPEDWINDOW|WS_VISIBLE,
					0,
					0,
					ENTRY_DEFAULT_WIDTH,
					ENTRY_DEFAULT_HEIGHT,
					0,
					NULL,
					instance,
					0
				);

		oswindow_set_window(m_hwnd);
		bgfx::winSetHwnd(m_hwnd);

		m_argc = argc;
		m_argv = argv;

		Thread thread;
		thread.start(WindowsDevice::main_loop, this);

		MSG msg;
		msg.message = WM_NULL;

		while (!s_exit)
		{
			while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		thread.stop();

		shutdown();

		DestroyWindow(m_hwnd);
		return 0;
	}

	//-----------------------------------------------------------------------------
	LRESULT pump_events(HWND hwnd, UINT id, WPARAM wparam, LPARAM lparam)
	{
		switch (id)
		{
			case WM_USER_SET_WINDOW_SIZE:
			{
				uint32_t width = GET_X_LPARAM(lparam);
				uint32_t height = GET_Y_LPARAM(lparam);
				m_queue.push_metrics_event(m_x, m_y, m_width, m_height);
				break;
			}
			case WM_USER_TOGGLE_WINDOW_FRAME:
			{
				m_queue.push_metrics_event(m_x, m_y, m_width, m_height);
				break;
			}
			case WM_DESTROY:
			{
				break;
			}
			case WM_QUIT:
			case WM_CLOSE:
			{
				m_queue.push_exit_event(0);
				break;
			}
			case WM_SIZE:
			{
				uint32_t width = GET_X_LPARAM(lparam);
				uint32_t height = GET_Y_LPARAM(lparam);
				m_queue.push_metrics_event(m_x, m_y, m_width, m_height);
				break;
			}
			case WM_SYSCOMMAND:
			{
				switch (wparam)
				{
					case SC_MINIMIZE:
					case SC_RESTORE:
					{
						HWND parent = GetWindow(hwnd, GW_OWNER);
						if (NULL != parent)
						{
							PostMessage(parent, id, wparam, lparam);
						}
					}
				}
				break;
			}
			case WM_MOUSEMOVE:
			{
				int32_t mx = GET_X_LPARAM(lparam);
				int32_t my = GET_Y_LPARAM(lparam);
				m_queue.push_mouse_event(m_x, m_y);
				break;
			}
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			{
				int32_t mx = GET_X_LPARAM(lparam);
				int32_t my = GET_Y_LPARAM(lparam);
				m_queue.push_mouse_event(mx, my, MouseButton::LEFT, id == WM_LBUTTONDOWN);
				break;
			}
			case WM_RBUTTONUP:
			case WM_RBUTTONDOWN:
			{
				int32_t mx = GET_X_LPARAM(lparam);
				int32_t my = GET_Y_LPARAM(lparam);
				m_queue.push_mouse_event(mx, my, MouseButton::RIGHT, id == WM_RBUTTONDOWN);
				break;
			}
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
			{
				int32_t mx = GET_X_LPARAM(lparam);
				int32_t my = GET_Y_LPARAM(lparam);
				m_queue.push_mouse_event(mx, my, MouseButton::MIDDLE, id == WM_MBUTTONDOWN);
				break;
			}
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
			case WM_KEYUP:
			case WM_SYSKEYUP:
			{
				KeyboardButton::Enum kb = win_translate_key(wparam & 0xff);

				int32_t modifier_mask = 0;

				if (kb == KeyboardButton::LSHIFT || kb == KeyboardButton::RSHIFT)
				{
					(id == WM_KEYDOWN || id == WM_SYSKEYDOWN) ? modifier_mask |= ModifierButton::SHIFT : modifier_mask &= ~ModifierButton::SHIFT;
				}
				else if (kb == KeyboardButton::LCONTROL || kb == KeyboardButton::RCONTROL)
				{
					(id == WM_KEYDOWN || id == WM_SYSKEYDOWN) ? modifier_mask |= ModifierButton::CTRL : modifier_mask &= ~ModifierButton::CTRL;
				}
				else if (kb == KeyboardButton::LALT || kb == KeyboardButton::RALT)
				{
					(id == WM_KEYDOWN || id == WM_SYSKEYDOWN) ? modifier_mask |= ModifierButton::ALT : modifier_mask &= ~ModifierButton::ALT;
				}

				m_queue.push_keyboard_event(modifier_mask, kb, (id == WM_KEYDOWN || id == WM_SYSKEYDOWN));
				break;
			}
			default:
				break;
		}
		return DefWindowProc(hwnd, id, wparam, lparam);
	}

private:

	static LRESULT CALLBACK window_proc(HWND hwnd, UINT id, WPARAM wparam, LPARAM lparam);

public:

	HWND m_hwnd;
	HDC m_hdc;
	uint32_t m_parent_window_handle;
	OsEventQueue m_queue;
};

static WindowsDevice s_wdvc;

LRESULT CALLBACK WindowsDevice::window_proc(HWND hwnd, UINT id, WPARAM wparam, LPARAM lparam)
{
	return s_wdvc.pump_events(hwnd, id, wparam, lparam);
}

} // namespace crown

int main(int argc, char** argv)
{
	WSADATA WsaData;
	int res = WSAStartup(MAKEWORD(2,2), &WsaData);
	CE_ASSERT(res == 0, "Unable to initialize socket");
	CE_UNUSED(WsaData);
	CE_UNUSED(res);

	crown::init();
	int32_t ret = crown::s_wdvc.run(argc, argv);
	crown::shutdown();

	WSACleanup();
	return ret;
}

#endif // CROWN_PLATFORM_WINDOWS
