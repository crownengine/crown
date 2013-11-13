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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windowsx.h>

#define WM_USER_SET_WINDOW_SIZE     (WM_USER+0)
#define WM_USER_TOGGLE_WINDOW_FRAME (WM_USER+1)
#define WM_USER_MOUSE_LOCK          (WM_USER+2)

#include "Crown.h"
#include "OsTypes.h"
#include "OsEventQueue.h"
#include "GLContext.h"
#include "BundleCompiler.h"

#include "OS.h"

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

//-----------------------------------------------------------------------------
class CE_EXPORT WindowsDevice : public Device
{
public:

	WindowsDevice() 
		: m_hwnd(0)
		, m_rect()
		, m_style(0)
		, m_width(0)
		, m_height(0)
		, m_old_width(0)
		, m_old_height(0)
		, m_frame_width(0)
		, m_frame_height(0)
		, m_aspect_ratio(0.0f)
		, m_x(0)
		, m_y(0)
		, m_frame(true)
		, m_mouse_lock(false)
		, m_started(false)
		, m_exit(false)
	{

	}

	//-----------------------------------------------------------------------------
	void init(int argc, char** argv)
	{
		parse_command_line(argc, argv);
		check_preferred_settings();

		#if defined(CROWN_DEBUG) || defined(CROWN_DEVELOPMENT)
			if (m_compile == 1)
			{
				m_bundle_compiler = CE_NEW(default_allocator(), BundleCompiler);
				if (!m_bundle_compiler->compile(m_bundle_dir, m_source_dir))
				{
					CE_DELETE(default_allocator(), m_bundle_compiler);
					Log::e("Exiting.");
					exit(EXIT_FAILURE);
				}

				if (!m_continue)
				{
					CE_DELETE(default_allocator(), m_bundle_compiler);
					exit(EXIT_SUCCESS);
				}
			}
		#endif

		read_configuration();
	}

	//-----------------------------------------------------------------------------
	void shutdown()
	{
		#if defined(CROWN_DEBUG) || defined(CROWN_DEVELOPMENT)
			CE_DELETE(default_allocator(), m_bundle_compiler);
		#endif
	}

	//-----------------------------------------------------------------------------
	int32_t	run(int argc, char** argv)
	{
		init(argc, argv);

		WSADATA WsaData;
		CE_ASSERT(WSAStartup(MAKEWORD(2,2), &WsaData) == 0L, "Unable to initialize socket");

		HINSTANCE instance = (HINSTANCE)GetModuleHandle(NULL);

		WNDCLASSEX wnd;
		memset(&wnd, 0, sizeof(wnd));
		wnd.cbSize = sizeof(wnd);
		wnd.lpfnWndProc = DefWindowProc;
		wnd.hInstance = instance;
		wnd.hIcon = LoadIcon(instance, IDI_APPLICATION);
		wnd.hCursor = LoadCursor(instance, IDC_ARROW);
		wnd.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wnd.lpszClassName = "crown_letterbox";
		wnd.hIconSm = LoadIcon(instance, IDI_APPLICATION);
		RegisterClassExA(&wnd);

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

		HWND hwnd = CreateWindowA(
						"crown_letterbox", 
						"CrownUnknown",
						WS_POPUP|WS_SYSMENU,
						-32000,
						-32000,
						0,
						0,
						NULL,
						NULL,
						instance,
						0
					);

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
		glcontext_set_window(m_hwnd);

		adjust(ENTRY_DEFAULT_WIDTH, ENTRY_DEFAULT_HEIGHT, true);
		m_width = ENTRY_DEFAULT_WIDTH;
		m_height = ENTRY_DEFAULT_HEIGHT;
		m_old_width = ENTRY_DEFAULT_WIDTH;
		m_old_height = ENTRY_DEFAULT_HEIGHT;

		m_argc = argc;
		m_argv = argv;

		OsThread thread("game-loop");
		thread.start(WindowsDevice::main_loop, this);
		m_started = true;

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

		shutdown();

		DestroyWindow(m_hwnd);
		DestroyWindow(hwnd);

		WSACleanup();

		return 0;
	}

