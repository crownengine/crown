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
#include "resource_types.h"
#include "string_id.h"
#include "types.h"
#include "window.h"
#include "world_types.h"

/// @defgroup Device Device
namespace crown
{
struct BgfxAllocator;
struct BgfxCallback;

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

	StringId64 _boot_package_name;
	StringId64 _boot_script_name;
	ResourcePackage* _boot_package;

	u16 _config_window_x;
	u16 _config_window_y;
	u16 _config_window_w;
	u16 _config_window_h;

	Array<World*> _worlds;

	u16 _width;
	u16 _height;
	s16 _mouse_curr_x;
	s16 _mouse_curr_y;
	s16 _mouse_last_x;
	s16 _mouse_last_y;

	bool _is_init;
	bool _is_running;
	bool _is_paused;

	u64 _frame_count;
	s64 _last_time;
	s64 _current_time;
	f32 _last_delta_time;
	f64 _time_since_start;

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
	u64 frame_count() const;

	/// Returns the time in seconds needed to render the last frame.
	f32 last_delta_time() const;

	/// Returns the time in seconds since the the application started.
	f64 time_since_start() const;

	/// Quits the application.
	void quit();

	/// Pauses the engine.
	void pause();

	/// Unpauses the engine.
	void unpause();

	/// Returns the main window resolution.
	void resolution(u16& width, u16& height);

	/// Updates all the subsystems.
	void update();

	/// Renders @a world using @a camera.
	void render(World& world, CameraInstance camera);

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

void init(const DeviceOptions& opts);
void update();
void shutdown();
Device* device();

} // namespace crown
