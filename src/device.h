/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "allocator.h"
#include "config.h"
#include "container_types.h"
#include "device_options.h"
#include "display.h"
#include "filesystem_types.h"
#include "input_types.h"
#include "linear_allocator.h"
#include "lua_types.h"
#include "os_event_queue.h"
#include "resource_types.h"
#include "string_id.h"
#include "types.h"
#include "window.h"
#include "world_types.h"

namespace crown
{

struct BgfxAllocator;
struct BgfxCallback;

/// @defgroup Device Device

/// This is the place where to look for accessing all of
/// the engine subsystems and related stuff.
///
/// @ingroup Device
class Device
{
	LinearAllocator _allocator;

	const DeviceOptions& _device_options;
	Filesystem* _bundle_filesystem;
	ResourceLoader* _resource_loader;
	ResourceManager* _resource_manager;
	BgfxAllocator* _bgfx_allocator;
	BgfxCallback* _bgfx_callback;
	ShaderManager* _shader_manager;
	MaterialManager* _material_manager;
	InputManager* _input_manager;
	UnitManager* _unit_manager;
	LuaEnvironment* _lua_environment;
	Display* _display;
	Window* _window;

	StringId64 _boot_package_id;
	StringId64 _boot_script_id;
	ResourcePackage* _boot_package;

	Array<World*> _worlds;

	uint16_t _width;
	uint16_t _height;
	int16_t _mouse_curr_x;
	int16_t _mouse_curr_y;
	int16_t _mouse_last_x;
	int16_t _mouse_last_y;

	bool _is_init;
	bool _is_running;
	bool _is_paused;

	uint64_t _frame_count;
	int64_t _last_time;
	int64_t _current_time;
	float _last_delta_time;
	double _time_since_start;

	void read_config();
	bool process_events();

public:

	Device(const DeviceOptions& opts);

	/// Initializes the engine.
	void init();

	/// Shutdowns the engine freeing all the allocated resources.
	void shutdown();

	/// Returns the number of command line parameters.
	int argc() const { return _device_options.argc(); }

	/// Returns command line parameters.
	const char** argv() const { return _device_options.argv(); }

	/// Returns a string identifying what platform the engine is running on.
	const char* platform() const { return CROWN_PLATFORM_NAME; }

	/// Returns a string identifying what architecture the engine is running on.
	const char* architecture() const { return CROWN_ARCH_NAME; }

	/// Returns a string identifying the engine version.
	const char* version() const { return CROWN_VERSION_STRING; }

	/// Returns wheter the engine is running.
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

	/// Returns the main window resolution.
	void resolution(uint16_t& width, uint16_t& height);

	/// Updates all the subsystems.
	void update();

	/// Renders the given @a world from the point of view of @a camera.
	void render_world(World& world, CameraInstance camera);

	/// Creates a new world.
	World* create_world();

	/// Destroys the world @a w.
	void destroy_world(World& w);

	/// Returns the resource package @a id.
	ResourcePackage* create_resource_package(StringId64 id);

	/// Destroys the resource package @a rp.
	/// @note
	/// Resources are not automatically unloaded.
	/// You have to call ResourcePackage::unload() before destroying a package.
	void destroy_resource_package(ResourcePackage& rp);

	/// Reloads the resource @a type @a name.
	void reload(StringId64 type, StringId64 name);

	/// Returns the resource manager.
	ResourceManager* resource_manager();

	/// Returns the lua environment.
	LuaEnvironment* lua_environment();

	/// Returns the input manager.
	InputManager* input_manager();

	/// Returns the shader manager.
	ShaderManager* shader_manager();

	/// Returns the material manager.
	MaterialManager* material_manager();

	/// Returns the unit manager.
	UnitManager* unit_manager();

	/// Returns the main display.
	Display* display();

	/// Returns the main window.
	Window* window();

private:

	// Disable copying
	Device(const Device&);
	Device& operator=(const Device&);
};

bool next_event(OsEvent& ev);
void init(const DeviceOptions& opts);
void update();
void shutdown();
Device* device();

} // namespace crown
