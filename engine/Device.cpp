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
OTHER DEALINGS IN THE SOFTWARE.
*/

#include <cstdlib>

#include "Config.h"
#include "Device.h"
#include "Accelerometer.h"
#include "Args.h"
#include "DebugRenderer.h"
#include "DiskFile.h"
#include "DiskFilesystem.h"
#include "JSONParser.h"
#include "Keyboard.h"
#include "Log.h"
#include "LuaEnvironment.h"
#include "Memory.h"
#include "Mouse.h"
#include "OS.h"
#include "OsWindow.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "StringSetting.h"
#include "StringUtils.h"
#include "TextReader.h"
#include "Touch.h"
#include "Types.h"
#include "Bundle.h"
#include "TempAllocator.h"
#include "ResourcePackage.h"
#include "RPCServer.h"
#include "SoundRenderer.h"
#include "World.h"
#include "LuaStack.h"

#if defined(LINUX) || defined(WINDOWS)
	#include "BundleCompiler.h"
#endif

#if defined(ANDROID)
	#include "ApkFilesystem.h"
#endif

#define MAX_SUBSYSTEMS_HEAP 16 * 1024 * 1024

namespace crown
{
//-----------------------------------------------------------------------------
Device::Device()
	: m_allocator(default_allocator(), MAX_SUBSYSTEMS_HEAP)
	, m_is_init(false)
	, m_is_running(false)
	, m_is_paused(false)
	, m_is_really_paused(false)

	, m_frame_count(0)

	, m_last_time(0)
	, m_current_time(0)
	, m_last_delta_time(0.0f)
	, m_time_since_start(0.0)

	, m_filesystem(NULL)
	, m_lua_environment(NULL)
	, m_renderer(NULL)
	, m_debug_renderer(NULL)
	, m_sound_renderer(NULL)

	, m_bundle_compiler(NULL)
	, m_rpc(NULL)
	, m_resource_manager(NULL)
	, m_resource_bundle(NULL)

