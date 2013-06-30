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

	void			init();

	void			shutdown();

	void			load_buffer(const char* buffer, size_t len);

	void			load_file(const char* file);

	void 			load_string(const char* str);

	void			get_global_symbol(const char* symbol);

	void			execute(int32_t args, int32_t results);

	const char*		lua_error();


//-----------------------------------------------------------------------------

	/// Load a function to proper module
	void			load_module_function(const char* module, const char* name, const lua_CFunction func);

private:

	LuaStack		m_stack;

	// Disable copying
					LuaEnvironment(const LuaEnvironment&);
	LuaEnvironment& operator=(const LuaEnvironment&);
};

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

CE_EXPORT int32_t luaopen_libcrown(lua_State* L);

} // namespace crown