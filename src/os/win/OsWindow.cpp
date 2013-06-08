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

#include "OsWindow.h"
#include "GLContext.h"
#include "Assert.h"
#include "Keyboard.h"
#include "StringUtils.h"
#include "Log.h"

namespace crown
{

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
OsWindow::OsWindow(uint32_t width, uint32_t height) :
	m_window_handle(NULL),
	m_x(0),
	m_y(0),
	m_fullscreen(false)
{
	CE_ASSERT(width != 0 || height != 0, "Width and height must differ from 0.");

	string::strcpy(m_window_name, "CrownWindowClass");
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_OWNDC;
	wcex.lpfnWndProc = (WNDPROC) DefWindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = GetModuleHandle(NULL);
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = m_window_name;
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	uint32_t registered = RegisterClassEx(&wcex);
	CE_ASSERT(registered != 0, "Unable to register a Window Class.");

	m_window_handle = CreateWindowEx(0, m_window_name, "", WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX, 0, 0, width, height, NULL, NULL, GetModuleHandle(NULL), NULL);
	CE_ASSERT(m_window_handle != NULL, "Unable to create a Window.");
	
	//Save the WGLRenderWindow pointer to the window's user data
	SetWindowLongPtr(m_window_handle, GWLP_USERDATA, (LONG) this);
	RECT rc;
	rc.left = 0;
	rc.top = 0;
	rc.right = width;
	rc.bottom = height;

	int32_t style, styleEx;
	style = GetWindowLong(m_window_handle, GWL_STYLE);
	styleEx = GetWindowLong(m_window_handle, GWL_EXSTYLE);
	AdjustWindowRectEx(&rc, style, false, styleEx);

	SetWindowPos(m_window_handle, 0, 0, 0, rc.right-rc.left, rc.bottom-rc.top, SWP_NOMOVE | SWP_NOZORDER);

	PIXELFORMATDESCRIPTOR pfd;
	int32_t pixel_format;
	/* get the device context (DC) */
	HDC device_context = GetDC(m_window_handle);
	/* set the pixel format for the DC */
	ZeroMemory(&pfd, sizeof (pfd));
	pfd.nSize = sizeof (pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 24;
	pfd.iLayerType = PFD_MAIN_PLANE;

	pixel_format = ChoosePixelFormat(device_context, &pfd);
	CE_ASSERT(pixel_format != 0, "Pixel format not supported.");
	
	bool pf_set = SetPixelFormat(device_context, pixel_format, &pfd);
	CE_ASSERT(pf_set, "Unable to set the pixel format, altough it seems to be supported.");

	set_win_handle_window(m_window_handle);
}

//-----------------------------------------------------------------------------
OsWindow::~OsWindow()
{
	if (m_window_handle)
	{
		DestroyWindow(m_window_handle);
	}
}

//-----------------------------------------------------------------------------
void OsWindow::show()
{
	ShowWindow(m_window_handle, SW_SHOW);
}

//-----------------------------------------------------------------------------
void OsWindow::hide()
{
	ShowWindow(m_window_handle, SW_HIDE);
}

//-----------------------------------------------------------------------------
void OsWindow::get_size(uint32_t& width, uint32_t& height)
{
	width = m_width;
	height = m_height;
}

//-----------------------------------------------------------------------------
void OsWindow::get_position(uint32_t& x, uint32_t& y)
{
	x = m_x;
	y = m_y;
}

//-----------------------------------------------------------------------------
void OsWindow::resize(uint32_t width, uint32_t height)
{
	SetWindowPos(m_window_handle, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
}

//-----------------------------------------------------------------------------
void OsWindow::move(uint32_t x, uint32_t y)
{
	SetWindowPos(m_window_handle, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);	
}

//-----------------------------------------------------------------------------
void OsWindow::show_cursor()
{
	ShowCursor(true);
}

//-----------------------------------------------------------------------------
void OsWindow::hide_cursor()
{
	ShowCursor(false);
}

//-----------------------------------------------------------------------------
void OsWindow::get_cursor_xy(int32_t& x, int32_t& y)
{
	POINT p;

	if (!GetCursorPos(&p))
	{
		CE_ASSERT(false, "Unable to get cursor coordinates\n");
	}

	x = p.x;
	y = p.y;
}

//-----------------------------------------------------------------------------
void OsWindow::set_cursor_xy(int32_t x, int32_t y)
{
	SetCursorPos(x, y);
}

//-----------------------------------------------------------------------------
void OsWindow::set_fullscreen(bool fs)
{
	if (m_fullscreen)
	{
		memset(&m_screen_setting, 0, sizeof(m_screen_setting)); // Makes Sure Memory's Cleared
		m_screen_setting.dmSize = sizeof(m_screen_setting); // Size Of The Devmode Structure
		m_screen_setting.dmPelsWidth = m_width; // Selected Screen Width
		m_screen_setting.dmPelsHeight = m_height; // Selected Screen Height
		m_screen_setting.dmBitsPerPel = 32; // Selected Bits Per Pixel
		m_screen_setting.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&m_screen_setting, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			m_fullscreen = false;
			Log::i("Fullscreen resolution not supported, switching to windowed mode.");
		}
		else
		{
			m_fullscreen = true;
			m_window_handle = CreateWindowEx(0, m_window_name, "", WS_POPUP, 0, 0, m_width, m_height, NULL, NULL, GetModuleHandle(NULL), NULL);
		}
	}
}

bool OsWindow::fullscreen()
{
	return m_fullscreen;
}


//-----------------------------------------------------------------------------
char* OsWindow::title()
{
	return m_window_name;
}

//-----------------------------------------------------------------------------
void OsWindow::set_title(const char* title)
{
	string::strcpy(m_window_name, title);
	SetWindowText(m_window_handle, m_window_name);
}

//-----------------------------------------------------------------------------
void OsWindow::frame()
{
	MSG msg;

	OsEventParameter data_button[4] = {0, 0, 0, 0};
	OsEventParameter data_key[4] = {0, 0, 0, 0};

	// Message are removed with PM_REMOVE
	while (PeekMessage(&msg, m_window_handle, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		switch (msg.message)
		{
			// Mouse
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			{
				OsEventType oset_type;

				if (msg.message == WM_LBUTTONDOWN)
				{
					oset_type = OSET_BUTTON_PRESS;
				}
				if (msg.message == WM_LBUTTONUP)
				{
					oset_type = OSET_BUTTON_RELEASE;
				}

				data_button[0].int_value = LOWORD(msg.lParam);
				data_button[1].int_value = HIWORD(msg.lParam);
				data_button[2].int_value = 0;	// MIDDLE_BUTTON

				push_event(oset_type, data_button[0], data_button[1], data_button[2], data_button[3]);

				break;
			}
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
			{
				OsEventType oset_type;

				if (msg.message == WM_RBUTTONDOWN)
				{
					oset_type = OSET_BUTTON_PRESS;
				}
				if (msg.message == WM_RBUTTONUP)
				{
					oset_type = OSET_BUTTON_RELEASE;
				}

				data_button[0].int_value = LOWORD(msg.lParam);
				data_button[1].int_value = HIWORD(msg.lParam);
				data_button[2].int_value = 1;	// RIGHT BUTTON

				push_event(oset_type, data_button[0], data_button[1], data_button[2], data_button[3]);

				break;
			}
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
			{
				OsEventType oset_type;

				if (msg.message == WM_MBUTTONDOWN)
				{
					oset_type = OSET_BUTTON_PRESS;
				}
				if (msg.message == WM_MBUTTONUP)
				{
					oset_type = OSET_BUTTON_RELEASE;
				}

				data_button[0].int_value = LOWORD(msg.lParam);
				data_button[1].int_value = HIWORD(msg.lParam);
				data_button[2].int_value = 2;	// MIDDLE_BUTTON

				push_event(oset_type, data_button[0], data_button[1], data_button[2], data_button[3]);

				break;
			}
			case WM_MOUSEMOVE:
			{
				data_button[0].int_value = LOWORD(msg.lParam);
				data_button[0].int_value = HIWORD(msg.lParam);
				push_event(OSET_MOTION_NOTIFY, data_button[0], data_button[1], data_button[2], data_button[3]);
				break;
			}

			// Keyboard
			case WM_KEYDOWN:
			case WM_KEYUP:
			{
				Key kc = translate_key(msg.wParam);

				int32_t modifier_mask = 0;

				// Check if any modifier key is pressed or released
				if (kc == KC_LSHIFT || kc == KC_RSHIFT)
				{
					(msg.message == WM_KEYDOWN) ? modifier_mask |= MK_SHIFT : modifier_mask &= ~MK_SHIFT;
				}
				else if (kc == KC_LCONTROL || kc == KC_RCONTROL)
				{
					(msg.message == WM_KEYDOWN) ? modifier_mask |= MK_CTRL : modifier_mask &= ~MK_CTRL;
				}
				else if (kc == KC_LALT || kc == KC_RALT)
				{
					(msg.message == WM_KEYDOWN) ? modifier_mask |= MK_ALT : modifier_mask &= ~MK_ALT;
				}

				OsEventType oset_type = msg.message == WM_KEYDOWN ? OSET_KEY_PRESS : OSET_KEY_RELEASE;

				data_key[0].int_value = ((int32_t)kc);
				data_key[1].int_value = modifier_mask;

				push_event(oset_type, data_key[0], data_key[1], data_key[2], data_key[3]);

				break;
			}
			case WM_CHAR:
			{
				// MUST BE IMPLEMENTED
				break;
			}

			default:
			{
				break;
			}

			// Touch
			// TODO

			// Accelerometer
			// TODO
		}
	}

}

} // namespace crown