	, m_renderer_init_request(false)
{
	// Bundle dir is current dir by default.
	string::strncpy(m_bundle_dir, os::get_cwd(), MAX_PATH_LENGTH);
	string::strncpy(m_source_dir, "", MAX_PATH_LENGTH);
	string::strncpy(m_boot_file, "lua/game", MAX_PATH_LENGTH);
}

//-----------------------------------------------------------------------------
Device::~Device()
{
}

//-----------------------------------------------------------------------------
void Device::init()
{
	// Initialize
	Log::i("Initializing Crown Engine %d.%d.%d...", CROWN_VERSION_MAJOR, CROWN_VERSION_MINOR, CROWN_VERSION_MICRO);

	// RPC only in debug or development builds
	#if defined(CROWN_DEBUG) || defined(CROWN_DEVELOPMENT)
		m_rpc = CE_NEW(m_allocator, RPCServer);
		m_rpc->add_handler(&m_command_handler);
		m_rpc->add_handler(&m_script_handler);
		m_rpc->add_handler(&m_stats_handler);
		m_rpc->add_handler(&m_ping_handler);
		m_rpc->init(false);
	#endif

	// Default bundle filesystem
	#if defined (LINUX) || defined(WINDOWS)
		m_filesystem = CE_NEW(m_allocator, DiskFilesystem)(m_bundle_dir);
	#elif defined(ANDROID)
		m_filesystem = CE_NEW(m_allocator, ApkFilesystem)();
	#endif
	Log::d("Filesystem created.");

	// Read settings from crown.config
	read_engine_settings();

	m_resource_bundle = Bundle::create(m_allocator, *m_filesystem);

	// Create resource manager
	m_resource_manager = CE_NEW(m_allocator, ResourceManager)(*m_resource_bundle, 0);
	Log::d("Resource manager created.");
	Log::d("Resource seed: %d", m_resource_manager->seed());

	// Create window
	m_window = CE_NEW(m_allocator, OsWindow);

	// Create input devices
	m_keyboard = CE_NEW(m_allocator, Keyboard);
	m_mouse = CE_NEW(m_allocator, Mouse);
	m_touch = CE_NEW(m_allocator, Touch);

	// Create renderer
	m_renderer = CE_NEW(m_allocator, Renderer)(m_allocator);
	m_renderer->init();
	Log::d("Renderer created.");

	// Create debug renderer
	m_debug_renderer = CE_NEW(m_allocator, DebugRenderer)(*m_renderer);
	Log::d("Debug renderer created.");

	m_lua_environment = CE_NEW(m_allocator, LuaEnvironment)();
	m_lua_environment->init();
	Log::d("Lua environment created.");

	m_sound_renderer = CE_NEW(m_allocator, SoundRenderer)(m_allocator);
	m_sound_renderer->init();
	Log::d("SoundRenderer created.");

	Log::d("Crown Engine initialized.");
	Log::d("Initializing Game...");

	m_is_init = true;
	start();

	// Execute lua boot file
	if (m_lua_environment->load_and_execute(m_boot_file))
	{
		if (!m_lua_environment->call_global("init", 0))
		{
			pause();
		}
	}
	else
	{
		pause();
	}

	Log::d("Total allocated size: %llu", m_allocator.allocated_size());
}

//-----------------------------------------------------------------------------
void Device::shutdown()
{
	CE_ASSERT(is_init(), "Engine is not initialized");

	// Shutdowns the game
	m_lua_environment->call_global("shutdown", 0);


	Log::d("Releasing SoundRenderer...");
	if (m_sound_renderer)
	{
		m_sound_renderer->shutdown();

		CE_DELETE(m_allocator, m_sound_renderer);
	}

	Log::d("Releasing LuaEnvironment...");
	if (m_lua_environment)
	{
		m_lua_environment->shutdown();
		
		CE_DELETE(m_allocator, m_lua_environment);
	}

	Log::d("Releasing Input Devices...");
	CE_DELETE(m_allocator, m_touch);
	CE_DELETE(m_allocator, m_mouse);
	CE_DELETE(m_allocator, m_keyboard);

	Log::d("Releasing DebugRenderer...");
	if (m_debug_renderer)
	{
		CE_DELETE(m_allocator, m_debug_renderer);
	}

	Log::d("Releasing Renderer...");
	if (m_renderer)
	{
		m_renderer->shutdown();
		CE_DELETE(m_allocator, m_renderer);
	}

	Log::d("Releasing ResourceManager...");
	if (m_resource_manager)
	{
		CE_DELETE(m_allocator, m_resource_manager);
	}

	if (m_resource_bundle)
	{
		Bundle::destroy(m_allocator, m_resource_bundle);
	}

	Log::d("Releasing Filesystem...");
	if (m_filesystem)
	{
		CE_DELETE(m_allocator, m_filesystem);
	}

	#if defined(CROWN_DEBUG) || defined(CROWN_DEVELOPMENT)
		m_rpc->execute_callbacks();
		m_rpc->shutdown();
		CE_DELETE(m_allocator, m_rpc);
		m_rpc = NULL;
	#endif

	m_allocator.clear();
	m_is_init = false;
}

//-----------------------------------------------------------------------------
bool Device::is_init() const
{
	return m_is_init;
}

//-----------------------------------------------------------------------------
bool Device::is_paused() const
{
	return m_is_paused;
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
LuaEnvironment* Device::lua_environment()
{
	return m_lua_environment;
}

//-----------------------------------------------------------------------------
OsWindow* Device::window()
{
	return m_window;
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
SoundRenderer* Device::sound_renderer()
{
	return m_sound_renderer;
}

//-----------------------------------------------------------------------------
Keyboard* Device::keyboard()
{
	return m_keyboard;
}

//-----------------------------------------------------------------------------
Mouse* Device::mouse()
{
	return m_mouse;
}

//-----------------------------------------------------------------------------
Touch* Device::touch()
{
	return m_touch;
}

//-----------------------------------------------------------------------------
Accelerometer* Device::accelerometer()
{
	return NULL;
}

//-----------------------------------------------------------------------------
void Device::start()
{
	CE_ASSERT(m_is_init, "Cannot start uninitialized engine.");

	m_is_running = true;
	m_last_time = os::milliseconds();
}

//-----------------------------------------------------------------------------
void Device::stop()
{
	CE_ASSERT(m_is_init, "Cannot stop uninitialized engine.");

	m_is_running = false;
}

//-----------------------------------------------------------------------------
void Device::pause()
{
	m_is_paused = true;
}

//-----------------------------------------------------------------------------
void Device::unpause()
{
	m_is_paused = false;
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
double Device::time_since_start() const
{
	return m_time_since_start;
}

//-----------------------------------------------------------------------------
void Device::frame()
{
	m_current_time = os::microseconds();
	m_last_delta_time = (m_current_time - m_last_time) / 1000000.0f;
	m_last_time = m_current_time;
	m_time_since_start += m_last_delta_time;

	m_rpc->update();

	if (!m_is_paused)
	{
		m_resource_manager->poll_resource_loader();

		if (!m_lua_environment->call_global("frame", 1, ARGUMENT_FLOAT, last_delta_time()))
		{
			pause();
		}

		m_renderer->frame();

		// FIXME: SoundRenderer should not be updated each frame
		m_sound_renderer->frame();
	}

	m_rpc->execute_callbacks();

	clear_lua_temporaries();

	m_frame_count++;
}

//-----------------------------------------------------------------------------
void Device::render_world(World& world, Camera& camera, float dt)
{
	world.update(camera, dt);
}

//-----------------------------------------------------------------------------
World* Device::create_world()
{
	return CE_NEW(default_allocator(), World);
}

//-----------------------------------------------------------------------------
void Device::destroy_world(World* world)
{
	CE_ASSERT_NOT_NULL(world);

	CE_DELETE(default_allocator(), world);
}

//-----------------------------------------------------------------------------
ResourcePackage* Device::create_resource_package(const char* name)
{
	CE_ASSERT_NOT_NULL(name);

	ResourceId package_id = m_resource_manager->load("package", name);
	m_resource_manager->flush();

	PackageResource* package_res = (PackageResource*) m_resource_manager->data(package_id);
	ResourcePackage* package = CE_NEW(default_allocator(), ResourcePackage)(*m_resource_manager, package_id, package_res);

	return package;
}

//-----------------------------------------------------------------------------
void Device::destroy_resource_package(ResourcePackage* package)
{
	CE_ASSERT_NOT_NULL(package);

	m_resource_manager->unload(package->resource_id());
	CE_DELETE(default_allocator(), package);
}

//-----------------------------------------------------------------------------
void Device::compile(const char* , const char* , const char* )
{
}

//-----------------------------------------------------------------------------
void Device::reload(const char* type, const char* name)
{
	#if defined(LINUX) || defined(WINDOWS)
		TempAllocator4096 temp;
		DynamicString filename(temp);
		filename += name;
		filename += '.';
		filename += type;

		if (!m_bundle_compiler->compile(m_bundle_dir, m_source_dir, filename.c_str()))
		{
			Log::d("Compilation failed.");
			return;
		}

		uint32_t type_hash = hash::murmur2_32(type, string::strlen(type), 0);

		switch (type_hash)
		{
			case LUA_TYPE:
			{
				m_lua_environment->load_and_execute(name);
				break;
			}
			default:
			{
				CE_ASSERT(false, "Oops, unknown resource type: %s", type);
				break;
			}
		}
	#endif
}
//-------------------------------------------------------------------------
void Device::read_engine_settings()
{
	// // Check crown.config existance
	// CE_ASSERT(m_filesystem->is_file("crown.config"), "Unable to open crown.config");

	// // Copy crown config in a buffer
	// TempAllocator4096 allocator;

	// File* config_file = m_filesystem->open("crown.config", FOM_READ);

	// char* json_string = (char*)allocator.allocate(config_file->size());

	// config_file->read(json_string, config_file->size());

	// m_filesystem->close(config_file);

	// // Parse crown.config
	// JSONParser parser(json_string);

	// JSONElement root = parser.root();

	// // Boot
	// if (root.has_key("boot"))
	// {
	// 	const char* boot = root.key("boot").string_value();
	// 	const size_t boot_length = string::strlen(boot) + 1;

	// 	string::strncpy(m_boot_file, boot, boot_length);
	// }
	// // Window width
	// if (root.has_key("window_width"))
	// {
	// 	m_preferred_window_width = root.key("window_width").int_value();
	// }
	// // Window height
	// if (root.has_key("window_height"))
	// {
	// 	m_preferred_window_height = root.key("window_height").int_value();
	// }

	// allocator.deallocate(json_string);

	// Log::i("Configuration set");
}

static Device* g_device;
void set_device(Device* device)
{
	g_device = device;
}

Device* device()
{
	return g_device;
}

} // namespace crown
