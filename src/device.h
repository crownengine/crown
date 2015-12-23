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
#include "os_event_queue.h"
#include "string_id.h"
#include "allocator.h"
#include "log.h"
#include "proxy_allocator.h"
#include "string_utils.h"
#include <bx/allocator.h>
#include <bgfx/bgfx.h>

namespace crown
{

/// @defgroup Device Device

/// This is the place where to look for accessing all of
/// the engine subsystems and related stuff.
///
/// @ingroup Device
struct Device
{
	Device(DeviceOptions& opts);

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

	bool process_events();
	void read_config();

private:

	// Used to allocate all subsystems
	LinearAllocator _allocator;

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

	DeviceOptions& _device_options;
	Filesystem* _bundle_filesystem;
	StringId64 _boot_package_id;
	StringId64 _boot_script_id;
	ResourcePackage* _boot_package;

	LuaEnvironment* _lua_environment;
	ResourceLoader* _resource_loader;
	ResourceManager* _resource_manager;
	InputManager* _input_manager;

	Array<World*> _worlds;

	struct BgfxCallback : public bgfx::CallbackI
	{
		virtual void fatal(bgfx::Fatal::Enum _code, const char* _str)
		{
			CE_ASSERT(false, "Fatal error: 0x%08x: %s", _code, _str);
		}

		virtual void traceVargs(const char* /*_filePath*/, uint16_t /*_line*/, const char* _format, va_list _argList)
		{
			char buf[2048];
			strncpy(buf, _format, sizeof(buf));
			buf[strlen32(buf)-1] = '\0'; // Remove trailing newline
			CE_LOGDV(buf, _argList);
		}

		virtual uint32_t cacheReadSize(uint64_t /*_id*/)
		{
			return 0;
		}

		virtual bool cacheRead(uint64_t /*_id*/, void* /*_data*/, uint32_t /*_size*/)
		{
			return false;
		}

		virtual void cacheWrite(uint64_t /*_id*/, const void* /*_data*/, uint32_t /*_size*/)
		{
		}

		virtual void screenShot(const char* /*_filePath*/, uint32_t /*_width*/, uint32_t /*_height*/, uint32_t /*_pitch*/, const void* /*_data*/, uint32_t /*_size*/, bool /*_yflip*/)
		{
		}

		virtual void captureBegin(uint32_t /*_width*/, uint32_t /*_height*/, uint32_t /*_pitch*/, bgfx::TextureFormat::Enum /*_format*/, bool /*_yflip*/)
		{
		}

		virtual void captureEnd()
		{
		}

		virtual void captureFrame(const void* /*_data*/, uint32_t /*_size*/)
		{
		}
	};

	struct BgfxAllocator : public bx::AllocatorI
	{
		BgfxAllocator(Allocator& a)
			: _allocator("bgfx", a)
		{
		}

		virtual void* realloc(void* _ptr, size_t _size, size_t _align, const char* /*_file*/, uint32_t /*_line*/)
		{
			if (!_ptr)
				return _allocator.allocate((uint32_t)_size, (uint32_t)_align == 0 ? 1 : (uint32_t)_align);

			if (_size == 0)
			{
				_allocator.deallocate(_ptr);
				return NULL;
			}

			// Realloc
			void* p = _allocator.allocate((uint32_t)_size, (uint32_t)_align == 0 ? 1 : (uint32_t)_align);
			_allocator.deallocate(_ptr);
			return p;
		}

	private:

		ProxyAllocator _allocator;
	};

	BgfxCallback _bgfx_callback;
	BgfxAllocator _bgfx_allocator;

private:

	// Disable copying
	Device(const Device&);
	Device& operator=(const Device&);
};

bool next_event(OsEvent& ev);
void init(DeviceOptions& opts);
void update();
void shutdown();
Device* device();

} // namespace crown