	//-----------------------------------------------------------------------------
	int32_t loop()
	{
		Device::init();

		while(!process_events() && is_running())
		{
			Device::frame();

			m_keyboard->update();
			m_mouse->update();
		}

		Device::shutdown();

		m_exit = true;

		return 0;
	}

	//-----------------------------------------------------------------------------
	void adjust(uint32_t width, uint32_t height, bool window_frame)
	{
		m_width = width;
		m_height = height;
		m_aspect_ratio = float(width) / float(height);

		ShowWindow(m_hwnd, SW_SHOWNORMAL);
		RECT rect;
		RECT newrect = {0, 0, (LONG)width, (LONG)height};
		DWORD style = WS_POPUP|WS_SYSMENU;

		if (m_frame)
		{
			GetWindowRect(m_hwnd, &m_rect);
			m_style = GetWindowLong(m_hwnd, GWL_STYLE);
		}

		if (window_frame)
		{
			rect = m_rect;
			style = m_style;
		}
		else
		{
			HMONITOR monitor = MonitorFromWindow(m_hwnd, MONITOR_DEFAULTTONEAREST);
			MONITORINFO mi;
			mi.cbSize = sizeof(mi);
			GetMonitorInfo(monitor, &mi);
			newrect = mi.rcMonitor;
			rect = mi.rcMonitor;
		}

		SetWindowLong(m_hwnd, GWL_STYLE, style);
		uint32_t prewidth = newrect.right - newrect.left;
		uint32_t preheight = newrect.bottom - newrect.top;
		AdjustWindowRect(&newrect, style, FALSE);
		m_frame_width = (newrect.right - newrect.left) - prewidth;
		m_frame_height = (newrect.bottom - newrect.top) - preheight;
		UpdateWindow(m_hwnd);

		if (rect.left < 0 || rect.top < 0)
		{
			rect.left = 0;
			rect.top = 0;
		}

		int32_t left_t = rect.left;
		int32_t top_t = rect.top;
		int32_t width_t = (newrect.right-newrect.left);
		int32_t height_t = (newrect.bottom-newrect.top);

		if (!window_frame)
		{
			float aspect_ratio = 1.0f / m_aspect_ratio;
			width_t = math::max(uint32_t(ENTRY_DEFAULT_WIDTH / 4), width);
			height_t = uint32_t(float(width) * aspect_ratio);

			left_t = newrect.left+(newrect.right-newrect.left-width) / 2;
			top_t = newrect.top+(newrect.bottom-newrect.top-height) / 2;
		}

		HWND parent = GetWindow(m_hwnd, GW_OWNER);
		if (NULL != parent)
		{
			if (window_frame)
			{
				SetWindowPos(parent, HWND_TOP, -32000, -32000, 0, 0, SWP_SHOWWINDOW);
			}
			else
			{
				SetWindowPos(parent, HWND_TOP, newrect.left, newrect.top, newrect.right-newrect.left, newrect.bottom-newrect.top, SWP_SHOWWINDOW);
			}
		}

		SetWindowPos(m_hwnd, HWND_TOP, left_t, top_t, width_t, height_t, SWP_SHOWWINDOW);

		ShowWindow(m_hwnd, SW_RESTORE);

		m_frame = window_frame;
	}

	//-----------------------------------------------------------------------------
	void set_mouse_pos(int32_t x, int32_t y)
	{
		POINT pt = {x, y};
		ClientToScreen(m_hwnd, &pt);
		SetCursorPos(pt.x, pt.y);
	}

	//-----------------------------------------------------------------------------
	void set_mouse_lock(bool lock)
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
	
	//-----------------------------------------------------------------------------
	static int32_t main_loop(void* data)
	{
		return ((WindowsDevice*)data)->loop();
	}

