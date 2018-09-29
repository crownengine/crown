/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "config.h"
#include "core/containers/types.h"
#include "core/filesystem/types.h"
#include "core/memory/allocator.h"
#include "core/memory/linear_allocator.h"
#include "core/strings/string_id.h"
#include "core/types.h"
#include "device/boot_config.h"
#include "device/console_server.h"
#include "device/device_options.h"
#include "device/display.h"
#include "device/input_types.h"
#include "device/pipeline.h"
#include "device/window.h"
#include "lua/types.h"
#include "resource/types.h"
#include "resource/types.h"
#include "world/types.h"

/// @defgroup Device Device
namespace crown
{
struct BgfxAllocator;
struct BgfxCallback;

/// This is the place where to look for accessing all of
/// the engine subsystems and related stuff.
///
/// @ingroup Device
struct Device
{
	LinearAllocator _allocator;

	const DeviceOptions& _device_options;
	BootConfig _boot_config;
	ConsoleServer* _console_server;
	Filesystem* _data_filesystem;
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
	Pipeline* _pipeline;
	Display* _display;
	Window* _window;
	ListNode _worlds;

	u16 _width;
	u16 _height;

	bool _quit;
	bool _paused;

	bool process_events(bool vsync);

	///
	Device(const DeviceOptions& opts, ConsoleServer& cs);
	Device(const Device&) = delete;
	Device& operator=(const Device&) = delete;

	/// Runs the engine.
	void run();

	/// Returns the number of command line parameters.
	int argc() const { return _device_options._argc; }

	/// Returns command line parameters.
	const char** argv() const { return (const char**)_device_options._argv; }

	/// Quits the application.
	void quit();

	/// Pauses the engine.
	void pause();

	/// Unpauses the engine.
	void unpause();

	/// Returns the main window resolution.
	void resolution(u16& width, u16& height);

	/// Renders @a world using @a camera.
	void render(World& world, UnitId camera_unit);

	/// Creates a new world.
	World* create_world();

	/// Destroys the @a world.
	void destroy_world(World& world);

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
	void log(const char* msg);
};

/// Runs the engine.
void run(const DeviceOptions& opts);

/// Returns the device.
Device* device();

} // namespace crown
