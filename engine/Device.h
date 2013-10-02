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
#include "RPCHandler.h"

namespace crown
{

class Filesystem;
class ResourceManager;
class OsWindow;
class Bundle;
class Renderer;
class DebugRenderer;
class Keyboard;
class Mouse;
class Touch;
class Accelerometer;
class LuaEnvironment;
class BundleCompiler;
class ResourcePackage;
class RPCServer;

/// The Engine.
/// It is the place where to look for accessing all of
/// the engine subsystems and related stuff.
class CE_EXPORT Device
{
public:

							Device();
							~Device();

	void					init();

	/// Shutdowns the engine freeing all the allocated resources
	void					shutdown();

	/// Returns wheter the engine is running (i.e. it is actually
	/// doing work).
	bool					is_running() const;

	/// Returns whether the engine is correctly initialized
	bool					is_init() const;

	/// Returns wheter the engine is paused
	bool 					is_paused() const;

	/// Return the number of frames rendered from the first
	/// call to Device::start()
	uint64_t				frame_count() const;

	/// Returns the time in seconds needed to render the last frame
	float					last_delta_time() const;

	/// Returns the time in seconds since the first call to start().
	double					time_since_start() const;

	/// Forces the engine to actually start doing work.
	void					start();

	/// Forces the engine to stop all the work it is doing
	/// and normally terminates the program.
	void					stop();

	/// Pauses the engine
	void					pause();

	/// Unpauses the engine
	void					unpause();

	virtual int32_t			run(int argc, char** argv) = 0;

	/// Updates all the subsystems
	void					frame();

	/// Returns the resource package with the given @a package_name name.
	ResourcePackage*		create_resource_package(const char* name);

	/// Destroy a previously created resource @a package.
	/// @note
	/// To unload the resources loaded by the package, you have to call
	/// ResourcePackage::unload() first.
	void					destroy_resource_package(ResourcePackage* package);

	void					compile(const char* bundle_dir, const char* source_dir, const char* resource);

	void					reload(const char* type, const char* name);

	Filesystem*				filesystem();
	ResourceManager*		resource_manager();
	LuaEnvironment*			lua_environment();

	OsWindow*				window();
	Renderer*				renderer();
	DebugRenderer*			debug_renderer();

	Keyboard*				keyboard();
	Mouse*					mouse();
	Touch*					touch();
	Accelerometer*			accelerometer();
	RPCServer*				rpc() { return m_rpc; }

private:

	void					read_engine_settings();

protected:

	// Used to allocate all subsystems
	LinearAllocator			m_allocator;

	// Preferred settings
	char					m_source_dir[MAX_PATH_LENGTH];
	char 					m_bundle_dir[MAX_PATH_LENGTH];
	char 					m_boot_file[MAX_PATH_LENGTH];

	bool					m_is_init		: 1;
	bool					m_is_running	: 1;
	bool					m_is_paused		: 1;

	bool 					m_is_really_paused :1;

	uint64_t				m_frame_count;

	uint64_t				m_last_time;
	uint64_t				m_current_time;
	float					m_last_delta_time;
	double					m_time_since_start;

	// Public subsystems
	Filesystem*				m_filesystem;

	OsWindow*				m_window;

	Keyboard*				m_keyboard;
	Mouse*					m_mouse;
	Touch*					m_touch;

	LuaEnvironment*			m_lua_environment;
	Renderer*				m_renderer;
	DebugRenderer*			m_debug_renderer;

	// Private subsystems
	BundleCompiler*			m_bundle_compiler;
	RPCServer*				m_rpc;
	ResourceManager*		m_resource_manager;
	Bundle*					m_resource_bundle;

	RPCCommandHandler		m_command_handler;
	RPCScriptHandler		m_script_handler;
	RPCStatsHandler			m_stats_handler;
	RPCPingHandler			m_ping_handler;

	bool 					m_renderer_init_request;

private:

	// Disable copying
	Device(const Device&);
	Device& operator=(const Device&);
};

CE_EXPORT Device* device();

CE_EXPORT void set_device(Device* device);

} // namespace crown
