/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "config.h"
#include "core/error/error.h"
#include "device/device.h"
#include "device/log.h"
#include "lua/lua_environment.h"
#include "lua/lua_stack.h"
#include "resource/lua_resource.h"
#include "resource/resource_manager.h"
#include <stdarg.h>

namespace { const crown::log_internal::System LUA = { "Lua" }; }

namespace crown
{
extern void load_api(LuaEnvironment& env);

LuaEnvironment::LuaEnvironment()
	: L(NULL)
	, _num_vec3(0)
	, _num_quat(0)
	, _num_mat4(0)
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
	load_api(*this);

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

LuaStack LuaEnvironment::execute(const LuaResource* lr)
{
	LuaStack stack(L);
	lua_pushcfunction(L, LuaEnvironment::error);
	luaL_loadbuffer(L, lua_resource::program(lr), lr->size, "<unknown>");
	lua_pcall(L, 0, 1, -2);
	lua_pop(L, 1);
	return stack;
}

LuaStack LuaEnvironment::execute_string(const char* s)
{
	LuaStack stack(L);
	lua_pushcfunction(L, LuaEnvironment::error);
	luaL_loadstring(L, s);
	lua_pcall(L, 0, 0, -2);
	lua_pop(L, 1);
	return stack;
}

void LuaEnvironment::add_module_function(const char* module, const char* name, const lua_CFunction func)
{
	luaL_Reg entry[2];
	entry[0].name = name;
	entry[0].func = func;
	entry[1].name = NULL;
	entry[1].func = NULL;

	luaL_register(L, module, entry);
	lua_pop(L, 1);
}

void LuaEnvironment::add_module_function(const char* module, const char* name, const char* func)
{
	// Create module if it does not exist
	luaL_Reg entry;
	entry.name = NULL;
	entry.func = NULL;
	luaL_register(L, module, &entry);
	lua_pop(L, 1);

	lua_getglobal(L, module);
	lua_getglobal(L, func);
	lua_setfield(L, -2, name);
	lua_setglobal(L, module);
}

void LuaEnvironment::add_module_metafunction(const char* module, const char* name, const lua_CFunction func)
{
	// Create module if it does not exist
	luaL_Reg entry[2];
	entry[0].name = NULL;
	entry[0].func = NULL;
	luaL_register(L, module, entry);
	lua_pop(L, 1);

	luaL_newmetatable(L, module);
	if (func)
	{
		entry[0].name = name;
		entry[0].func = func;
		entry[1].name = NULL;
		entry[1].func = NULL;
		luaL_register(L, NULL, entry);
	}
	else
	{
		lua_pushstring(L, name);
		lua_pushvalue(L, -2);
		lua_settable(L, -3);
	}

	lua_getglobal(L, module);
	lua_pushvalue(L, -2);
	lua_setmetatable(L, -2);
	lua_pop(L, -1);
}

void LuaEnvironment::call_global(const char* func, u8 argc, ...)
{
	CE_ENSURE(NULL != func);

	LuaStack stack(L);

	va_list vl;
	va_start(vl, argc);

	lua_pushcfunction(L, LuaEnvironment::error);
	lua_getglobal(L, func);

	for (u8 i = 0; i < argc; i++)
	{
		const int type = va_arg(vl, int);
		switch (type)
		{
		case ARGUMENT_FLOAT:
			stack.push_float(va_arg(vl, f64));
			break;

		default:
			CE_FATAL("Unknown argument type");
			break;
		}
	}

	va_end(vl);
	lua_pcall(L, argc, 0, -argc - 2);
	lua_pop(L, -1);

	CE_ASSERT(lua_gettop(L) == 0, "Stack not clean");
}

LuaStack LuaEnvironment::get_global(const char* global)
{
	LuaStack stack(L);
	lua_getglobal(L, global);
	return stack;
}

Vector3* LuaEnvironment::next_vector3(const Vector3& v)
{
	CE_ASSERT(_num_vec3 < LUA_MAX_VECTOR3, "Maximum number of Vector3 reached");
	return &(_vec3[_num_vec3++] = v);
}

Quaternion* LuaEnvironment::next_quaternion(const Quaternion& q)
{
	CE_ASSERT(_num_quat < LUA_MAX_QUATERNION, "Maximum number of Quaternion reached");
	return &(_quat[_num_quat++] = q);
}

Matrix4x4* LuaEnvironment::next_matrix4x4(const Matrix4x4& m)
{
	CE_ASSERT(_num_mat4 < LUA_MAX_MATRIX4X4, "Maximum number of Matrix4x4 reached");
	return &(_mat4[_num_mat4++] = m);
}

bool LuaEnvironment::is_vector3(const Vector3* p) const
{
	return p >= &_vec3[0]
		&& p <= &_vec3[LUA_MAX_VECTOR3 - 1];
}

bool LuaEnvironment::is_quaternion(const Quaternion* p) const
{
	return p >= &_quat[0]
		&& p <= &_quat[LUA_MAX_QUATERNION - 1];
}

bool LuaEnvironment::is_matrix4x4(const Matrix4x4* p) const
{
	return p >= &_mat4[0]
		&& p <= &_mat4[LUA_MAX_MATRIX4X4 - 1];
}

void LuaEnvironment::temp_count(u32& num_vec3, u32& num_quat, u32& num_mat4)
{
	num_vec3 = _num_vec3;
	num_quat = _num_quat;
	num_mat4 = _num_mat4;
}

void LuaEnvironment::set_temp_count(u32 num_vec3, u32 num_quat, u32 num_mat4)
{
	_num_vec3 = num_vec3;
	_num_quat = num_quat;
	_num_mat4 = num_mat4;
}

void LuaEnvironment::reset_temporaries()
{
	_num_vec3 = 0;
	_num_quat = 0;
	_num_mat4 = 0;
}

int LuaEnvironment::error(lua_State* L)
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

	loge(LUA, lua_tostring(L, -1)); // Print error message
	lua_pop(L, 1); // Remove error message from stack
	lua_pop(L, 1); // Remove debug.traceback from stack

	device()->pause();
	return 0;
}

int LuaEnvironment::require(lua_State* L)
{
	LuaStack stack(L);
	const LuaResource* lr = (LuaResource*)device()->_resource_manager->get(RESOURCE_TYPE_SCRIPT, stack.get_resource_id(1));
	luaL_loadbuffer(L, lua_resource::program(lr), lr->size, "");
	return 1;
}

} // namespace crown
