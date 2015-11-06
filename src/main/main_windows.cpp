/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "config.h"

#if CROWN_PLATFORM_WINDOWS

#include "os_event_queue.h"
#include "thread.h"
#include "crown.h"
#include "command_line.h"
#include "console_server.h"
#include "bundle_compiler.h"
#include <bgfxplatform.h>
#include <winsock2.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windowsx.h>

namespace crown
{

static KeyboardButton::Enum win_translate_key(int32_t winkey)
{
	switch (winkey)
	{
		case VK_BACK:     return KeyboardButton::BACKSPACE;
		case VK_TAB:      return KeyboardButton::TAB;
		case VK_SPACE:    return KeyboardButton::SPACE;
		case VK_ESCAPE:   return KeyboardButton::ESCAPE;
		case VK_RETURN:   return KeyboardButton::ENTER;
		case VK_F1:       return KeyboardButton::F1;
		case VK_F2:       return KeyboardButton::F2;
		case VK_F3:       return KeyboardButton::F3;
		case VK_F4:       return KeyboardButton::F4;
		case VK_F5:       return KeyboardButton::F5;
		case VK_F6:       return KeyboardButton::F6;
		case VK_F7:       return KeyboardButton::F7;
		case VK_F8:       return KeyboardButton::F8;
		case VK_F9:       return KeyboardButton::F9;
		case VK_F10:      return KeyboardButton::F10;
		case VK_F11:      return KeyboardButton::F11;
		case VK_F12:      return KeyboardButton::F12;
		case VK_HOME:     return KeyboardButton::HOME;
		case VK_LEFT:     return KeyboardButton::LEFT;
		case VK_UP:       return KeyboardButton::UP;
		case VK_RIGHT:    return KeyboardButton::RIGHT;
		case VK_DOWN:     return KeyboardButton::DOWN;
		case VK_PRIOR:    return KeyboardButton::PAGE_UP;
		case VK_NEXT:     return KeyboardButton::PAGE_DOWN;
		case VK_LSHIFT:   return KeyboardButton::LSHIFT;
		case VK_RSHIFT:   return KeyboardButton::RSHIFT;
		case VK_LCONTROL: return KeyboardButton::LCONTROL;
		case VK_RCONTROL: return KeyboardButton::RCONTROL;
		case VK_CAPITAL:  return KeyboardButton::CAPS_LOCK;
		case VK_LMENU:    return KeyboardButton::LALT;
		case VK_RMENU:    return KeyboardButton::RALT;
		case VK_LWIN:     return KeyboardButton::LSUPER;
		case VK_RWIN:     return KeyboardButton::RSUPER;
		case VK_NUMPAD0:  return KeyboardButton::KP_0;
		case VK_NUMPAD1:  return KeyboardButton::KP_1;
		case VK_NUMPAD2:  return KeyboardButton::KP_2;
		case VK_NUMPAD3:  return KeyboardButton::KP_3;
		case VK_NUMPAD4:  return KeyboardButton::KP_4;
		case VK_NUMPAD5:  return KeyboardButton::KP_5;
		case VK_NUMPAD6:  return KeyboardButton::KP_6;
		case VK_NUMPAD7:  return KeyboardButton::KP_7;
		case VK_NUMPAD8:  return KeyboardButton::KP_8;
		case VK_NUMPAD9:  return KeyboardButton::KP_9;
		case '0':         return KeyboardButton::NUM_0;
		case '1':         return KeyboardButton::NUM_1;
		case '2':         return KeyboardButton::NUM_2;
		case '3':         return KeyboardButton::NUM_3;
		case '4':         return KeyboardButton::NUM_4;
		case '5':         return KeyboardButton::NUM_5;
		case '6':         return KeyboardButton::NUM_6;
		case '7':         return KeyboardButton::NUM_7;
		case '8':         return KeyboardButton::NUM_8;
		case '9':         return KeyboardButton::NUM_9;
		case 'A':         return KeyboardButton::A;
		case 'B':         return KeyboardButton::B;
		case 'C':         return KeyboardButton::C;
		case 'D':         return KeyboardButton::D;
		case 'E':         return KeyboardButton::E;
		case 'F':         return KeyboardButton::F;
		case 'G':         return KeyboardButton::G;
		case 'H':         return KeyboardButton::H;
		case 'I':         return KeyboardButton::I;
		case 'J':         return KeyboardButton::J;
		case 'K':         return KeyboardButton::K;
		case 'L':         return KeyboardButton::L;
		case 'M':         return KeyboardButton::M;
		case 'N':         return KeyboardButton::N;
		case 'O':         return KeyboardButton::O;
		case 'P':         return KeyboardButton::P;
		case 'Q':         return KeyboardButton::Q;
		case 'R':         return KeyboardButton::R;
		case 'S':         return KeyboardButton::S;
		case 'T':         return KeyboardButton::T;
		case 'U':         return KeyboardButton::U;
		case 'V':         return KeyboardButton::V;
		case 'W':         return KeyboardButton::W;
		case 'X':         return KeyboardButton::X;
		case 'Y':         return KeyboardButton::Y;
		case 'Z':         return KeyboardButton::Z;
		default:          return KeyboardButton::NONE;
	}
}

static bool s_exit = false;

struct MainThreadArgs
{
	DeviceOptions* opts;
};

int32_t func(void* data)
{
	MainThreadArgs* args = (MainThreadArgs*)data;
	crown::init(*args->opts);
	crown::update();
	crown::shutdown();
	s_exit = true;
	return EXIT_SUCCESS;
}

struct WindowsDevice
{
	WindowsDevice()
		: _hwnd(NULL)
		, _hdc(NULL)
	{
	}

