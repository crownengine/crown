/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */


#include "config.h"
#include "ceassert.h"
#include "lua_environment.h"
#include "lua_stack.h"
#include "lua_resource.h"
#include <stdarg.h>

namespace crown
{

namespace lua_globals { extern int error_handler(lua_State*); }

LuaEnvironment::LuaEnvironment(lua_State* L)
	: L(L)
{
}

void LuaEnvironment::execute(const LuaResource* lr)
{
	using namespace lua_resource;
	lua_pushcfunction(L, lua_globals::error_handler);
	luaL_loadbuffer(L, program(lr), size(lr), "<unknown>");
	lua_pcall(L, 0, 0, -2);
	lua_pop(L, 1);
}

void LuaEnvironment::execute_string(const char* s)
{
	lua_pushcfunction(L, lua_globals::error_handler);
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

void LuaEnvironment::load_module_enum(const char* module, const char* name, uint32_t value)
{
	// Checks table existance
	lua_pushstring(L, module);
	lua_rawget(L, LUA_GLOBALSINDEX);
	if (!lua_istable(L, -1)) // If not exixts
	{
		// Creates table
		lua_newtable(L);
		lua_setglobal(L, module);
	}

	// Adds field to table
	lua_getglobal(L, module);
	lua_pushinteger(L, value);
	lua_setfield(L, -2, name);
	lua_pop(L, 2);
}

void LuaEnvironment::call_global(const char* func, uint8_t argc, ...)
{
	CE_ASSERT_NOT_NULL(func);

	LuaStack stack(L);

	va_list vl;
	va_start(vl, argc);

	lua_pushcfunction(L, lua_globals::error_handler);
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

void LuaEnvironment::call_physics_callback(Actor* actor_0, Actor* actor_1, Unit* unit_0, Unit* unit_1, const Vector3& where, const Vector3& normal, const char* type)
{
	LuaStack stack(L);

	lua_pushcfunction(L, lua_globals::error_handler);
	lua_getglobal(L, "g_physics_callback");

	stack.push_table();
	stack.push_key_begin("actor_0"); (actor_0 ? stack.push_actor(actor_0) : stack.push_nil()); stack.push_key_end();
	stack.push_key_begin("actor_1"); (actor_1 ? stack.push_actor(actor_1) : stack.push_nil()); stack.push_key_end();
	stack.push_key_begin("unit_0"); (unit_0 ? stack.push_unit(unit_0) : stack.push_nil()); stack.push_key_end();
	stack.push_key_begin("unit_1"); (unit_1 ? stack.push_unit(unit_1) : stack.push_nil()); stack.push_key_end();
	stack.push_key_begin("where"); stack.push_vector3(where); stack.push_key_end();
	stack.push_key_begin("normal"); stack.push_vector3(normal); stack.push_key_end();
	stack.push_key_begin("type"); stack.push_string(type); stack.push_key_end();

	lua_pcall(L, 1, 0, -3);
	lua_pop(L, -1);
}

void LuaEnvironment::call_trigger_callback(Actor* trigger, Actor* other, const char* type)
{
	LuaStack stack(L);

	lua_pushcfunction(L, lua_globals::error_handler);
	lua_getglobal(L, "g_trigger_callback");

	stack.push_table();
	stack.push_key_begin("trigger"); (trigger ? stack.push_actor(trigger) : stack.push_nil()); stack.push_key_end();
	stack.push_key_begin("other"); (other ? stack.push_actor(other) : stack.push_nil()); stack.push_key_end();
	stack.push_key_begin("type"); stack.push_string(type); stack.push_key_end();

	lua_pcall(L, 1, 0, -3);
	lua_pop(L, -1);
}

} // namespace crown
