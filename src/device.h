/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "config.h"
#include "linear_allocator.h"
#include "world_types.h"
#include "resource_types.h"
#include "lua_types.h"
#include "filesystem_types.h"
#include "container_types.h"
#include "input_types.h"
#include "device_options.h"

namespace crown
{

/// @defgroup Device Device

/// This is the place where to look for accessing all of
/// the engine subsystems and related stuff.
///
/// @ingroup Device
struct Device
{
	Device(const DeviceOptions& opts);

	void init();

	/// Shutdowns the engine freeing all the allocated resources.
	void shutdown();

	/// Returns a string identifying what platform the engine is running on.
	const char* platform() const { return CROWN_PLATFORM_NAME; }

	/// Returns a string identifying what architecture the engine is running on.
	const char* architecture() const { return CROWN_ARCH_NAME; }

	/// Returns a string identifying the engine version.
	const char* version() const { return CROWN_VERSION_MAJOR "." CROWN_VERSION_MINOR "." CROWN_VERSION_MICRO; }

	/// Returns wheter the engine is running (i.e. it is advancing
	/// the simulation).
	bool is_running() const;

	/// Return the number of frames rendered.
	uint64_t frame_count() const;

	/// Returns the time in seconds needed to render the last frame.
	float last_delta_time() const;

	/// Returns the time in seconds since the the application started.
	double time_since_start() const;

	/// Quits the application.
	void quit();

	/// Pauses the engine.
	void pause();

	/// Unpauses the engine.
	void unpause();

	void update_resolution(uint16_t width, uint16_t height);

	/// Returns the main window resolution.
	void resolution(uint16_t& width, uint16_t& height);

	/// Updates all the subsystems.
	void update();

	/// Renders the given @a world from the point of view of @a camera.
	void render_world(World& world, Camera* camera);

	/// Creates a new world.
	World* create_world();

	/// Destroys the world @a w.
	void destroy_world(World& w);

	/// Returns the resource package @a id.
	ResourcePackage* create_resource_package(StringId64 id);

	/// Destroy a previously created resource @a package.
	/// @note
	/// To unload the resources loaded by the package, you have to call
	/// ResourcePackage::unload() first.
	void destroy_resource_package(ResourcePackage& package);

	/// Reloads the resource @a type @a name.
	void reload(StringId64 type, StringId64 name);

	/// Returns the resource manager.
	ResourceManager* resource_manager();

	/// Returns the lua environment.
	LuaEnvironment* lua_environment();

	/// Returns the input manager.
	InputManager* input_manager();

private:

	void read_config();

private:

	// Used to allocate all subsystems
	LinearAllocator _allocator;

	uint16_t _width;
	uint16_t _height;

	bool _is_init;
	bool _is_running;
	bool _is_paused;

	uint64_t _frame_count;
	int64_t _last_time;
	int64_t _current_time;
	float _last_delta_time;
	double _time_since_start;

	const DeviceOptions& _device_options;
	Filesystem* _bundle_filesystem;
	StringId64 _boot_package_id;
	StringId64 _boot_script_id;
	ResourcePackage* _boot_package;

	LuaEnvironment* _lua_environment;
	ResourceManager* _resource_manager;
	InputManager* _input_manager;

	Array<World*> _worlds;

private:

	// Disable copying
	Device(const Device&);
	Device& operator=(const Device&);
};

namespace device_globals
{
	void init(const DeviceOptions& opts);
	void shutdown();
} // namespace device_globals

Device* device();

} // namespace crown
