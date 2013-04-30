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
#include "DebugRenderer.h"
#include "Types.h"
#include "String.h"
#include "Args.h"
#include "Game.h"
#include <cstdlib>
#include "ArchiveResourceArchive.h"
#include "FileResourceArchive.h"
#include "ResourceManager.h"

#ifdef CROWN_BUILD_OPENGL
	#include "renderers/gl/GLRenderer.h"
#endif

#ifdef CROWN_BUILD_OPENGLES
	#include "renderers/gles/GLESRenderer.h"
#endif

namespace crown
{

static const char* GAME_LIBRARY_NAME = "libgame.so";

//-----------------------------------------------------------------------------
Device::Device() :
	m_preferred_window_width(1000),
	m_preferred_window_height(625),
	m_preferred_window_fullscreen(0),
	m_preferred_renderer(RENDERER_GL),
	m_preferred_mode(MODE_RELEASE),

	m_is_init(false),
	m_is_running(false),

	m_filesystem(NULL),
	m_resource_manager(NULL),
	m_input_manager(NULL),
	m_renderer(NULL),
	m_debug_renderer(NULL),

	m_resource_archive(NULL),

	m_game(NULL),
	m_game_library(NULL)
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
	if (is_init())
	{
		Log::E("Crown Engine is already initialized.");
		return false;
	}

	if (parse_command_line(argc, argv) == false)
	{
		return false;
	}

	// Initialize
	Log::I("Initializing Crown Engine %d.%d.%d...", CROWN_VERSION_MAJOR, CROWN_VERSION_MINOR, CROWN_VERSION_MICRO);

	// Select current dir if no root path provided
	if (string::strcmp(m_preferred_root_path, string::EMPTY) == 0)
	{
		m_filesystem = new Filesystem(os::get_cwd());
	}
	else
	{
		m_filesystem = new Filesystem(m_preferred_root_path);
	}

	// Select appropriate resource archive
	if (m_preferred_mode == MODE_DEVELOPMENT)
	{
		m_resource_archive = new FileResourceArchive(*m_filesystem);
	}
	else
	{
		m_resource_archive = new ArchiveResourceArchive(*m_filesystem);
	}

	// Create resource manager
	m_resource_manager = new ResourceManager(*m_resource_archive, m_resource_allocator);

	// Create input manager
	m_input_manager = new InputManager();

	// Select appropriate renderer
	if (m_preferred_renderer == RENDERER_GL)
	{
		#ifdef CROWN_BUILD_OPENGL
		m_renderer = new GLRenderer;
		#else
		Log::E("Crown Engine was not built with OpenGL support.");
		return false;
		#endif
	}
	else if (m_preferred_renderer == RENDERER_GLES)
	{
		#ifdef CROWN_BUILD_OPENGLES
		m_renderer = new GLESRenderer;
		#else
		Log::E("Crown Engine was not built with OpenGL|ES support.");
		return false;
		#endif
	}

	// Create debug renderer
	m_debug_renderer = new DebugRenderer(*m_renderer);

	Log::I("Crown Engine initialized.");

	Log::I("Initializing Game...");

	const char* game_library_path = m_filesystem->build_os_path(m_filesystem->root_path(), GAME_LIBRARY_NAME);
	m_game_library = os::open_library(game_library_path);

	if (m_game_library == NULL)
	{
		Log::E("Error while loading game library.");
		return false;
	}

	create_game_t* create_game = (create_game_t*)os::lookup_symbol(m_game_library, "create_game");

	m_game = create_game();

	m_game->init();

	m_is_init = true;

	start();

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

	os::close_library(m_game_library);

	if (m_input_manager)
	{
		delete m_input_manager;
	}

	Log::I("Releasing Renderer...");

	if (m_renderer)
	{
		delete m_renderer;
	}

	Log::I("Releasing DebugRenderer...");
	if (m_debug_renderer)
	{
		delete m_debug_renderer;
	}

	Log::I("Releasing ResourceManager...");
	if (m_resource_archive)
	{
		delete m_resource_archive;
	}

	if (m_resource_manager)
	{
		delete m_resource_manager;
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
ResourceManager* Device::resource_manager()
{
	return m_resource_manager;
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
DebugRenderer* Device::debug_renderer()
{
	return m_debug_renderer;
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

	m_debug_renderer->draw_all();

	m_renderer->end_frame();
}

//-----------------------------------------------------------------------------
bool Device::parse_command_line(int argc, char** argv)
{
	ArgsOption options[] = 
	{
		"help",       AOA_NO_ARGUMENT,       NULL,        'i',
		"root-path",  AOA_REQUIRED_ARGUMENT, NULL,        'r',
		"user-path",  AOA_REQUIRED_ARGUMENT, NULL,        'u',
		"width",      AOA_REQUIRED_ARGUMENT, NULL,        'w',
		"height",     AOA_REQUIRED_ARGUMENT, NULL,        'h',
		"fullscreen", AOA_NO_ARGUMENT,       &m_preferred_window_fullscreen, 1,
		"gl",         AOA_NO_ARGUMENT,       &m_preferred_renderer, RENDERER_GL,
		"gles",       AOA_NO_ARGUMENT,       &m_preferred_renderer, RENDERER_GLES,
		"dev",        AOA_NO_ARGUMENT,       &m_preferred_mode, MODE_DEVELOPMENT,
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
				if (!os::is_absolute_path(args.option_argument()))
				{
					os::printf("%s: error: the root path must be absolute.\n", argv[0]);
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
				if (!os::is_absolute_path(args.option_argument()))
				{
					os::printf("%s: error: the user path must be absolute.\n", argv[0]);
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
	os::printf("  --gl                  Use OpenGL as rendering backend.\n");
	os::printf("  --gles                Use OpenGL|ES as rendering backend.\n");  
	os::printf("  --dev                 Run the engine in development mode\n");
}

Device g_device;
Device* device()
{
	return &g_device;
}

} // namespace crown

