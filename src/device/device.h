/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "allocator.h"
#include "compiler_types.h"
#include "config.h"
#include "console_server.h"
#include "container_types.h"
#include "device_options.h"
#include "display.h"
#include "filesystem_types.h"
#include "input_types.h"
#include "linear_allocator.h"
#include "log.h"
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
	ConsoleServer* _console_server;
	DataCompiler* _data_compiler;
	Filesystem* _bundle_filesystem;
	File* _last_log;
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
	Array<World*> _worlds;

	u16 _width;
	u16 _height;

	bool _quit;
	bool _paused;

	u64 _frame_count;
	f32 _last_delta_time;
	f64 _time_since_start;

	bool process_events(s16& mouse_x, s16& mouse_y, s16& mouse_last_x, s16& mouse_last_y, bool vsync);

public:

	Device(const DeviceOptions& opts);

	/// Runs the engine.
	void run();

	/// Returns the number of command line parameters.
	int argc() const { return _device_options._argc; }

	/// Returns command line parameters.
	const char** argv() const { return (const char**)_device_options._argv; }

	/// Returns a string identifying what platform the engine is running on.
	const char* platform() const { return CROWN_PLATFORM_NAME; }

	/// Returns a string identifying what architecture the engine is running on.
	const char* architecture() const { return CROWN_ARCH_NAME; }

	/// Returns a string identifying the engine version.
	const char* version() const { return CROWN_VERSION; }

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

	/// Logs @a msg to log file and console.
	void log(const char* msg, LogSeverity::Enum severity);

	/// Returns the console server.
	ConsoleServer* console_server();

	/// Returns the bundle compiler.
	DataCompiler* data_compiler();

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

/// Runs the engine.
void run(const DeviceOptions& opts);

/// Returns the device.
Device* device();

} // namespace crown
