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

#include "lua.hpp"
#include "Types.h"
#include "Config.h"
#include "LuaStack.h"

#ifdef WINDOWS
	#define CE_EXPORT extern "C" __declspec(dllexport)
#else
	#define CE_EXPORT
#endif


namespace crown
{

/// LuaEnvironment is a wrapper of a subset of Lua functions and 
/// provides utilities for extending Lua
class LuaEnvironment
{

public:
	/// Constructor
					LuaEnvironment(lua_State* L);

	LuaStack		stack();

	const char*		error();

	void			init();

	void			shutdown();

	void			load_buffer(const char* buffer, size_t len);

	void			load_file(const char* file);

	void 			load_string(const char* str);

	void			get_global_symbol(const char* symbol);

	void			execute(int32_t args, int32_t results);

//-----------------------------------------------------------------------------

	/// Load a function to proper module
	void			load_module_function(const char* module, const char* name, const lua_CFunction func);

private:

	void			lua_error();
	// Disable copying
					LuaEnvironment(const LuaEnvironment&);
	LuaEnvironment& operator=(const LuaEnvironment&);

private:

	LuaStack		m_stack;

	char			m_error_buffer[1024];

	char			m_tmp_buffer[1024];
};

void load_int_setting(LuaEnvironment& env);
void load_float_setting(LuaEnvironment& env);
void load_string_setting(LuaEnvironment& env);

void load_vec2(LuaEnvironment& env);
void load_vec3(LuaEnvironment& env);
void load_mat4(LuaEnvironment& env);
void load_quat(LuaEnvironment& env);
void load_math(LuaEnvironment& env);
void load_mouse(LuaEnvironment& env);
void load_keyboard(LuaEnvironment& env);
void load_touch(LuaEnvironment& env);
void load_accelerometer(LuaEnvironment& env);
void load_camera(LuaEnvironment& env);
void load_device(LuaEnvironment& env);
void load_window(LuaEnvironment& env);

CE_EXPORT int32_t luaopen_libcrown(lua_State* L);

} // namespace crown