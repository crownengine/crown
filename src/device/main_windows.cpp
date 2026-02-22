/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#if CROWN_PLATFORM_WINDOWS
#include "core/command_line.h"
#include "core/containers/array.inl"
#include "core/guid.h"
#include "core/memory/globals.h"
#include "core/memory/memory.inl"
#include "core/option.inl"
#include "core/profiler.h"
#include "core/thread/mpsc_queue.inl"
#include "core/thread/thread.h"
#include "core/unit_tests.h"
#include "device/device.h"
#include "device/device_event_queue.inl"
#include "resource/data_compiler.h"
#include <fcntl.h>    // _O_TEXT
#include <io.h>       // _open_osfhandle
#include <stdio.h>    // FILE, freopen, stdio etc.
#include <windowsx.h> // GET_X_LPARAM, GET_Y_LPARAM
#include <winsock2.h>
#include <xinput.h>
#define STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_NOUNALIGNED
#include <stb_sprintf.h>

namespace crown
{
static KeyboardButton::Enum win_translate_key(s32 winkey)
{
	switch (winkey) {
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
	case VK_INSERT:   return KeyboardButton::INS;
	case VK_DELETE:   return KeyboardButton::DEL;
	case VK_END:      return KeyboardButton::END;
	case VK_LSHIFT:   return KeyboardButton::SHIFT_LEFT;
	case VK_RSHIFT:   return KeyboardButton::SHIFT_RIGHT;
	case VK_LCONTROL: return KeyboardButton::CTRL_LEFT;
	case VK_RCONTROL: return KeyboardButton::CTRL_RIGHT;
	case VK_CAPITAL:  return KeyboardButton::CAPS_LOCK;
	case VK_LMENU:    return KeyboardButton::ALT_LEFT;
	case VK_RMENU:    return KeyboardButton::ALT_RIGHT;
	case VK_LWIN:     return KeyboardButton::SUPER_LEFT;
	case VK_RWIN:     return KeyboardButton::SUPER_RIGHT;
	case VK_NUMLOCK:  return KeyboardButton::NUM_LOCK;
	case VK_DECIMAL:  return KeyboardButton::NUMPAD_DELETE;
	case VK_MULTIPLY: return KeyboardButton::NUMPAD_MULTIPLY;
	case VK_ADD:      return KeyboardButton::NUMPAD_ADD;
	case VK_SUBTRACT: return KeyboardButton::NUMPAD_SUBTRACT;
	case VK_DIVIDE:   return KeyboardButton::NUMPAD_DIVIDE;
	case VK_NUMPAD0:  return KeyboardButton::NUMPAD_0;
	case VK_NUMPAD1:  return KeyboardButton::NUMPAD_1;
	case VK_NUMPAD2:  return KeyboardButton::NUMPAD_2;
	case VK_NUMPAD3:  return KeyboardButton::NUMPAD_3;
	case VK_NUMPAD4:  return KeyboardButton::NUMPAD_4;
	case VK_NUMPAD5:  return KeyboardButton::NUMPAD_5;
	case VK_NUMPAD6:  return KeyboardButton::NUMPAD_6;
	case VK_NUMPAD7:  return KeyboardButton::NUMPAD_7;
	case VK_NUMPAD8:  return KeyboardButton::NUMPAD_8;
	case VK_NUMPAD9:  return KeyboardButton::NUMPAD_9;
	case '0':         return KeyboardButton::NUMBER_0;
	case '1':         return KeyboardButton::NUMBER_1;
	case '2':         return KeyboardButton::NUMBER_2;
	case '3':         return KeyboardButton::NUMBER_3;
	case '4':         return KeyboardButton::NUMBER_4;
	case '5':         return KeyboardButton::NUMBER_5;
	case '6':         return KeyboardButton::NUMBER_6;
	case '7':         return KeyboardButton::NUMBER_7;
	case '8':         return KeyboardButton::NUMBER_8;
	case '9':         return KeyboardButton::NUMBER_9;
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
	default:          return KeyboardButton::COUNT;
	}
}

struct XinputToJoypad
{
	WORD bit;
	JoypadButton::Enum button;
};

static XinputToJoypad s_xinput_to_joypad[] =
{
	{ XINPUT_GAMEPAD_DPAD_UP,        JoypadButton::UP             },
	{ XINPUT_GAMEPAD_DPAD_DOWN,      JoypadButton::DOWN           },
	{ XINPUT_GAMEPAD_DPAD_LEFT,      JoypadButton::LEFT           },
	{ XINPUT_GAMEPAD_DPAD_RIGHT,     JoypadButton::RIGHT          },
	{ XINPUT_GAMEPAD_START,          JoypadButton::START          },
	{ XINPUT_GAMEPAD_BACK,           JoypadButton::BACK           },
	{ XINPUT_GAMEPAD_LEFT_THUMB,     JoypadButton::THUMB_LEFT     },
	{ XINPUT_GAMEPAD_RIGHT_THUMB,    JoypadButton::THUMB_RIGHT    },
	{ XINPUT_GAMEPAD_LEFT_SHOULDER,  JoypadButton::SHOULDER_LEFT  },
	{ XINPUT_GAMEPAD_RIGHT_SHOULDER, JoypadButton::SHOULDER_RIGHT },
	{ XINPUT_GAMEPAD_A,              JoypadButton::A              },
	{ XINPUT_GAMEPAD_B,              JoypadButton::B              },
	{ XINPUT_GAMEPAD_X,              JoypadButton::X              },
	{ XINPUT_GAMEPAD_Y,              JoypadButton::Y              }
};

// See: https://learn.microsoft.com/en-us/windows/win32/xinput/xinput-versions
static const char *s_xinput_dll_names[] =
{
	"xinput1_4.dll",
	"xinput1_3.dll",
	"xinput9_1_0.dll"
};

#define XINPUT_IMPORT() \
	DL_IMPORT_FUNC(XInputGetState, DWORD, (DWORD, XINPUT_STATE *))

#define DL_IMPORT_FUNC(func_name, return_type, params) \
	typedef return_type (*PROTO_ ## func_name)params;  \
	static PROTO_ ## func_name func_name

XINPUT_IMPORT();

#undef DL_IMPORT_FUNC

struct Joypad
{
	struct Axis
	{
		s16 lx, ly, lz;
		s16 rx, ry, rz;
	};

	XINPUT_STATE _state[CROWN_MAX_JOYPADS];
	Axis _axis[CROWN_MAX_JOYPADS];
	bool _connected[CROWN_MAX_JOYPADS];

	Joypad()
	{
		memset(&_state, 0, sizeof(_state));
		memset(&_axis, 0, sizeof(_axis));
		memset(&_connected, 0, sizeof(_connected));
	}

	void update(DeviceEventQueue &queue)
	{
		if (XInputGetState == NULL)
			return;

		for (u8 i = 0; i < CROWN_MAX_JOYPADS; ++i) {
			XINPUT_STATE state;
			memset(&state, 0, sizeof(state));

			const DWORD result = XInputGetState(i, &state);
			const bool connected = result == ERROR_SUCCESS;

			if (connected != _connected[i])
				queue.push_status_event(InputDeviceType::JOYPAD, i, connected);

			_connected[i] = connected;

			if (!connected || state.dwPacketNumber == _state[i].dwPacketNumber)
				continue;

			XINPUT_GAMEPAD &gamepad = _state[i].Gamepad;

			const WORD diff = state.Gamepad.wButtons ^ gamepad.wButtons;
			const WORD curr = state.Gamepad.wButtons;
			if (diff != 0) {
				for (u8 bb = 0; bb < countof(s_xinput_to_joypad); ++bb) {
					WORD bit = s_xinput_to_joypad[bb].bit;
					if (bit & diff) {
						queue.push_button_event(InputDeviceType::JOYPAD
							, i
							, s_xinput_to_joypad[bb].button
							, (curr & bit) != 0
							);
						gamepad.wButtons = curr;
					}
				}
			}

			if (state.Gamepad.sThumbLX != gamepad.sThumbLX) {
				_axis[i].lx = state.Gamepad.sThumbLX;
				queue.push_axis_event(InputDeviceType::JOYPAD
					, i
					, JoypadAxis::LEFT
					, _axis[i].lx
					, _axis[i].ly
					, 0
					);

				gamepad.sThumbLX = state.Gamepad.sThumbLX;
			}
			if (state.Gamepad.sThumbLY != gamepad.sThumbLY) {
				_axis[i].ly = state.Gamepad.sThumbLY;
				queue.push_axis_event(InputDeviceType::JOYPAD
					, i
					, JoypadAxis::LEFT
					, _axis[i].lx
					, _axis[i].ly
					, 0
					);

				gamepad.sThumbLY = state.Gamepad.sThumbLY;
			}
			if (state.Gamepad.sThumbRX != gamepad.sThumbRX) {
				_axis[i].rx = state.Gamepad.sThumbRX;
				queue.push_axis_event(InputDeviceType::JOYPAD
					, i
					, JoypadAxis::RIGHT
					, _axis[i].rx
					, _axis[i].ry
					, 0
					);

				gamepad.sThumbRX = state.Gamepad.sThumbRX;
			}
			if (state.Gamepad.sThumbRY != gamepad.sThumbRY) {
				_axis[i].ry = state.Gamepad.sThumbRY;
				queue.push_axis_event(InputDeviceType::JOYPAD
					, i
					, JoypadAxis::RIGHT
					, _axis[i].rx
					, _axis[i].ry
					, 0
					);

				gamepad.sThumbRY = state.Gamepad.sThumbRY;
			}
			if (state.Gamepad.bLeftTrigger != gamepad.bLeftTrigger) {
				_axis[i].lz = state.Gamepad.bLeftTrigger;
				queue.push_axis_event(InputDeviceType::JOYPAD
					, i
					, JoypadAxis::TRIGGER_LEFT
					, 0
					, 0
					, _axis[i].lz
					);

				gamepad.bLeftTrigger = state.Gamepad.bLeftTrigger;
			}
			if (state.Gamepad.bRightTrigger != gamepad.bRightTrigger) {
				_axis[i].rz = state.Gamepad.bRightTrigger;
				queue.push_axis_event(InputDeviceType::JOYPAD
					, i
					, JoypadAxis::TRIGGER_RIGHT
					, 0
					, 0
					, _axis[i].rz
					);

				gamepad.bRightTrigger = state.Gamepad.bRightTrigger;
			}
		}
	}
};

static bool s_exit = false;
static HCURSOR _win_cursors[MouseCursor::COUNT];
static bool push_event(const OsEvent &ev);

struct WindowsDevice
{
	HWND _hwnd;
	HCURSOR _hcursor;
	MPSCQueue<OsEvent, CROWN_MAX_OS_EVENTS> _events;
	DeviceEventQueue _queue;
	Joypad _joypad;
	s16 _mouse_last_x;
	s16 _mouse_last_y;
	CursorMode::Enum _cursor_mode;
	DeviceOptions *_options;
	void *_xinput_lib;

	WindowsDevice(Allocator &a, DeviceOptions &opts)
		: _hwnd(NULL)
		, _hcursor(NULL)
		, _events(a)
		, _queue(push_event)
		, _mouse_last_x(INT16_MAX)
		, _mouse_last_y(INT16_MAX)
		, _cursor_mode(CursorMode::NORMAL)
		, _options(&opts)
		, _xinput_lib(NULL)
	{
#define DL_IMPORT_FUNC(func_name, return_type, params)                             \
	func_name = (PROTO_ ## func_name)os::library_symbol(_xinput_lib, # func_name); \
	CE_ENSURE(func_name != NULL);

		for (u32 i = 0; i < countof(s_xinput_dll_names); ++i) {
			if ((_xinput_lib = os::library_open(s_xinput_dll_names[i])) != NULL) {
				XINPUT_IMPORT();
				break;
			}
		}

#undef DL_IMPORT_FUNC
	}

	~WindowsDevice()
	{
		if (_xinput_lib != NULL)
			os::library_close(_xinput_lib);
	}

	int run()
	{
		HINSTANCE instance = (HINSTANCE)GetModuleHandle(NULL);
		WNDCLASSEX wnd;
		memset(&wnd, 0, sizeof(wnd));
		wnd.cbSize = sizeof(wnd);
		wnd.style = CS_HREDRAW | CS_VREDRAW;
		wnd.lpfnWndProc = window_proc;
		wnd.hInstance = instance;
		wnd.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wnd.hCursor = LoadCursor(NULL, IDC_ARROW);
		wnd.lpszClassName = "crown";
		wnd.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
		RegisterClassExA(&wnd);

		DWORD style = 0;
		DWORD exstyle = 0;
		if (_options->_parent_window == 0) {
			style |= WS_OVERLAPPEDWINDOW;
		} else {
			style |= WS_POPUP;
			exstyle |= WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_NOACTIVATE;
		}

		RECT rect;
		rect.left   = 0;
		rect.top    = 0;
		rect.right  = _options->_window_width.value();
		rect.bottom = _options->_window_height.value();
		AdjustWindowRect(&rect, style, FALSE);

		_hwnd = CreateWindowExA(exstyle
			, "crown"
			, "Crown"
			, style
			, _options->_window_x
			, _options->_window_y
			, rect.right - rect.left
			, rect.bottom - rect.top
			, 0
			, NULL
			, instance
			, NULL
			);
		CE_ASSERT(_hwnd != NULL, "CreateWindowA: GetLastError = %d", GetLastError());

		if (_options->_parent_window != 0)
			SetParent(_hwnd, (HWND)(UINT_PTR)_options->_parent_window);

		RAWINPUTDEVICE rid;
		rid.usUsagePage = 0x01;
		rid.usUsage = 0x06;
		rid.dwFlags = RIDEV_NOLEGACY;
		rid.hwndTarget = _hwnd;
		RegisterRawInputDevices(&rid, 1, sizeof(rid));

		_hcursor = LoadCursorA(NULL, IDC_ARROW);

		// Create standard cursors
		_win_cursors[MouseCursor::ARROW]               = LoadCursorA(NULL, IDC_ARROW);
		_win_cursors[MouseCursor::HAND]                = LoadCursorA(NULL, IDC_HAND);
		_win_cursors[MouseCursor::TEXT_INPUT]          = LoadCursorA(NULL, IDC_IBEAM);
		_win_cursors[MouseCursor::CORNER_TOP_LEFT]     = LoadCursorA(NULL, IDC_SIZENESW);
		_win_cursors[MouseCursor::CORNER_TOP_RIGHT]    = LoadCursorA(NULL, IDC_SIZENWSE);
		_win_cursors[MouseCursor::CORNER_BOTTOM_LEFT]  = LoadCursorA(NULL, IDC_SIZENWSE);
		_win_cursors[MouseCursor::CORNER_BOTTOM_RIGHT] = LoadCursorA(NULL, IDC_SIZENESW);
		_win_cursors[MouseCursor::SIZE_HORIZONTAL]     = LoadCursorA(NULL, IDC_SIZEWE);
		_win_cursors[MouseCursor::SIZE_VERTICAL]       = LoadCursorA(NULL, IDC_SIZENS);
		_win_cursors[MouseCursor::WAIT]                = LoadCursorA(NULL, IDC_WAIT);

		Thread main_thread;
		main_thread.start([](void *user_data) {
				WindowsDevice *win = (WindowsDevice *)user_data;
				int ec = crown::main_runtime(*win->_options);
				s_exit = true;
				PostMessageA(win->_hwnd, WM_USER + 1, 0, 0);
				return ec;
			}
			, this
			);

		Thread joypad_thread;
		joypad_thread.start([](void *user_data) {
				WindowsDevice *win = (WindowsDevice *)user_data;
				while (!s_exit) {
					win->_joypad.update(win->_queue);
					os::sleep(4);
				}
				return EXIT_SUCCESS;
			}
			, this
			);

		// Windows event loop.
		MSG msg;
		msg.message = WM_NULL;
		while (GetMessage(&msg, NULL, 0U, 0U) != 0) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		joypad_thread.stop();
		main_thread.stop();

		// Destroy standard cursors
		DestroyIcon((HICON)_win_cursors[MouseCursor::WAIT]);
		DestroyIcon((HICON)_win_cursors[MouseCursor::SIZE_VERTICAL]);
		DestroyIcon((HICON)_win_cursors[MouseCursor::SIZE_HORIZONTAL]);
		DestroyIcon((HICON)_win_cursors[MouseCursor::CORNER_BOTTOM_RIGHT]);
		DestroyIcon((HICON)_win_cursors[MouseCursor::CORNER_BOTTOM_LEFT]);
		DestroyIcon((HICON)_win_cursors[MouseCursor::CORNER_TOP_RIGHT]);
		DestroyIcon((HICON)_win_cursors[MouseCursor::CORNER_TOP_LEFT]);
		DestroyIcon((HICON)_win_cursors[MouseCursor::TEXT_INPUT]);
		DestroyIcon((HICON)_win_cursors[MouseCursor::HAND]);
		DestroyIcon((HICON)_win_cursors[MouseCursor::ARROW]);

		DestroyWindow(_hwnd);

		return main_thread.exit_code();
	}

	LRESULT pump_events(HWND hwnd, UINT id, WPARAM wparam, LPARAM lparam)
	{
		switch (id) {
		case WM_DESTROY:
		case WM_USER + 1:
			PostQuitMessage(0);
			return 0;

		case WM_CLOSE:
			s_exit = true;
			_queue.push_exit_event();
			return 0;

		case WM_SIZE: {
			u32 width  = GET_X_LPARAM(lparam);
			u32 height = GET_Y_LPARAM(lparam);
			if (_cursor_mode == CursorMode::DISABLED) {
				RECT clipRect;
				GetWindowRect(_hwnd, &clipRect);
				ClipCursor(&clipRect);
			}
			_queue.push_resolution_event(width, height);
			break;
		}

		case WM_SYSCOMMAND:
			switch (wparam) {
			case SC_MINIMIZE:
			case SC_RESTORE: {
				HWND parent = GetWindow(hwnd, GW_OWNER);
				if (parent != NULL) {
					PostMessage(parent, id, wparam, lparam);
				}
				break;
			}
			}
			break;

		case WM_MOUSEWHEEL: {
			short delta = GET_WHEEL_DELTA_WPARAM(wparam);
			_queue.push_axis_event(InputDeviceType::MOUSE
				, 0
				, MouseAxis::WHEEL
				, 0
				, delta / WHEEL_DELTA
				, 0
				);
			break;
		}

		case WM_MOUSEMOVE: {
			s32 mx = GET_X_LPARAM(lparam);
			s32 my = GET_Y_LPARAM(lparam);
			s16 deltax = mx - (_mouse_last_x == INT16_MAX ? mx : _mouse_last_x);
			s16 deltay = my - (_mouse_last_y == INT16_MAX ? my : _mouse_last_y);
			if (_cursor_mode == CursorMode::DISABLED) {
				RECT clipRect;
				GetWindowRect(_hwnd, &clipRect);
				unsigned width = clipRect.right - clipRect.left;
				unsigned height = clipRect.bottom - clipRect.top;

				if (mx != (s32)width/2 || my != (s32)height/2) {
					_queue.push_axis_event(InputDeviceType::MOUSE
						, 0
						, MouseAxis::CURSOR_DELTA
						, deltax
						, deltay
						, 0
						);
					POINT mouse_pos = { (long)width/2, (long)height/2 };
					ClientToScreen(_hwnd, &mouse_pos);
					SetCursorPos(mouse_pos.x, mouse_pos.y);
					_mouse_last_x = (s16)width/2;
					_mouse_last_y = (s16)height/2;
				}
			} else if (_cursor_mode == CursorMode::NORMAL) {
				_queue.push_axis_event(InputDeviceType::MOUSE
					, 0
					, MouseAxis::CURSOR_DELTA
					, deltax
					, deltay
					, 0
					);
				_mouse_last_x = (s16)mx;
				_mouse_last_y = (s16)my;
			}
			_queue.push_axis_event(InputDeviceType::MOUSE
				, 0
				, MouseAxis::CURSOR
				, (s16)mx
				, (s16)my
				, 0
				);
			break;
		}

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP: {
			MouseButton::Enum mb;
			if (id == WM_LBUTTONDOWN || id == WM_LBUTTONUP)
				mb = MouseButton::LEFT;
			else if (id == WM_RBUTTONDOWN || id == WM_RBUTTONUP)
				mb = MouseButton::RIGHT;
			else /* if  (id == WM_MBUTTONDOWN || id == WM_MBUTTONUP) */
				mb = MouseButton::MIDDLE;

			bool down = id == WM_LBUTTONDOWN
				|| id == WM_RBUTTONDOWN
				|| id == WM_MBUTTONDOWN
				;

			_queue.push_button_event(InputDeviceType::MOUSE
				, 0
				, mb
				, down
				);
			break;
		}

		case WM_INPUT: {
			// See: https://blog.molecular-matters.com/2011/09/05/properly-handling-keyboard-input
			char data[sizeof(RAWINPUT)] = {};
			UINT size = sizeof(RAWINPUT);
			GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, data, &size, sizeof(RAWINPUTHEADER));

			// Extract keyboard raw input data.
			RAWINPUT *raw = reinterpret_cast<RAWINPUT *>(data);
			if (raw->header.dwType == RIM_TYPEKEYBOARD) {
				const RAWKEYBOARD &keyboard = raw->data.keyboard;
				UINT vk = keyboard.VKey;
				UINT sc = keyboard.MakeCode;
				UINT flags = keyboard.Flags;
				KeyboardButton::Enum kb = KeyboardButton::COUNT;

				if (vk == 255) {
					// Discard "fake keys" which are part of an escaped sequence.
					break;
				} else if (vk == VK_SHIFT) {
					// Correct left-hand / right-hand SHIFT.
					vk = MapVirtualKey(sc, MAPVK_VSC_TO_VK_EX);
				} else if (vk == VK_NUMLOCK) {
					// Correct PAUSE/BREAK and NUM LOCK silliness, and set the extended bit.
					sc = MapVirtualKey(vk, MAPVK_VK_TO_VSC) | 0x100;
				}

				// e0 and e1 are escape sequences used for certain special keys, such as PRINT and
				// PAUSE/BREAK. See http://www.win.tue.nl/~aeb/linux/kbd/scs-1.html
				const bool e0 = (flags & RI_KEY_E0) != 0;
				const bool e1 = (flags & RI_KEY_E1) != 0;

				if (e1) {
					// For escaped sequences, turn the virtual key into the correct scan code using
					// MapVirtualKey. however, MapVirtualKey is unable to map VK_PAUSE (this is a
					// known bug), hence we map that by hand.
					if (vk == VK_PAUSE)
						sc = 0x45;
					else
						sc = MapVirtualKey(vk, MAPVK_VK_TO_VSC);
				}

				switch (vk) {
				// Right-hand CONTROL and ALT have their e0 bit set.
				case VK_CONTROL:
					vk = e0 ? VK_RCONTROL : VK_LCONTROL;
					break;
				case VK_MENU:
					vk = e0 ? VK_RMENU : VK_LMENU;
					break;
				// NUMPAD ENTER has its e0 bit set.
				case VK_RETURN:
					if (e0)
						kb = KeyboardButton::NUMPAD_ENTER;
					break;
				// The standard INSERT, DELETE, HOME, END, PRIOR and NEXT keys will always have
				// their e0 bit set, but the corresponding keys on the NUMPAD will not.
				case VK_INSERT:
					if (!e0) vk = VK_NUMPAD0;
					break;
				case VK_DELETE:
					if (!e0)
						vk = VK_DECIMAL;
					break;
				case VK_HOME:
					if (!e0)
						vk = VK_NUMPAD7;
					break;
				case VK_END:
					if (!e0)
						vk = VK_NUMPAD1;
					break;
				case VK_PRIOR:
					if (!e0)
						vk = VK_NUMPAD9;
					break;
				case VK_NEXT:
					if (!e0)
						vk = VK_NUMPAD3;
					break;
				// The standard arrow keys will always have their e0 bit set, but the corresponding
				// keys on the NUMPAD will not.
				case VK_LEFT:
					if (!e0)
						vk = VK_NUMPAD4;
					break;
				case VK_RIGHT:
					if (!e0)
						vk = VK_NUMPAD6;
					break;
				case VK_UP:
					if (!e0)
						vk = VK_NUMPAD8;
					break;
				case VK_DOWN:
					if (!e0)
						vk = VK_NUMPAD2;
					break;
				// NUMPAD5 doesn't have its e0 bit set.
				case VK_CLEAR:
					if (!e0)
						vk = VK_NUMPAD5;
					break;
				}

				if (kb == KeyboardButton::COUNT)
					kb = win_translate_key(vk);

				if (kb != KeyboardButton::COUNT) {
					_queue.push_button_event(InputDeviceType::KEYBOARD
						, 0
						, kb
						, (flags & RI_KEY_BREAK) == 0
						);
				}
			}
			break;
		}

		case WM_CHAR: {
			uint8_t utf8[4] = { 0 };
			uint8_t len = (uint8_t)WideCharToMultiByte(CP_UTF8
				, 0
				, (LPCWSTR)&wparam
				, 1
				, (LPSTR)utf8
				, sizeof(utf8)
				, NULL
				, NULL
				);

			if (len)
				_queue.push_text_event(len, utf8);
			break;
		}

		case WM_SETCURSOR:
			if (LOWORD(lparam) == HTCLIENT) {
				if (_hcursor != NULL)
					SetCursor(_hcursor);
				else
					SetCursor(NULL);
				return TRUE;
			}

		default:
			break;
		}

		return DefWindowProc(hwnd, id, wparam, lparam);
	}

	static LRESULT CALLBACK window_proc(HWND hwnd, UINT id, WPARAM wparam, LPARAM lparam);
};

static WindowsDevice *s_windows_device;

LRESULT CALLBACK WindowsDevice::window_proc(HWND hwnd, UINT id, WPARAM wparam, LPARAM lparam)
{
	return s_windows_device->pump_events(hwnd, id, wparam, lparam);
}

struct WindowWin : public Window
{
	u16 _x;
	u16 _y;
	u16 _width;
	u16 _height;

	WindowWin()
		: _x(0)
		, _y(0)
		, _width(CROWN_DEFAULT_WINDOW_WIDTH)
		, _height(CROWN_DEFAULT_WINDOW_HEIGHT)
	{
	}

	void open(u16 x, u16 y, u16 width, u16 height, u32 parent) override
	{
		CE_UNUSED(parent);
		move(x, y);
		resize(width, height);
	}

	void close() override
	{
	}

	void show() override
	{
		ShowWindow(s_windows_device->_hwnd, SW_SHOW);

		if (s_windows_device->_options->_parent_window == 0)
			SetForegroundWindow(s_windows_device->_hwnd);
	}

	void hide() override
	{
		ShowWindow(s_windows_device->_hwnd, SW_HIDE);
	}

	void resize(u16 width, u16 height) override
	{
		_width = width;
		_height = height;

		DWORD style = GetWindowLongA(s_windows_device->_hwnd, GWL_STYLE);
		RECT rect;
		rect.left   = 0;
		rect.top    = 0;
		rect.right  = _width;
		rect.bottom = _height;
		AdjustWindowRect(&rect, style, FALSE);

		MoveWindow(s_windows_device->_hwnd
			, _x
			, _y
			, rect.right - rect.left
			, rect.bottom - rect.top
			, FALSE
			);
	}

	void move(u16 x, u16 y) override
	{
		_x = x;
		_y = y;
		resize(_width, _height);
	}

	void minimize() override
	{
		ShowWindow(s_windows_device->_hwnd, SW_MINIMIZE);
	}

	void maximize() override
	{
		ShowWindow(s_windows_device->_hwnd, SW_MAXIMIZE);
	}

	void restore() override
	{
		ShowWindow(s_windows_device->_hwnd, SW_RESTORE);
	}

	const char *title() override
	{
		static char buf[512];
		memset(buf, 0, sizeof(buf));
		GetWindowText(s_windows_device->_hwnd, buf, sizeof(buf));
		return buf;
	}

	void set_title(const char *title) override
	{
		SetWindowText(s_windows_device->_hwnd, title);
	}

	void show_cursor(bool show) override
	{
		s_windows_device->_hcursor = show ? LoadCursorA(NULL, IDC_ARROW) : NULL;
		SetCursor(s_windows_device->_hcursor);
	}

	void set_fullscreen(bool fullscreen) override
	{
		CE_UNUSED(fullscreen);
	}

	void set_cursor(MouseCursor::Enum cursor) override
	{
		s_windows_device->_hcursor = _win_cursors[cursor];
		SetCursor(s_windows_device->_hcursor);
	}

	bool set_cursor_mode(CursorMode::Enum mode) override
	{
		if (mode == s_windows_device->_cursor_mode)
			return true;
		s_windows_device->_cursor_mode = mode;

		if (mode == CursorMode::DISABLED) {
			RECT clipRect;
			GetWindowRect(s_windows_device->_hwnd, &clipRect);
			unsigned width = clipRect.right - clipRect.left;
			unsigned height = clipRect.bottom - clipRect.top;

			s_windows_device->_mouse_last_x = width/2;
			s_windows_device->_mouse_last_y = height/2;
			POINT mouse_pos = { (long)width/2, (long)height/2 };
			ClientToScreen(s_windows_device->_hwnd, &mouse_pos);
			SetCursorPos(mouse_pos.x, mouse_pos.y);

			show_cursor(false);
			ClipCursor(&clipRect);
		} else if (mode == CursorMode::NORMAL) {
			show_cursor(true);
			ClipCursor(NULL);
		}

		return true;
	}

	void *native_handle() override
	{
		return (void *)(uintptr_t)s_windows_device->_hwnd;
	}

	void *native_handle_type() override
	{
		return (void *)(uintptr_t)bgfx::NativeWindowHandleType::Default;
	}

	void *native_display() override
	{
		return NULL;
	}
};

namespace window
{
	Window *create(Allocator &a)
	{
		return CE_NEW(a, WindowWin)();
	}

	void destroy(Allocator &a, Window &w)
	{
		CE_DELETE(a, &w);
	}

} // namespace window

struct DisplayWin : public Display
{
	void modes(Array<DisplayMode> &modes) override
	{
		CE_UNUSED(modes);
	}

	void set_mode(u32 id) override
	{
		CE_UNUSED(id);
	}
};

namespace display
{
	Display *create(Allocator &a)
	{
		return CE_NEW(a, DisplayWin)();
	}

	void destroy(Allocator &a, Display &d)
	{
		CE_DELETE(a, &d);
	}

} // namespace display

static bool push_event(const OsEvent &ev)
{
	return s_windows_device->_events.push(ev);
}

bool next_event(OsEvent &ev)
{
	return s_windows_device->_events.pop(ev);
}

struct InitGlobals
{
	InitGlobals()
	{
		memory_globals::init();
		profiler_globals::init();
		guid_globals::init();
	}

	~InitGlobals()
	{
		guid_globals::shutdown();
		profiler_globals::shutdown();
		memory_globals::shutdown();
	}
};

} // namespace crown

int main(int argc, char **argv)
{
	using namespace crown;

	if (AttachConsole(ATTACH_PARENT_PROCESS) != 0) {
		const DWORD handles[] = { STD_OUTPUT_HANDLE, STD_ERROR_HANDLE, STD_INPUT_HANDLE };
		const char *modes[] = { "w", "w", "r" };
		FILE *stdfds[] = { stdout, stderr, stdin };

		for (u32 i = 0; i < countof(handles); ++i) {
			HANDLE out = GetStdHandle(handles[i]);
			int fd = _open_osfhandle((intptr_t)out, _O_TEXT);
			if (fd != -1) {
				*stdfds[i] = *_fdopen(fd, modes[i]);
				setvbuf(stdfds[i], NULL, _IONBF, 0); // No buffering.
			}
		}
	}

	// code-format off
	PHANDLER_ROUTINE signal_handler = [](DWORD dwCtrlType) {
			switch (dwCtrlType) {
			case CTRL_C_EVENT:
				if (device())
					device()->quit();
				return TRUE;

			default:
				return FALSE;
			}
		};
	// code-format on
	SetConsoleCtrlHandler(signal_handler, TRUE);

	WSADATA wsdata;
	int err = WSAStartup(MAKEWORD(2, 2), &wsdata);
	CE_ASSERT(err == 0, "WSAStartup: error = %d", err);
	CE_UNUSED(wsdata);
	CE_UNUSED(err);

#if CROWN_BUILD_UNIT_TESTS
	CommandLine cl(argc, (const char **)argv);
	if (cl.has_option("run-unit-tests")) {
		return main_unit_tests();
	}
#endif

	InitGlobals m;
	CE_UNUSED(m);

	DeviceOptions opts(default_allocator(), argc, (const char **)argv);
	bool quit = false;
	int ec = opts.parse(&quit);

	if (quit)
		return ec;

#if CROWN_CAN_COMPILE
	if (ec == EXIT_SUCCESS && (opts._do_compile || opts._server)) {
		ec = main_data_compiler(opts);
		if (!opts._do_continue)
			return ec;
	}
#endif

	if (ec == EXIT_SUCCESS) {
		s_windows_device = CE_NEW(default_allocator(), WindowsDevice)(default_allocator(), opts);
		ec = s_windows_device->run();
		CE_DELETE(default_allocator(), s_windows_device);
	}

	FreeConsole();
	WSACleanup();
	SetConsoleCtrlHandler(signal_handler, FALSE);
	return ec;
}

#endif // if CROWN_PLATFORM_WINDOWS