	int32_t	run(DeviceOptions* opts)
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

		_hwnd = CreateWindowA("crown"
			, "Crown"
			, WS_OVERLAPPEDWINDOW | WS_VISIBLE
			, opts->window_x()
			, opts->window_y()
			, opts->window_width()
			, opts->window_height()
			, 0
			, NULL
			, instance
			, 0);
		CE_ASSERT(_hwnd != NULL, "CreateWindowA: GetLastError = %d", GetLastError());

		bgfx::winSetHwnd(_hwnd);

		MainThreadArgs mta;
		mta.opts = opts;

		Thread main_thread;
		main_thread.start(func, &mta);

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

		main_thread.stop();
		DestroyWindow(_hwnd);
		return EXIT_SUCCESS;
	}

	LRESULT pump_events(HWND hwnd, UINT id, WPARAM wparam, LPARAM lparam)
	{
		switch (id)
		{
			case WM_DESTROY:
			{
				break;
			}
			case WM_QUIT:
			case WM_CLOSE:
			{
				s_exit = true;
				_queue.push_exit_event(0);
				return 0;
			}
			case WM_SIZE:
			{
				uint32_t width = GET_X_LPARAM(lparam);
				uint32_t height = GET_Y_LPARAM(lparam);
				_queue.push_metrics_event(0, 0, width, height);
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
			case WM_MOUSEWHEEL:
			{
				int32_t mx = GET_X_LPARAM(lparam);
				int32_t my = GET_Y_LPARAM(lparam);
				short delta = GET_WHEEL_DELTA_WPARAM(wparam);
				_queue.push_mouse_event(mx, my, (float)(delta/WHEEL_DELTA));
				break;
			}
			case WM_MOUSEMOVE:
			{
				int32_t mx = GET_X_LPARAM(lparam);
				int32_t my = GET_Y_LPARAM(lparam);
				_queue.push_mouse_event(mx, my);
				break;
			}
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			{
				int32_t mx = GET_X_LPARAM(lparam);
				int32_t my = GET_Y_LPARAM(lparam);
				_queue.push_mouse_event(mx, my, MouseButton::LEFT, id == WM_LBUTTONDOWN);
				break;
			}
			case WM_RBUTTONUP:
			case WM_RBUTTONDOWN:
			{
				int32_t mx = GET_X_LPARAM(lparam);
				int32_t my = GET_Y_LPARAM(lparam);
				_queue.push_mouse_event(mx, my, MouseButton::RIGHT, id == WM_RBUTTONDOWN);
				break;
			}
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
			{
				int32_t mx = GET_X_LPARAM(lparam);
				int32_t my = GET_Y_LPARAM(lparam);
				_queue.push_mouse_event(mx, my, MouseButton::MIDDLE, id == WM_MBUTTONDOWN);
				break;
			}
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
			case WM_KEYUP:
			case WM_SYSKEYUP:
			{
				KeyboardButton::Enum kb = win_translate_key(wparam & 0xff);

				_queue.push_keyboard_event(kb, (id == WM_KEYDOWN || id == WM_SYSKEYDOWN));
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

	HWND _hwnd;
	HDC _hdc;
	OsEventQueue _queue;
};

static WindowsDevice s_wdvc;

LRESULT CALLBACK WindowsDevice::window_proc(HWND hwnd, UINT id, WPARAM wparam, LPARAM lparam)
{
	return s_wdvc.pump_events(hwnd, id, wparam, lparam);
}

bool next_event(OsEvent& ev)
{
	return s_wdvc._queue.pop_event(ev);
}
} // namespace crown

int main(int argc, char** argv)
{
	using namespace crown;
	memory_globals::init();

	WSADATA dummy;
	int err = WSAStartup(MAKEWORD(2, 2), &dummy);
	CE_ASSERT(err == 0, "WSAStartup: error = %d", err);
	CE_UNUSED(dummy);
	CE_UNUSED(err);

	DeviceOptions opts(argc, argv);

	console_server_globals::init(opts.console_port(), opts.wait_console());
	bundle_compiler_globals::init(opts.source_dir(), opts.bundle_dir());

	bool do_continue = true;
	int exitcode = EXIT_SUCCESS;

	do_continue = bundle_compiler::main(opts.do_compile(), opts.do_continue(), opts.platform());

	if (do_continue)
		exitcode = crown::s_wdvc.run(&opts);

	bundle_compiler_globals::shutdown();
	console_server_globals::shutdown();

	WSACleanup();

	memory_globals::shutdown();
	return exitcode;
}

#endif // CROWN_PLATFORM_WINDOWS
