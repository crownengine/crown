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

#include <windowsx.h>

#define WM_USER_SET_WINDOW_SIZE     (WM_USER+0)
#define WM_USER_TOGGLE_WINDOW_FRAME (WM_USER+1)
#define WM_USER_MOUSE_LOCK          (WM_USER+2)

#include "Crown.h"

#define ENTRY_DEFAULT_WIDTH 1024
#define ENTRY_DEFAULT_HEIGHT 768

namespace crown
{

extern void set_win_handle_window(HWND hwnd);

//-----------------------------------------------------------------------------
void init()
{
	memory::init();
	os::init_os();
}

//-----------------------------------------------------------------------------
void shutdown()
{
	memory::shutdown();
}

//-----------------------------------------------------------------------------
static Key translate_key(int32_t winKey)
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

//-----------------------------------------------------------------------------
class WindowsDevice : public Device
{
public:

	WindowsDevice() :
		m_hwnd(0),
		m_style(0),
		m_width(0),
		m_height(0),
		m_old_width(0),
		m_old_height(0),
		m_frame_width(0),
		m_frame_height(0),
		m_aspect_ratio(0.0f),
		m_x(0),
		m_y(0),
		m_frame(false),
		m_mouse_lock(false),
		m_started(false),
		m_exit(false)
	{
	}

