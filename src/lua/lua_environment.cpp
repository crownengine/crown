/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */


#include "config.h"
#include "device.h"
#include "error.h"
#include "log.h"
#include "lua_environment.h"
#include "lua_resource.h"
#include "lua_stack.h"
#include "resource_manager.h"
#include <stdarg.h>

namespace crown
{

// Lua modules
extern void load_debug_line(LuaEnvironment& env);
extern void load_device(LuaEnvironment& env);
extern void load_gui(LuaEnvironment& env);
extern void load_math(LuaEnvironment& env);
extern void load_physics_world(LuaEnvironment& env);
extern void load_resource_package(LuaEnvironment& env);
extern void load_sound_world(LuaEnvironment& env);
extern void load_raycast(LuaEnvironment& env);
extern void load_resource_package(LuaEnvironment& env);
extern void load_sound_world(LuaEnvironment& env);
extern void load_sprite(LuaEnvironment& env);
extern void load_window(LuaEnvironment& env);
extern void load_world(LuaEnvironment& env);
extern void load_material(LuaEnvironment& env);
extern void load_input(LuaEnvironment& env);
extern void load_render_world(LuaEnvironment& env);

// When an error occurs, logs the error message and pauses the engine.
static int error_handler(lua_State* L)
{
	lua_getfield(L, LUA_GLOBALSINDEX, "debug");
	if (!lua_istable(L, -1))
	{
		lua_pop(L, 1);
		return 0;
	}

	lua_getfield(L, -1, "traceback");
	if (!lua_isfunction(L, -1))
	{
		lua_pop(L, 2);
		return 0;
	}

	lua_pushvalue(L, 1); // Pass error message
	lua_pushinteger(L, 2);
	lua_call(L, 2, 1); // Call debug.traceback

	CE_LOGE(lua_tostring(L, -1)); // Print error message
	lua_pop(L, 1); // Remove error message from stack
	lua_pop(L, 1); // Remove debug.traceback from stack

	device()->pause();
	return 0;
}

// Redirects require to the resource manager.
static int require(lua_State* L)
{
	using namespace lua_resource;
	LuaStack stack(L);
	const LuaResource* lr = (LuaResource*)device()->resource_manager()->get(SCRIPT_TYPE, stack.get_resource_id(1));
	luaL_loadbuffer(L, program(lr), size(lr), "");
	return 1;
}

LuaEnvironment::LuaEnvironment()
	: L(NULL)
	, _vec3_used(0)
	, _quat_used(0)
	, _mat4_used(0)
{
	L = luaL_newstate();
	CE_ASSERT(L, "Unable to create lua state");
}

LuaEnvironment::~LuaEnvironment()
{
	lua_close(L);
}

void LuaEnvironment::load_libs()
{
	lua_gc(L, LUA_GCSTOP, 0);

	// Open default libraries
	luaL_openlibs(L);

	// Register crown libraries
	load_debug_line(*this);
	load_device(*this);
	load_gui(*this);
	load_math(*this);
	load_physics_world(*this);
	load_resource_package(*this);
	load_sound_world(*this);
	load_window(*this);
	load_world(*this);
	load_material(*this);
	load_input(*this);
	load_render_world(*this);

	// Register custom loader
	lua_getfield(L, LUA_GLOBALSINDEX, "package");
	lua_getfield(L, -1, "loaders");
	lua_remove(L, -2);

	int num_loaders = 0;
	lua_pushnil(L);
	while (lua_next(L, -2) != 0)
	{
		lua_pop(L, 1);
		num_loaders++;
	}
	lua_pushinteger(L, num_loaders + 1);
	lua_pushcfunction(L, require);
	lua_rawset(L, -3);
	lua_pop(L, 1);

	// Create metatable for lightuserdata
	lua_pushlightuserdata(L, 0);
	lua_getfield(L, LUA_REGISTRYINDEX, "Lightuserdata_mt");
	lua_setmetatable(L, -2);
	lua_pop(L, 1);

	// Ensure stack is clean
	CE_ASSERT(lua_gettop(L) == 0, "Stack not clean");

	lua_gc(L, LUA_GCRESTART, 0);
}

void LuaEnvironment::execute(const LuaResource* lr)
{
	using namespace lua_resource;
	lua_pushcfunction(L, error_handler);
	luaL_loadbuffer(L, program(lr), size(lr), "<unknown>");
	lua_pcall(L, 0, 0, -2);
	lua_pop(L, 1);
}

void LuaEnvironment::execute_string(const char* s)
{
	lua_pushcfunction(L, error_handler);
	luaL_loadstring(L, s);
	lua_pcall(L, 0, 0, -2);
	lua_pop(L, 1);
}

void LuaEnvironment::load_module_function(const char* module, const char* name, const lua_CFunction func)
{
	luaL_newmetatable(L, module);
	luaL_Reg entry[2];

	entry[0].name = name;
	entry[0].func = func;
	entry[1].name = NULL;
	entry[1].func = NULL;

	luaL_register(L, NULL, entry);
	lua_setglobal(L, module);
	lua_pop(L, -1);
}

void LuaEnvironment::load_module_function(const char* module, const char* name, const char* value)
{
	luaL_newmetatable(L, module);
	lua_getglobal(L, value);
	lua_setfield(L, -2, name);
	lua_setglobal(L, module);
}

void LuaEnvironment::load_module_constructor(const char* module, const lua_CFunction func)
{
	// Create dummy tables to be used as module's metatable
	lua_createtable(L, 0, 1);
	lua_pushstring(L, "__call");
	lua_pushcfunction(L, func);
	lua_settable(L, 1); // dummy.__call = func
	lua_getglobal(L, module);
	lua_pushvalue(L, -2); // Duplicate dummy metatable
	lua_setmetatable(L, -2); // setmetatable(module, dummy)
	lua_pop(L, -1);
}

void LuaEnvironment::call_global(const char* func, uint8_t argc, ...)
{
	CE_ASSERT_NOT_NULL(func);

	LuaStack stack(L);

	va_list vl;
	va_start(vl, argc);

	lua_pushcfunction(L, error_handler);
	lua_getglobal(L, func);

	for (uint8_t i = 0; i < argc; i++)
	{
		const int type = va_arg(vl, int);
		switch (type)
		{
			case ARGUMENT_FLOAT:
			{
				stack.push_float(va_arg(vl, double));
				break;
			}
			default:
			{
				CE_ASSERT(false, "Oops, lua argument unknown");
				break;
			}
		}
	}

	va_end(vl);
	lua_pcall(L, argc, 0, -argc - 2);
	lua_pop(L, -1);
}

Vector3* LuaEnvironment::next_vector3(const Vector3& v)
{
	CE_ASSERT(_vec3_used < CROWN_MAX_LUA_VECTOR3, "Maximum number of Vector3 reached");

	return &(_vec3_buffer[_vec3_used++] = v);
}

Quaternion* LuaEnvironment::next_quaternion(const Quaternion& q)
{
	CE_ASSERT(_quat_used < CROWN_MAX_LUA_QUATERNION, "Maximum number of Quaternion reached");
	return &(_quat_buffer[_quat_used++] = q);
}

Matrix4x4* LuaEnvironment::next_matrix4x4(const Matrix4x4& m)
{
	CE_ASSERT(_mat4_used < CROWN_MAX_LUA_MATRIX4X4, "Maximum number of Matrix4x4 reached");

	return &(s_mat4_buffer[_mat4_used++] = m);
}

bool LuaEnvironment::is_vector3(const Vector3* p)
{
	return (p >= &_vec3_buffer[0] && p <= &_vec3_buffer[CROWN_MAX_LUA_VECTOR3 - 1]);
}

bool LuaEnvironment::is_quaternion(const Quaternion* p)
{
	return (p >= &_quat_buffer[0] && p <= &_quat_buffer[CROWN_MAX_LUA_QUATERNION - 1]);
}

bool LuaEnvironment::is_matrix4x4(const Matrix4x4* p)
{
	return (p >= &s_mat4_buffer[0] && p <= &s_mat4_buffer[CROWN_MAX_LUA_MATRIX4X4 - 1]);
}

void LuaEnvironment::clear_temporaries()
{
	_vec3_used = 0;
	_quat_used = 0;
	_mat4_used = 0;
}

} // namespace crown
