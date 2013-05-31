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

#include <cstdlib>

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
#include "TextureResource.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Touch.h"
#include "Accelerometer.h"
#include "JSONParser.h"
#include "DiskFile.h"

#ifdef CROWN_BUILD_OPENGL
	#include "renderers/gl/GLRenderer.h"
#endif

#ifdef CROWN_BUILD_OPENGLES
	#include "renderers/gles/GLESRenderer.h"
#endif

namespace crown
{

static void (*game_init)(void) = NULL;
static void (*game_shutdown)(void) = NULL;
static void (*game_frame)(float) = NULL;

//-----------------------------------------------------------------------------
Device::Device() :
	m_preferred_window_width(1000),
	m_preferred_window_height(625),
	m_preferred_window_fullscreen(0),
	m_preferred_renderer(RENDERER_GL),
	m_preferred_mode(MODE_RELEASE),

	m_is_init(false),
	m_is_running(false),

	m_frame_count(0),

	m_last_time(0),
	m_current_time(0),
	m_last_delta_time(0.0f),

	m_filesystem(NULL),
	m_input_manager(NULL),
	m_renderer(NULL),
	m_debug_renderer(NULL),

	m_resource_manager(NULL),
	m_resource_archive(NULL),

	m_game_library(NULL)
{
	// Select executable dir by default
	string::strncpy(m_preferred_root_path, os::get_cwd(), os::MAX_PATH_LENGTH);
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
		Log::e("Crown Engine is already initialized.");
		return false;
	}

	parse_command_line(argc, argv);
	check_preferred_settings();

	// Initialize
	Log::i("Initializing Crown Engine %d.%d.%d...", CROWN_VERSION_MAJOR, CROWN_VERSION_MINOR, CROWN_VERSION_MICRO);

	create_filesystem();

	create_resource_manager();

	create_input_manager();

	create_renderer();

	create_debug_renderer();

	read_engine_settings();

	Log::i("Crown Engine initialized.");

	Log::i("Initializing Game...");

	// Try to locate the game library
	if (!m_filesystem->exists(GAME_LIBRARY_NAME))
	{
		Log::e("Unable to find the game library in the root path.", GAME_LIBRARY_NAME);
		return false;
	}

	// Try to load the game library and bind functions
	const char* game_library_path = m_filesystem->os_path(GAME_LIBRARY_NAME);

	m_game_library = os::open_library(game_library_path);

	if (m_game_library == NULL)
	{
		Log::e("Unable to load the game.");
		return false;
	}

	*(void**)(&game_init) = os::lookup_symbol(m_game_library, "init");
	*(void**)(&game_shutdown) = os::lookup_symbol(m_game_library, "shutdown");
	*(void**)(&game_frame) = os::lookup_symbol(m_game_library, "frame");

	// Initialize the game
	game_init();

	m_is_init = true;

	start();

	return true;
}