	//-----------------------------------------------------------------------------
	bool process_events()
	{
		OsEvent event;
		do
		{
			m_queue.pop_event(&event);

			if (event.type != OsEvent::NONE)
			{
				switch (event.type)
				{
					case OsEvent::MOUSE:
					{
						const OsMouseEvent& ev = event.mouse;
						switch (ev.type)
						{
							case OsMouseEvent::BUTTON: m_mouse->set_button_state(ev.x, ev.y, ev.button, ev.pressed); break;
							case OsMouseEvent::MOVE: m_mouse->set_position(ev.x, ev.y); break;
							default: CE_FATAL("Oops, unknown mouse event type"); break;
						}

						break;
					}
					case OsEvent::KEYBOARD:
					{
						const OsKeyboardEvent& ev = event.keyboard;
						m_keyboard->set_button_state(ev.button, ev.pressed);
						break;
					}
					case OsEvent::METRICS:
					{
						const OsMetricsEvent& ev = event.metrics;
						m_mouse->set_metrics(ev.width, ev.height);
						break;
					}
					case OsEvent::EXIT:
					{
						return true;
					}
					default:
					{
						Log::d("Unmanaged");
						break;
					}
				}
			}
		}
		while (event.type != OsEvent::NONE);

		return false;
	}

	//-----------------------------------------------------------------------------
	LRESULT bump_events(HWND hwnd, UINT id, WPARAM wparam, LPARAM lparam)
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

				m_queue.push_metrics_event(m_x, m_y, m_width, m_height);

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

				m_queue.push_metrics_event(m_x, m_y, m_width, m_height);