	//-----------------------------------------------------------------------------
	int32_t	run(int argc, char** argv)
	{
		HINSTANCE instance = (HINSTANCE)GetModuleHandle(NULL);

		WNDCLASSEX wnd;
		memset(&wnd, 0, sizeof(wnd) );
		wnd.cbSize = sizeof(wnd);
		wnd.lpfnWndProc = DefWindowProc;
		wnd.hInstance = instance;
		wnd.hIcon = LoadIcon(instance, IDI_APPLICATION);
		wnd.hCursor = LoadCursor(instance, IDC_ARROW);
		wnd.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wnd.lpszClassName = "crown_letterbox";
		wnd.hIconSm = LoadIcon(instance, IDI_APPLICATION);
		RegisterClassExA(&wnd);

		memset(&wnd, 0, sizeof(wnd) );
		wnd.cbSize = sizeof(wnd);
		wnd.style = CS_HREDRAW | CS_VREDRAW;
		wnd.lpfnWndProc = window_proc;
		wnd.hInstance = instance;
		wnd.hIcon = LoadIcon(instance, IDI_APPLICATION);
		wnd.hCursor = LoadCursor(instance, IDC_ARROW);
		wnd.lpszClassName = "crown";
		wnd.hIconSm = LoadIcon(instance, IDI_APPLICATION);
		RegisterClassExA(&wnd);

		HWND hwnd = CreateWindowA("crown_letterbox", "CROWN", WS_POPUP|WS_SYSMENU, -32000, -32000, 0,
									0, NULL, NULL, instance, 0);

		m_hwnd = CreateWindowA("crown", "CROWN", WS_OVERLAPPEDWINDOW|WS_VISIBLE, 0, 0, ENTRY_DEFAULT_WIDTH,
								ENTRY_DEFAULT_HEIGHT, hwnd, NULL, instance, 0);

		set_win_handle_window(m_hwnd);

		adjust(ENTRY_DEFAULT_WIDTH, ENTRY_DEFAULT_HEIGHT, true);
		m_width = ENTRY_DEFAULT_WIDTH;
		m_height = ENTRY_DEFAULT_HEIGHT;
		m_old_width = ENTRY_DEFAULT_WIDTH;
		m_old_height = ENTRY_DEFAULT_HEIGHT;

		m_argc = argc;
		m_argv = argv;

		OsThread thread("game-loop");
		thread.start(WindowsDevice::thread_proc, this);
		m_started = true;

//		m_eventQueue.postSizeEvent(m_width, m_height);

		MSG msg;
		msg.message = WM_NULL;

		while (!m_exit)
		{
			WaitMessage();

			while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		thread.stop();

		DestroyWindow(m_hwnd);
		DestroyWindow(hwnd);

		return 0;
	}

	//-----------------------------------------------------------------------------
	void adjust(uint32_t width, uint32_t height, bool window_frame)
	{
		// Nothing right now
	}

	//-----------------------------------------------------------------------------
	void set_mouse_pos(int32_t x, int32_t y)
	{
		POINT pt = {x, y};
		ClientToScreen(m_hwnd, &pt);
		SetCursorPos(pt.x, pt.y);
	}

	//-----------------------------------------------------------------------------
	void setMouseLock(bool lock)
	{
		if (lock != m_mouse_lock)
		{
			if (lock)
			{
				m_x = m_width / 2;
				m_y = m_height / 2;
				ShowCursor(false);
				set_mouse_pos(m_x, m_y);
			}
			else
			{
				set_mouse_pos(m_x, m_y);
				ShowCursor(true);
			}

			m_mouse_lock = lock;
		}
	}

public:
	
	static int32_t thread_proc(void* data)
	{
		WindowsDevice* engine = (WindowsDevice*)data;

		if (!engine->init(engine->m_argc, engine->m_argv))
		{
			exit(-1);
		}

		while (engine->is_running())
		{
			engine->frame();
		}

		engine->shutdown();

		return 0;
	}

	LRESULT process(HWND hwnd, UINT id, WPARAM wparam, LPARAM lparam)
	{
		if (m_started)
		{
			switch (id)
			{
			case WM_USER_SET_WINDOW_SIZE:
			{
				uint32_t width = GET_X_LPARAM(lparam);
				uint32_t height = GET_Y_LPARAM(lparam);
				adjust(width, height, true);
				break;
			}
			case WM_USER_TOGGLE_WINDOW_FRAME:
			{
				if (m_frame)
				{
					m_old_width = m_width;
					m_old_height = m_height;
				}
				adjust(m_old_width, m_old_height, !m_frame);
				break;
			}
			case WM_USER_MOUSE_LOCK:
			{
				// setMouseLock(!!lparam);
				break;
			}
			case WM_DESTROY:
			{
				break;
			}
			case WM_QUIT:
			case WM_CLOSE:
			{
				Log::i("EXIT");
				m_exit = true;
				// m_eventQueue.postExitEvent();
				break;
			}
			case WM_SIZING:
			{
				RECT& rect = *(RECT*)lparam;
				uint32_t width = rect.right - rect.left - m_frame_width;
				uint32_t height = rect.bottom - rect.top - m_frame_height;

				//Recalculate size according to aspect ratio
				switch (wparam)
				{
				case WMSZ_LEFT:
				case WMSZ_RIGHT:
					{
						float aspect_ratio = 1.0f / m_aspect_ratio;
						width = math::max((uint32_t)ENTRY_DEFAULT_WIDTH / 4, width);
						height = uint32_t(float(width)*aspect_ratio);
					}
					break;

				default:
					{
						float aspect_ratio = m_aspect_ratio;
						height = math::max((uint32_t)ENTRY_DEFAULT_HEIGHT / 4, height);
						width = uint32_t(float(height)*aspect_ratio);
					}
					break;
				}

				//Recalculate position using different anchor points
				switch(wparam)
				{
				case WMSZ_LEFT:
				case WMSZ_TOPLEFT:
				case WMSZ_BOTTOMLEFT:
					rect.left = rect.right - width - m_frame_width;
					rect.bottom = rect.top + height + m_frame_height;
					break;

				default:
					rect.right = rect.left + width + m_frame_width;
					rect.bottom = rect.top + height + m_frame_height;
					break;
				}

				// m_eventQueue.postSizeEvent(m_width, m_height);
				return 0;
			}
			case WM_SIZE:
			{
				uint32_t width = GET_X_LPARAM(lparam);
				uint32_t height = GET_Y_LPARAM(lparam);

				m_width = width;
				m_height = height;
				// m_eventQueue.postSizeEvent(m_width, m_height);
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

				if (m_mouse_lock)
				{
					mx -= m_x;
					my -= m_y;

					if (mx == 0	&& my == 0)
					{
						break;
					}

					set_mouse_pos(m_x, m_y);
				}

				// m_eventQueue.postMouseEvent(mx, my);
				break;
			}
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_LBUTTONDBLCLK:
			{
				int32_t mx = GET_X_LPARAM(lparam);
				int32_t my = GET_Y_LPARAM(lparam);
				// m_eventQueue.postMouseEvent(mx, my, MouseButton::Left, id == WM_LBUTTONDOWN);
				Log::i("Left Click! '%d' '%d'", mx, my);
				break;
			}
			case WM_RBUTTONUP:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONDBLCLK:
			{
				int32_t mx = GET_X_LPARAM(lparam);
				int32_t my = GET_Y_LPARAM(lparam);
				// m_eventQueue.postMouseEvent(mx, my, MouseButton::Right, id == WM_RBUTTONDOWN);
				Log::i("Right Click! '%d' '%d'", mx, my);
				break;
			}
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
			case WM_MBUTTONDBLCLK:
			{
				int32_t mx = GET_X_LPARAM(lparam);
				int32_t my = GET_Y_LPARAM(lparam);
				// m_eventQueue.postMouseEvent(mx, my, MouseButton::Middle, id == WM_MBUTTONDOWN);
				Log::i("Middle Click! '%d' '%d'", mx, my);

				break;
			}
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
			case WM_KEYUP:
			case WM_SYSKEYUP:
			{
				Key key = translate_key(wparam);
				uint8_t modifiers = 0;
				// m_eventQueue.postKeyEvent(key, modifiers, id == WM_KEYDOWN || id == WM_SYSKEYDOWN);
				Log::i("Key! '%d'", key);
				break;
			}
			default:
				break;
			}
		}

		return DefWindowProc(hwnd, id, wparam, lparam);
	}

private:

	static LRESULT CALLBACK window_proc(HWND hwnd, UINT id, WPARAM wparam, LPARAM lparam);

public:

	HWND m_hwnd;
	RECT m_rect;
	DWORD m_style;
	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_old_width;
	uint32_t m_old_height;
	uint32_t m_frame_width;
	uint32_t m_frame_height;
	float m_aspect_ratio;
	int32_t m_x;
	int32_t m_y;
	bool m_frame;
	bool m_mouse_lock;
	bool m_started;
	bool m_exit;

	int32_t m_argc;
	char**	m_argv;
};

WindowsDevice* engine;

LRESULT CALLBACK WindowsDevice::window_proc(HWND hwnd, UINT id, WPARAM wparam, LPARAM lparam)
{
	return ((WindowsDevice*)engine)->process(hwnd, id, wparam, lparam);
}

} // namespace crown


int main(int argc, char** argv)
{
	using namespace crown;
	init();

	engine = CE_NEW(default_allocator(), WindowsDevice)();
	set_device(crown::engine);
	
	((WindowsDevice*)engine)->run(argc, argv);

	shutdown();

	return 0;
}