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

#include "os_types.h"
#include "os_event_queue.h"
#include "bundle_compiler.h"
#include "device.h"
#include "thread.h"
#include "log.h"
#include "os_window.h"
#include "args.h"
#include "json_parser.h"
#include "os.h"
#include <bgfxplatform.h>

#define ENTRY_DEFAULT_WIDTH 1000
#define ENTRY_DEFAULT_HEIGHT 625

namespace crown
{

extern void set_win_handle_window(HWND hwnd);

//-----------------------------------------------------------------------------
void init()
{
	memory::init();
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
class WindowsDevice : public Device
{
public:

	WindowsDevice()
		: m_hwnd(0)
		, m_width(0)
		, m_height(0)
		, m_x(0)
		, m_y(0)
		, m_exit(false)
		, m_wait_console(0)
		, m_platform("windows")
	{
	}

	//-----------------------------------------------------------------------------
	void init(int argc, char** argv)
	{
		parse_command_line(argc, argv);
		check_preferred_settings();

		#if defined(CROWN_DEBUG)
			m_console = CE_NEW(default_allocator(), ConsoleServer)();
			m_console->init(m_console_port, m_wait_console == 1);

			if (m_compile == 1)
			{
				m_bundle_compiler = CE_NEW(default_allocator(), BundleCompiler);
				if (!m_bundle_compiler->compile(m_bundle_dir, m_source_dir, m_platform))
				{
					CE_DELETE(default_allocator(), m_bundle_compiler);
					CE_LOGE("Exiting.");
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
		#if defined(CROWN_DEBUG)
			CE_DELETE(default_allocator(), m_bundle_compiler);

			m_console->shutdown();
			CE_DELETE(default_allocator(), m_console);
		#endif
	}

	//-----------------------------------------------------------------------------
	void display_modes(Array<DisplayMode>& /*modes*/)
	{
		// #error "Implement me"
	}

	//-----------------------------------------------------------------------------
	void set_display_mode(uint32_t /*id*/)
	{
		// #error "Implement me"
	}

	//-----------------------------------------------------------------------------
	void set_fullscreen(bool full)
	{
		// #error "Implement me"
	}

	//-----------------------------------------------------------------------------
	int32_t	run(int argc, char** argv)
	{
		WSADATA WsaData;
		int res = WSAStartup(MAKEWORD(2,2), &WsaData);
		CE_ASSERT(res == 0, "Unable to initialize socket");
		CE_UNUSED(WsaData);
		CE_UNUSED(res);

		init(argc, argv);

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

		m_width = ENTRY_DEFAULT_WIDTH;
		m_height = ENTRY_DEFAULT_HEIGHT;

		m_argc = argc;
		m_argv = argv;

		Thread thread;
		thread.start(WindowsDevice::main_loop, this);

		MSG msg;
		msg.message = WM_NULL;

		while (!m_exit)
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
		WSACleanup();
		return 0;
	}

	//-----------------------------------------------------------------------------
	int32_t loop()
	{
		Device::init();

		while(!process_events() && is_running())
		{
			#if defined(CROWN_DEBUG)
				m_console->update();
			#endif

			Device::frame();

			m_keyboard->update();
			m_mouse->update();
		}

		Device::shutdown();

		m_exit = true;

		return 0;
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
		bool exit = false;

		while(m_queue.pop_event(event))
		{
			if (event.type == OsEvent::NONE) continue;

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
					m_window->m_x = ev.x;
					m_window->m_y = ev.y;
					m_window->m_width = ev.width;
					m_window->m_height = ev.height;
					break;
				}
				case OsEvent::EXIT:
				{
					exit = true;
					break;
				}
				default:
				{
					CE_FATAL("Unknown Os Event");
					break;
				}
			}
		}

		return exit;
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
			"  --compile                  Do a full compile of the resources.\n"
			"  --platform <platform>      Compile resources for the given <platform>.\n"
			"      Possible values for <platform> are:\n"
			"          linux\n"
			"          android\n"
			"          windows\n"
			"  --continue                 Continue the execution after the resource compilation step.\n"
			"  --file-server              Read resources from a remote engine instance.\n"
			"  --console-port             Set the network port of the console server.\n"
			"  --wait-console             Wait for a console connection before starting up.\n";

		static ArgsOption options[] =
		{
			{ "help",             AOA_NO_ARGUMENT,       NULL,           'i' },
			{ "source-dir",       AOA_REQUIRED_ARGUMENT, NULL,           's' },
			{ "bundle-dir",       AOA_REQUIRED_ARGUMENT, NULL,           'b' },
			{ "compile",          AOA_NO_ARGUMENT,       &m_compile,       1 },
			{ "platform",         AOA_REQUIRED_ARGUMENT, NULL,           'r' },
			{ "continue",         AOA_NO_ARGUMENT,       &m_continue,      1 },
			{ "width",            AOA_REQUIRED_ARGUMENT, NULL,           'w' },
			{ "height",           AOA_REQUIRED_ARGUMENT, NULL,           'h' },
			{ "fullscreen",       AOA_NO_ARGUMENT,       &m_fullscreen,    1 },
			{ "parent-window",    AOA_REQUIRED_ARGUMENT, NULL,           'p' },
			{ "file-server",      AOA_NO_ARGUMENT,       &m_fileserver,    1 },
			{ "console-port",     AOA_REQUIRED_ARGUMENT, NULL,           'c' },
			{ "wait-console",     AOA_NO_ARGUMENT,       &m_wait_console,  1 },
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
				// Console port
				case 'c':
				{
					m_console_port = string::parse_uint(args.optarg());
					break;
				}
				// Platform
				case 'r':
				{
					m_platform = args.optarg();
					break;
				}
				case 'i':
				case '?':
				default:
				{
					printf(help_message);
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
				CE_LOGE("You have to specify the source directory when running in compile mode.");
				exit(EXIT_FAILURE);
			}

			if (!os::is_absolute_path(m_source_dir))
			{
				CE_LOGE("The source directory must be absolute.");
				exit(EXIT_FAILURE);
			}
		}

		if (!os::is_absolute_path(m_bundle_dir))
		{
			CE_LOGE("The bundle directory must be absolute.");
			exit(EXIT_FAILURE);
		}
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
			DynamicString boot;
			root.key("boot").to_string(boot);

			string::strncpy(m_boot_file, boot.c_str(), (boot.length() > MAX_PATH_LENGTH) ? MAX_PATH_LENGTH : boot.length() + 1);
		}

		// Window width
		if (root.has_key("window_width"))
		{
			m_width = root.key("window_width").to_int();
		}

		// Window height
		if (root.has_key("window_height"))
		{
			m_height = root.key("window_height").to_int();
		}
	}

private:

	static LRESULT CALLBACK window_proc(HWND hwnd, UINT id, WPARAM wparam, LPARAM lparam);

public:

	HWND m_hwnd;
	HDC m_hdc;
	uint32_t m_width;
	uint32_t m_height;
	int32_t m_x;
	int32_t m_y;
	bool m_exit;

	uint32_t m_parent_window_handle;
	int32_t m_fullscreen;
	int32_t m_compile;
	int32_t m_continue;
	int32_t m_wait_console;
	const char* m_platform;

	OsEventQueue m_queue;
};

WindowsDevice* engine;

LRESULT CALLBACK WindowsDevice::window_proc(HWND hwnd, UINT id, WPARAM wparam, LPARAM lparam)
{
	return ((WindowsDevice*)engine)->pump_events(hwnd, id, wparam, lparam);
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
