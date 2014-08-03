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
*/

#pragma once

#include "Types.h"
#include "Config.h"
#include "OS.h"
#include "LinearAllocator.h"
#include "Resource.h"
#include "WorldTypes.h"

namespace crown
{

struct Accelerometer;
class Bundle;
class BundleCompiler;
class ConsoleServer;
class Filesystem;
struct Keyboard;
class LuaEnvironment;
struct Mouse;
class OsWindow;
class Renderer;
class ResourceManager;
struct ResourcePackage;
struct Touch;
class World;
class WorldManager;
struct Camera;

/// @defgroup Device Device

/// Holds data for a display mode.
///
/// @ingroup Device
struct DisplayMode
{
	uint32_t id;
	uint16_t width;
	uint16_t height;
};

/// This is the place where to look for accessing all of
/// the engine subsystems and related stuff.
///
/// @ingroup Device
class Device
{
public:

	Device();
	~Device();

	void init();

	/// Shutdowns the engine freeing all the allocated resources
	void shutdown();

	/// Returns the number of command line arguments passed to
	/// the engine executable.
	int32_t argc() const { return m_argc; }

	/// Returns the string value of the command line arguments passed
	/// to the engine executable.
	/// The size of the returned array is given by Device::argc().
	const char** argv() const { return (const char**) m_argv; }

	/// Returns wheter the engine is running (i.e. it is actually
	/// doing work).
	bool is_running() const;

	/// Returns whether the engine is correctly initialized
	bool is_init() const;

	/// Returns wheter the engine is paused
	bool is_paused() const;

	/// Return the number of frames rendered from the first
	/// call to Device::start()
	uint64_t frame_count() const;

	/// Returns the time in seconds needed to render the last frame
	float last_delta_time() const;

	/// Returns the time in seconds since the first call to start().
	double time_since_start() const;

	/// Forces the engine to actually start doing work.
	void start();

	/// Forces the engine to stop all the work it is doing
	/// and normally terminates the program.
	void stop();

	/// Pauses the engine
	void pause();

	/// Unpauses the engine
	void unpause();

	virtual int32_t run(int argc, char** argv) = 0;

	/// Returns an array of video @a modes.
	/// Each DisplayMode has an id that can be passed to set_video_mode().
	virtual void display_modes(Array<DisplayMode>& modes) = 0;

	/// Sets the video mode @a id.
	/// @note See video_modes().
	virtual void set_display_mode(uint32_t id) = 0;

	/// Sets whether in fullscreen or not.
	virtual void set_fullscreen(bool full) = 0;

	/// Updates all the subsystems
	void frame();

	/// Updates the given @a world and renders it from the given @a camera.
	void update_world(World* world, float dt);

	/// Renders the given @a world from the point of view of the given @æ camera.
	void render_world(World* world, Camera* camera);

	WorldId create_world();
	void destroy_world(WorldId world);

	/// Returns the resource package with the given @a package_name name.
	ResourcePackage* create_resource_package(const char* name);

	/// Destroy a previously created resource @a package.
	/// @note
	/// To unload the resources loaded by the package, you have to call
	/// ResourcePackage::unload() first.
	void destroy_resource_package(ResourcePackage* package);

	void reload(const char* type, const char* name);

	Filesystem* filesystem();
	ResourceManager* resource_manager();
	LuaEnvironment* lua_environment();

	OsWindow* window();
	Renderer* renderer();

	Keyboard* keyboard();
	Mouse* mouse();
	Touch* touch();
	Accelerometer* accelerometer();
	ConsoleServer* console() { return m_console; }
	WorldManager* world_manager() { return m_world_manager; }

protected:

	// Used to allocate all subsystems
	LinearAllocator m_allocator;

	int32_t m_argc;
	char** m_argv;

	// Preferred settings
	char m_source_dir[MAX_PATH_LENGTH];
	char m_bundle_dir[MAX_PATH_LENGTH];
	char m_boot_file[MAX_PATH_LENGTH];
	int32_t m_fileserver;
	uint16_t m_console_port;

	bool m_is_init		: 1;
	bool m_is_running	: 1;
	bool m_is_paused	: 1;

	uint64_t m_frame_count;

	uint64_t m_last_time;
	uint64_t m_current_time;
	float m_last_delta_time;
	double m_time_since_start;

	// Public subsystems
	Filesystem* m_filesystem;

	OsWindow* m_window;

	Keyboard* m_keyboard;
	Mouse* m_mouse;
	Touch* m_touch;

	LuaEnvironment* m_lua_environment;
	Renderer* m_renderer;

	// Private subsystems
	BundleCompiler* m_bundle_compiler;
	ConsoleServer* m_console;
	ResourceManager* m_resource_manager;
	Bundle* m_resource_bundle;
	WorldManager* m_world_manager;

private:

	// Disable copying
	Device(const Device&);
	Device& operator=(const Device&);
};

CE_EXPORT Device* device();

CE_EXPORT void set_device(Device* device);

} // namespace crown
