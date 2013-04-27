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

#include "Config.h"
#include "Device.h"
#include "Filesystem.h"
#include "InputManager.h"
#include "Log.h"
#include "OS.h"
#include "Renderer.h"
#include "Types.h"
#include "String.h"
#include "Args.h"
#include "Game.h"
#include <cstdlib>

#include "renderers/gl/GLRenderer.h"
//#include "renderers/gles/GLESRenderer.h"

namespace crown
{

//-----------------------------------------------------------------------------
Device::Device() :
	m_preferred_window_width(1000),
	m_preferred_window_height(625),
	m_preferred_window_fullscreen(false),

	m_is_init(false),
	m_is_running(false),

	m_filesystem(NULL),
	m_input_manager(NULL),
	m_renderer(NULL)
{
	string::strcpy(m_preferred_root_path, string::EMPTY);
	string::strcpy(m_preferred_user_path, string::EMPTY);
}

//-----------------------------------------------------------------------------
Device::~Device()
{
}

//-----------------------------------------------------------------------------
bool Device::init(int argc, char** argv)
{
	if (parse_command_line(argc, argv) == false)
	{
		return false;
	}

	if (is_init())
	{
		Log::E("Crown Engine is already initialized.");
		return false;
	}

	// Initialize
	Log::I("Initializing Crown Engine %d.%d.%d...", CROWN_VERSION_MAJOR, CROWN_VERSION_MINOR, CROWN_VERSION_MICRO);

	// Set the root path
	m_filesystem = new Filesystem(m_preferred_root_path);

	m_input_manager = new InputManager();

	// Create the renderer
	// FIXME FIXME FIXME
	// #ifdef CROWN_BUILD_OPENGL
	 	m_renderer = new GLRenderer();
		Log::I("Using GLRenderer.");
	// #elif defined CROWN_BUILD_OPENGLES
	//	m_renderer = new GLESRenderer();
	// #endif

	m_is_init = true;

	start();

	Log::I("Crown Engine initialized.");

	Log::I("Initializing Game...");

	const char* game_library_path = m_filesystem->build_os_path(m_filesystem->root_path(), "libgame.so");
	m_game_library = os::open_library(game_library_path);

	if (m_game_library == NULL)
	{
		Log::E("Error while loading game library.");
		return false;
	}

	create_game_t* create_game = (create_game_t*)os::lookup_symbol(m_game_library, "create_game");

	m_game = create_game();

	m_game->init();

	return true;
}

//-----------------------------------------------------------------------------
void Device::shutdown()
{
	if (is_init() == false)
	{
		Log::E("Crown Engine is not initialized.");	
		return;
	}

	m_game->shutdown();

	destroy_game_t* destroy_game = (destroy_game_t*)os::lookup_symbol(m_game_library, "destroy_game");

	destroy_game(m_game);
	m_game = NULL;

	if (m_input_manager)
	{
		delete m_input_manager;
	}

	Log::I("Releasing Renderer...");

	if (m_renderer)
	{
		delete m_renderer;
	}

	Log::I("Releasing Filesystem...");

	if (m_filesystem)
	{
		delete m_filesystem;
	}

	m_is_init = false;
}

//-----------------------------------------------------------------------------
bool Device::is_init() const
{
	return m_is_init;
}

//-----------------------------------------------------------------------------
Filesystem* Device::filesystem()
{
	return m_filesystem;
}

//-----------------------------------------------------------------------------
InputManager* Device::input_manager()
{
	return m_input_manager;
}

//-----------------------------------------------------------------------------
Renderer* Device::renderer()
{
	return m_renderer;
}

//-----------------------------------------------------------------------------
void Device::start()
{
	if (is_init() == false)
	{
		Log::E("Cannot start uninitialized engine.");
		return;
	}

	m_is_running = true;
}

//-----------------------------------------------------------------------------
void Device::stop()
{
	if (is_init() == false)
	{
		Log::E("Cannot stop uninitialized engine.");
		return;
	}

	m_is_running = false;
}

//-----------------------------------------------------------------------------
bool Device::is_running() const
{
	return m_is_running;
}

//-----------------------------------------------------------------------------
void Device::frame()
{
	m_input_manager->event_loop();

	m_renderer->begin_frame();

	m_game->update();

	m_renderer->end_frame();
}

//-----------------------------------------------------------------------------
bool Device::parse_command_line(int argc, char** argv)
{
	int32_t fullscreen = 0;

	ArgsOption options[] = 
	{
		"help",       AOA_NO_ARGUMENT,       NULL,        'i',
		"root-path",  AOA_REQUIRED_ARGUMENT, NULL,        'r',
		"user-path",  AOA_REQUIRED_ARGUMENT, NULL,        'u',
		"width",      AOA_REQUIRED_ARGUMENT, NULL,        'w',
		"height",     AOA_REQUIRED_ARGUMENT, NULL,        'h',
		"fullscreen", AOA_NO_ARGUMENT,       &fullscreen,  1,
		NULL, 0, NULL, 0
	};

	Args args(argc, argv, "", options);

	while (1)
	{
		int32_t ret = args.next_option();

		switch (ret)
		{
			case -1:
			{
				return true;
			}
			case 0:
			{
				m_preferred_window_fullscreen = fullscreen;

				break;
			}
			// Help
			case 'i':
			{
				print_help_message();
				return false;
			}
			// Root path
			case 'r':
			{
				if (args.option_argument() == NULL)
				{
					os::printf("%s: error: missing absolute path after `-root-path`\n", argv[0]);
					return false;
				}

				string::strcpy(m_preferred_root_path, args.option_argument());

				break;
			}
			// User path
			case 'u':
			{
				if (args.option_argument() == NULL)
				{
					os::printf("%s: error: missing absolute path after `--user-path`\n", argv[0]);
					return false;
				}

				string::strcpy(m_preferred_user_path, args.option_argument());

				break;
			}
			// Window width
			case 'w':
			{
				if (args.option_argument() == NULL)
				{
					os::printf("%s: error: missing width value after `--width`\n", argv[0]);
					return false;
				}

				m_preferred_window_width = atoi(args.option_argument());
				break;
			}
			// Window height
			case 'h':
			{
				if (args.option_argument() == NULL)
				{
					os::printf("%s: error: missing height value after `--height`\n", argv[0]);
					return false;
				}

				m_preferred_window_height = atoi(args.option_argument());
				break;
			}
			default:
			{
				break;
			}
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
void Device::print_help_message()
{
	os::printf("Usage: crown [options]\n");
	os::printf("Options:\n\n");

	os::printf("All of the following options take precedence over\n");
	os::printf("environment variables and configuration files.\n\n");

	os::printf("  --help                Show this help.\n");
	os::printf("  --root-path <path>    Use <path> as the filesystem root path.\n");
	os::printf("  --user-path <path>    Use <path> as the filesystem user path.\n");
	os::printf("  --width <width>       Set the <width> of the render window.\n");
	os::printf("  --height <width>      Set the <height> of the render window.\n");
	os::printf("  --fullscreen          Start in fullscreen.\n");
}

Device g_device;
Device* device()
{
	return &g_device;
}

} // namespace crown