				break;
			}
			case WM_USER_MOUSE_LOCK:
			{
				set_mouse_lock(!!lparam);
				break;
			}
			case WM_DESTROY:
			{
				break;
			}
			case WM_QUIT:
			case WM_CLOSE:
			{
				m_exit = true;

				OsExitEvent event;
				event.code = 0;
				m_queue.push_exit_event(0);

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

				return 0;
			}
			case WM_SIZE:
			{
				uint32_t width = GET_X_LPARAM(lparam);
				uint32_t height = GET_Y_LPARAM(lparam);

				m_width = width;
				m_height = height;

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
				KeyboardButton::Enum kb = win_translate_key(wparam);

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

				m_queue.push_keyboard_event(modifier_mask, kb, id == WM_KEYDOWN || id == WM_SYSKEYDOWN);
		
				break;
			}
			default:
				break;
			}
		}

		return DefWindowProc(hwnd, id, wparam, lparam);
	}

	//-----------------------------------------------------------------------------
	void parse_command_line(int argc, char** argv)
	{
		static const char* help_message =
			"Usage: crown [options]\n"
			"Options:\n\n"

			"All of the following options take precedence over\n"
			"environment variables and configuration files.\n\n"

			"  --help                     Show this help.\n"
			"  --bundle-dir <path>        Use <path> as the source directory for compiled resources.\n"
			"  --width <width>            Set the <width> of the main window.\n"
			"  --height <width>           Set the <height> of the main window.\n"
			"  --fullscreen               Start in fullscreen.\n"
			"  --parent-window <handle>   Set the parent window <handle> of the main window.\n"
			"                             Used only by tools.\n"

			"\nAvailable only in debug and development builds:\n\n"

			"  --source-dir <path>        Use <path> as the source directory for resource compilation.\n"
			"  --compile                  Run the engine as resource compiler.\n"
			"  --continue                 Do a full compile of the resources and continue the execution.\n";

		static ArgsOption options[] = 
		{
			{ "help",             AOA_NO_ARGUMENT,       NULL,        'i' },
			{ "source-dir",       AOA_REQUIRED_ARGUMENT, NULL,        's' },
			{ "bundle-dir",       AOA_REQUIRED_ARGUMENT, NULL,        'b' },
			{ "compile",          AOA_NO_ARGUMENT,       &m_compile,   1 },
			{ "continue",         AOA_NO_ARGUMENT,       &m_continue,  1 },
			{ "width",            AOA_REQUIRED_ARGUMENT, NULL,        'w' },
			{ "height",           AOA_REQUIRED_ARGUMENT, NULL,        'h' },
			{ "fullscreen",       AOA_NO_ARGUMENT,       &m_fullscreen, 1 },
			{ "parent-window",    AOA_REQUIRED_ARGUMENT, NULL,        'p' },
			{ NULL, 0, NULL, 0 }
		};

		Args args(argc, argv, "", options);

		int32_t opt;
		while ((opt = args.getopt()) != -1)
		{
			switch (opt)
			{
				case 0:
				{
					break;
				}
				// Source directory
				case 's':
				{
					string::strncpy(m_source_dir, args.optarg(), MAX_PATH_LENGTH);
					break;
				}
				// Bundle directory
				case 'b':
				{
					string::strncpy(m_bundle_dir, args.optarg(), MAX_PATH_LENGTH);
					break;
				}
				// Window width
				case 'w':
				{
					m_width = atoi(args.optarg());
					break;
				}
				// Window height
				case 'h':
				{
					m_height = atoi(args.optarg());
					break;
				}
				// Parent window
				case 'p':
				{
					m_parent_window_handle = string::parse_uint(args.optarg());
					break;
				}
				case 'i':
				case '?':
				default:
				{
					os::printf(help_message);
					exit(EXIT_FAILURE);
				}
			}
		}
	}

	//-----------------------------------------------------------------------------
	void check_preferred_settings()
	{
		if (m_compile == 1)
		{
			if (string::strcmp(m_source_dir, "") == 0)
			{
				Log::e("You have to specify the source directory when running in compile mode.");
				exit(EXIT_FAILURE);
			}

			if (!os::is_absolute_path(m_source_dir))
			{
				Log::e("The source directory must be absolute.");
				exit(EXIT_FAILURE);
			}
		}

		if (!os::is_absolute_path(m_bundle_dir))
		{
			Log::e("The bundle directory must be absolute.");
			exit(EXIT_FAILURE);
		}

		// if (m_width == 0 || m_height == 0)
		// {
		// 	Log::e("Window width and height must be greater than zero.");
		// 	exit(EXIT_FAILURE);
		// }
	}

	//-------------------------------------------------------------------------
	void read_configuration()
	{
		DiskFilesystem fs(m_bundle_dir);

		// crown.config is mandatory
		CE_ASSERT(fs.is_file("crown.config"), "Unable to open crown.config");

		File* config_file = fs.open("crown.config", FOM_READ);

		TempAllocator4096 alloc;
		char* json_string = (char*)alloc.allocate(config_file->size());
		config_file->read(json_string, config_file->size());
		fs.close(config_file);

		// Parse crown.config
		JSONParser parser(json_string);
		JSONElement root = parser.root();

		// Boot
		if (root.has_key("boot"))
		{
			const char* boot = root.key("boot").string_value();
			const size_t boot_length = string::strlen(boot);

			string::strncpy(m_boot_file, boot, (boot_length > MAX_PATH_LENGTH) ? MAX_PATH_LENGTH : boot_length + 1);
		}

		// Window width
		if (root.has_key("window_width"))
		{
			m_width = root.key("window_width").int_value();
		}

		// Window height
		if (root.has_key("window_height"))
		{
			m_height = root.key("window_height").int_value();
		}
	}

private:

	static LRESULT CALLBACK window_proc(HWND hwnd, UINT id, WPARAM wparam, LPARAM lparam);

public:

	HWND m_hwnd;
	HDC m_hdc;
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

	uint32_t m_parent_window_handle;
	int32_t m_fullscreen;
	int32_t m_compile;
	int32_t m_continue;	

	OsEventQueue m_queue;
};

WindowsDevice* engine;

LRESULT CALLBACK WindowsDevice::window_proc(HWND hwnd, UINT id, WPARAM wparam, LPARAM lparam)
{
	return ((WindowsDevice*)engine)->bump_events(hwnd, id, wparam, lparam);
}

} // namespace crown


int main(int argc, char** argv)
{
	using namespace crown;
	init();

	engine = CE_NEW(default_allocator(), WindowsDevice)();
	set_device(engine);
	
	int32_t ret = ((WindowsDevice*)engine)->run(argc, argv);

	CE_DELETE(default_allocator(), engine);

	shutdown();

	return ret;
}