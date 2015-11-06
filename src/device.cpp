/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "device.h"
#include "array.h"
#include "config.h"
#include "debug_line.h"
#include "input_manager.h"
#include "log.h"
#include "lua_environment.h"
#include "material_manager.h"
#include "memory.h"
#include "os.h"
#include "resource_manager.h"
#include "resource_package.h"
#include "types.h"
#include "world.h"
#include "json_parser.h"
#include "filesystem.h"
#include "path.h"
#include "disk_filesystem.h"

#define MAX_SUBSYSTEMS_HEAP 8 * 1024 * 1024

namespace crown
{

Device::Device(const DeviceOptions& opts)
	: _allocator(default_allocator(), MAX_SUBSYSTEMS_HEAP)
	, _width(0)
	, _height(0)
	, _is_init(false)
	, _is_running(false)
	, _is_paused(false)
	, _frame_count(0)
	, _last_time(0)
	, _current_time(0)
	, _last_delta_time(0.0f)
	, _time_since_start(0.0)
	, _device_options(opts)
	, _bundle_filesystem(NULL)
	, _boot_package_id(uint64_t(0))
	, _boot_script_id(uint64_t(0))
	, _boot_package(NULL)
	, _lua_environment(NULL)
	, _resource_manager(NULL)
	, _input_manager(NULL)
	, _worlds(default_allocator())
{
}

void Device::init()
{
	// Initialize
	CE_LOGI("Initializing Crown Engine %s...", version());

	_bundle_filesystem = CE_NEW(_allocator, DiskFilesystem)(_device_options.bundle_dir());

	read_config();

	// Create resource manager
	CE_LOGD("Creating resource manager...");
	_resource_manager = CE_NEW(_allocator, ResourceManager)(*_bundle_filesystem);

	CE_LOGD("Creating material manager...");
	material_manager::init();
	debug_line::init();

	_lua_environment = CE_NEW(_allocator, LuaEnvironment)();
	_lua_environment->load_libs();

	_input_manager = CE_NEW(_allocator, InputManager)();

	CE_LOGD("Crown Engine initialized.");
	CE_LOGD("Initializing Game...");

	_is_init = true;
	_is_running = true;
	_last_time = os::clocktime();

	_boot_package = create_resource_package(_boot_package_id);
	_boot_package->load();
	_boot_package->flush();

	_lua_environment->execute((LuaResource*)_resource_manager->get(SCRIPT_TYPE, _boot_script_id));
	_lua_environment->call_global("init", 0);
}

void Device::shutdown()
{
	CE_ASSERT(_is_init, "Engine is not initialized");

	CE_DELETE(_allocator, _input_manager);

	// Shutdowns the game
	_lua_environment->call_global("shutdown", 0);

	_boot_package->unload();
	destroy_resource_package(*_boot_package);

	CE_DELETE(_allocator, _lua_environment);

	CE_LOGD("Releasing material manager...");
	debug_line::shutdown();
	material_manager::shutdown();

	CE_LOGD("Releasing resource manager...");
	CE_DELETE(_allocator, _resource_manager);

	CE_DELETE(_allocator, _bundle_filesystem);

	_allocator.clear();
	_is_init = false;
}

void Device::quit()
{
	_is_running = false;
}

void Device::pause()
{
	_is_paused = true;
	CE_LOGI("Engine paused.");
}

void Device::unpause()
{
	_is_paused = false;
	CE_LOGI("Engine unpaused.");
}

void Device::update_resolution(uint16_t width, uint16_t height)
{
	_width = width;
	_height = height;
}

void Device::resolution(uint16_t& width, uint16_t& height)
{
	width = _width;
	height = _height;
}

bool Device::is_running() const
{
	return _is_running;
}

uint64_t Device::frame_count() const
{
	return _frame_count;
}

float Device::last_delta_time() const
{
	return _last_delta_time;
}

double Device::time_since_start() const
{
	return _time_since_start;
}

void Device::update()
{
	_current_time = os::clocktime();
	const int64_t time = _current_time - _last_time;
	_last_time = _current_time;
	const double freq = (double) os::clockfrequency();
	_last_delta_time = time * (1.0 / freq);
	_time_since_start += _last_delta_time;

	if (!_is_paused)
	{
		_resource_manager->complete_requests();
		_lua_environment->call_global("update", 1, ARGUMENT_FLOAT, last_delta_time());
		_lua_environment->call_global("render", 1, ARGUMENT_FLOAT, last_delta_time());
	}

	_input_manager->update();

	_frame_count++;
	_lua_environment->clear_temporaries();
}

void Device::render_world(World& world, Camera* camera)
{
	world.render(camera);
}

World* Device::create_world()
{
	World* w = CE_NEW(default_allocator(), World)(*_resource_manager, *_lua_environment);
	array::push_back(_worlds, w);
	return w;
}

void Device::destroy_world(World& w)
{
	for (uint32_t i = 0, n = array::size(_worlds); i < n; ++i)
	{
		if (&w == _worlds[i])
		{
			CE_DELETE(default_allocator(), &w);
			_worlds[i] = _worlds[n - 1];
			array::pop_back(_worlds);
			return;
		}
	}

	CE_ASSERT(false, "Bad world");
}

ResourcePackage* Device::create_resource_package(StringId64 id)
{
	return CE_NEW(default_allocator(), ResourcePackage)(id, *_resource_manager);
}

void Device::destroy_resource_package(ResourcePackage& package)
{
	CE_DELETE(default_allocator(), &package);
}

void Device::reload(StringId64 type, StringId64 name)
{
	const void* old_resource = _resource_manager->get(type, name);
	_resource_manager->reload(type, name);
	const void* new_resource = _resource_manager->get(type, name);

	if (type == SCRIPT_TYPE)
	{
		_lua_environment->execute((const LuaResource*)new_resource);
	}
}

ResourceManager* Device::resource_manager()
{
	return _resource_manager;
}

LuaEnvironment* Device::lua_environment()
{
	return _lua_environment;
}

InputManager* Device::input_manager()
{
	return _input_manager;
}

void Device::read_config()
{
	TempAllocator1024 ta;
	DynamicString project_path(ta);

	if (_device_options.project() != NULL)
	{
		project_path += _device_options.project();
		project_path += path::SEPARATOR;
	}

	project_path += "crown.config";

	File* tmpfile = _bundle_filesystem->open(project_path.c_str(), FOM_READ);
	JSONParser config(*tmpfile);
	_bundle_filesystem->close(tmpfile);
	JSONElement root = config.root();

	_boot_script_id = root.key("boot_script").to_resource_id();
	_boot_package_id = root.key("boot_package").to_resource_id();
}

namespace device_globals
{
	char _buffer[sizeof(Device)];
	Device* _device = NULL;

	void init(const DeviceOptions& opts)
	{
		CE_ASSERT(_device == NULL, "Crown already initialized");
		_device = new (_buffer) Device(opts);
		_device->init();
	}

	void shutdown()
	{
		_device->shutdown();
		_device->~Device();
		_device = NULL;
	}
} // namespace device_globals

Device* device()
{
	return device_globals::_device;
}

} // namespace crown