//-----------------------------------------------------------------------------
void Device::shutdown()
{
	if (is_init() == false)
	{
		Log::e("Crown Engine is not initialized.");	
		return;
	}

	// Shutdowns the game
	game_shutdown();

	// Unload the game library
	if (m_game_library)
	{
		os::close_library(m_game_library);
	}

	if (m_input_manager)
	{
		delete m_input_manager;
	}

	Log::i("Releasing Renderer...");

	if (m_renderer)
	{
		delete m_renderer;
	}

	Log::i("Releasing DebugRenderer...");
	if (m_debug_renderer)
	{
		delete m_debug_renderer;
	}

	Log::i("Releasing ResourceManager...");
	if (m_resource_archive)
	{
		delete m_resource_archive;
	}

	if (m_resource_manager)
	{
		delete m_resource_manager;
	}

	Log::i("Releasing Filesystem...");

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
Keyboard* Device::keyboard()
{
	return m_input_manager->keyboard();
}

//-----------------------------------------------------------------------------
Mouse* Device::mouse()
{
	return m_input_manager->mouse();
}

//-----------------------------------------------------------------------------
Touch* Device::touch()
{
	return m_input_manager->touch();
}

//-----------------------------------------------------------------------------
Accelerometer* Device::accelerometer()
{
	return m_input_manager->accelerometer();
}

//-----------------------------------------------------------------------------
void Device::start()
{
	if (is_init() == false)
	{
		Log::e("Cannot start uninitialized engine.");
		return;
	}

	m_is_running = true;

	m_last_time = os::milliseconds();
}

//-----------------------------------------------------------------------------
void Device::stop()
{
	if (is_init() == false)
	{
		Log::e("Cannot stop uninitialized engine.");
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
uint64_t Device::frame_count() const
{
	return m_frame_count;
}

//-----------------------------------------------------------------------------
float Device::last_delta_time() const
{
	return m_last_delta_time;
}

//-----------------------------------------------------------------------------
void Device::frame()
{
	m_current_time = os::microseconds();
	m_last_delta_time = (m_current_time - m_last_time) / 1000000.0f;
	m_last_time = m_current_time;

	m_resource_manager->check_load_queue();
	m_resource_manager->bring_loaded_online();

	m_input_manager->event_loop();

	m_renderer->begin_frame();

	game_frame(last_delta_time());

	m_debug_renderer->draw_all();

	m_renderer->end_frame();

	m_frame_count++;
}

//-----------------------------------------------------------------------------
ResourceId Device::load(const char* name)
{
	return m_resource_manager->load(name);
}

//-----------------------------------------------------------------------------
void Device::unload(ResourceId name)
{
	m_resource_manager->unload(name);
}

//-----------------------------------------------------------------------------
void Device::reload(ResourceId name)
{
	(void)name;
}

//-----------------------------------------------------------------------------
bool Device::is_loaded(ResourceId name)
{
	return m_resource_manager->is_loaded(name);
}

//-----------------------------------------------------------------------------
const void* Device::data(ResourceId name)
{
	return m_resource_manager->data(name);
}

//-----------------------------------------------------------------------------
void Device::create_filesystem()
{
	m_filesystem = new Filesystem(m_preferred_root_path);

	Log::d("Filesystem created.");
	Log::d("Filesystem root path: %s", m_filesystem->root_path());
}

//-----------------------------------------------------------------------------
void Device::create_resource_manager()
{
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

	Log::d("Resource manager created.");
	Log::d("Resource seed: %d", m_resource_manager->seed());
}

//-----------------------------------------------------------------------------
void Device::create_input_manager()
{
	// Create input manager
	m_input_manager = new InputManager();

	Log::d("Input manager created.");
}

//-----------------------------------------------------------------------------
void Device::create_renderer()
{
	// Select appropriate renderer
	if (m_preferred_renderer == RENDERER_GL)
	{
		#ifdef CROWN_BUILD_OPENGL
		m_renderer = new GLRenderer;
		#else
		Log::e("Crown Engine was not built with OpenGL support.");
		exit(EXIT_FAILURE);
		#endif
	}
	else if (m_preferred_renderer == RENDERER_GLES)
	{
		#ifdef CROWN_BUILD_OPENGLES
		m_renderer = new GLESRenderer;
		#else
		Log::e("Crown Engine was not built with OpenGL|ES support.");
		exit(EXIT_FAILURE);
		#endif
	}

	Log::d("Renderer created.");
}

//-----------------------------------------------------------------------------
void Device::create_debug_renderer()
{
	// Create debug renderer
	m_debug_renderer = new DebugRenderer(*m_renderer);

	Log::d("Debug renderer created.");
}

//-----------------------------------------------------------------------------
void Device::parse_command_line(int argc, char** argv)
{
	static ArgsOption options[] = 
	{
		"help",       AOA_NO_ARGUMENT,       NULL,        'i',
		"root-path",  AOA_REQUIRED_ARGUMENT, NULL,        'r',
		"width",      AOA_REQUIRED_ARGUMENT, NULL,        'w',
		"height",     AOA_REQUIRED_ARGUMENT, NULL,        'h',
		"fullscreen", AOA_NO_ARGUMENT,       &m_preferred_window_fullscreen, 1,
		"gl",         AOA_NO_ARGUMENT,       &m_preferred_renderer, RENDERER_GL,
		"gles",       AOA_NO_ARGUMENT,       &m_preferred_renderer, RENDERER_GLES,
		"dev",        AOA_NO_ARGUMENT,       &m_preferred_mode, MODE_DEVELOPMENT,
		NULL, 0, NULL, 0
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
			// Root path
			case 'r':
			{
				string::strcpy(m_preferred_root_path, args.optarg());
				break;
			}
			// Window width
			case 'w':
			{
				m_preferred_window_width = atoi(args.optarg());
				break;
			}
			// Window height
			case 'h':
			{
				m_preferred_window_height = atoi(args.optarg());
				break;
			}
			case 'i':
			case '?':
			default:
			{
				print_help_message();
				exit(EXIT_FAILURE);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void Device::check_preferred_settings()
{
	if (!os::is_absolute_path(m_preferred_root_path))
	{
		Log::e("The root path must be absolute.");
		exit(EXIT_FAILURE);
	}

	if (m_preferred_window_width == 0 || m_preferred_window_height == 0)
	{
		Log::e("Window width and height must be greater than zero.");
		exit(EXIT_FAILURE);
	}
}

//-----------------------------------------------------------------------------
void Device::read_engine_settings()
{
	DiskFile* file = m_filesystem->open("crown.cfg", FOM_READ);
	JSONParser json(file);
	
	// const char* value = json.get_array("crown").get_object("utils").get_string("file").to_string();

	// os::printf("Value:\t%s\n", value);
}


//-----------------------------------------------------------------------------
void Device::print_help_message()
{
	os::printf(
	"Usage: crown [options]\n"
	"Options:\n\n"

	"All of the following options take precedence over\n"
	"environment variables and configuration files.\n\n"

	"  --help                Show this help.\n"
	"  --root-path <path>    Use <path> as the filesystem root path.\n"
	"  --width <width>       Set the <width> of the render window.\n"
	"  --height <width>      Set the <height> of the render window.\n"
	"  --fullscreen          Start in fullscreen.\n"
	"  --dev                 Run the engine in development mode\n");
}

Device g_device;
Device* device()
{
	return &g_device;
}

} // namespace crown

