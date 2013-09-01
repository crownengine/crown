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

#define MAX_SUBSYSTEMS_HEAP 1024 * 1024

namespace crown
{

class Filesystem;
class ResourceManager;
class OsWindow;
class Bundle;
class Renderer;
class DebugRenderer;
class InputManager;
class Keyboard;
class Mouse;
class Touch;
class Accelerometer;
class LuaEnvironment;
class ConsoleServer;
class BundleCompiler;

/// The Engine.
/// It is the place where to look for accessing all of
/// the engine subsystems and related stuff.
class CE_EXPORT Device
{
public:

							Device();
							~Device();

	/// Initializes the engine allowing to pass command line
	/// parameters to configure some parameters.
	bool					init(int argc, char** argv);

	/// Shutdowns the engine freeing all the allocated resources
	void					shutdown();

	/// Returns wheter the engine is running (i.e. it is actually
	/// doing work).
	bool					is_running() const;

	/// Returns whether the engine is correctly initialized
	bool					is_init() const;

	/// Return the number of frames rendered from the first
	/// call to Device::start()
	uint64_t				frame_count() const;

	/// Returns the time in milliseconds needed to render
	/// the last frame
	float					last_delta_time() const;

	/// Forces the engine to actually start doing work.
	void					start();

	/// Forces the engine to stop all the work it is doing
	/// and normally terminates the program.
	void					stop();

	/// Updates all the subsystems
	void					frame();

	void					compile(const char* bundle_dir, const char* source_dir, const char* resource);

	void					reload(ResourceId name);

	Filesystem*				filesystem();
	ResourceManager*		resource_manager();
	InputManager*			input_manager();
	LuaEnvironment*			lua_environment();

	OsWindow*				window();
	Renderer*				renderer();
	DebugRenderer*			debug_renderer();

	Keyboard*				keyboard();
	Mouse*					mouse();
	Touch*					touch();
	Accelerometer*			accelerometer();

	ConsoleServer*			console_server();

private:

	void					parse_command_line(int argc, char** argv);
	void					check_preferred_settings();
	void					read_engine_settings();
	void					print_help_message();

private:

	// Used to allocate all subsystems
	uint8_t					m_subsystems_heap[MAX_SUBSYSTEMS_HEAP];
	LinearAllocator			m_allocator;

	// Preferred settings
	int32_t					m_preferred_window_width;
	int32_t					m_preferred_window_height;
	int32_t					m_preferred_window_fullscreen;
	uint32_t				m_parent_window_handle;
	char					m_source_dir[MAX_PATH_LENGTH];
	char 					m_bundle_dir[MAX_PATH_LENGTH];
	char 					m_boot_file[MAX_PATH_LENGTH];
	int32_t					m_compile;
	int32_t					m_continue;

	int32_t					m_quit_after_init;

	bool					m_is_init		: 1;
	bool					m_is_running	: 1;

	uint64_t				m_frame_count;

	uint64_t				m_last_time;
	uint64_t				m_current_time;
	float					m_last_delta_time;

	// Public subsystems
	Filesystem*				m_filesystem;

	OsWindow*				m_window;
	InputManager*			m_input_manager;
	LuaEnvironment*			m_lua_environment;
	Renderer*				m_renderer;
	DebugRenderer*			m_debug_renderer;

	// Private subsystems
	BundleCompiler*			m_bundle_compiler;
	ResourceManager*		m_resource_manager;
	Bundle*					m_resource_bundle;

	// Debug subsystems
	ConsoleServer*			m_console_server;

private:

	// Disable copying
	Device(const Device&);
	Device& operator=(const Device&);
};

CE_EXPORT Device* device();

} // namespace crown

