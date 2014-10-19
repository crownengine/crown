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


#include "config.h"
#include "assert.h"
#include "lua_environment.h"
#include "lua_stack.h"
#include "lua_resource.h"
#include <stdarg.h>

namespace crown
{

namespace lua_globals { extern int error_handler(lua_State*); }

LuaEnvironment::LuaEnvironment(lua_State* L)
	: _L(L)
{
}

void LuaEnvironment::execute(const LuaResource* lr)
{
	using namespace lua_resource;
	lua_pushcfunction(_L, lua_globals::error_handler);
	luaL_loadbuffer(_L, program(lr), size(lr), "<unknown>");
	lua_pcall(_L, 0, 0, -2);
	lua_pop(_L, 1);
}

void LuaEnvironment::execute_string(const char* s)
{
	lua_pushcfunction(_L, lua_globals::error_handler);
	luaL_loadstring(_L, s);
	lua_pcall(_L, 0, 0, -2);
	lua_pop(_L, 1);
}

void LuaEnvironment::load_module_function(const char* module, const char* name, const lua_CFunction func)
{
	luaL_newmetatable(_L, module);
	luaL_Reg entry[2];

	entry[0].name = name;
	entry[0].func = func;
	entry[1].name = NULL;
	entry[1].func = NULL;

	luaL_register(_L, NULL, entry);
	lua_setglobal(_L, module);
	lua_pop(_L, -1);
}

void LuaEnvironment::load_module_function(const char* module, const char* name, const char* value)
{
	luaL_newmetatable(_L, module);
	lua_getglobal(_L, value);
	lua_setfield(_L, -2, name);
	lua_setglobal(_L, module);
}

void LuaEnvironment::load_module_constructor(const char* module, const lua_CFunction func)
{
	// Create dummy tables to be used as module's metatable
	lua_createtable(_L, 0, 1);
	lua_pushstring(_L, "__call");
	lua_pushcfunction(_L, func);
	lua_settable(_L, 1); // dummy.__call = func
	lua_getglobal(_L, module);
	lua_pushvalue(_L, -2); // Duplicate dummy metatable
	lua_setmetatable(_L, -2); // setmetatable(module, dummy)
	lua_pop(_L, -1);
}

void LuaEnvironment::load_module_enum(const char* module, const char* name, uint32_t value)
{
	// Checks table existance
	lua_pushstring(_L, module);
	lua_rawget(_L, LUA_GLOBALSINDEX);
	if (!lua_istable(_L, -1)) // If not exixts
	{
		// Creates table
		lua_newtable(_L);
		lua_setglobal(_L, module);
	}

	// Adds field to table
	lua_getglobal(_L, module);
	lua_pushinteger(_L, value);
	lua_setfield(_L, -2, name);
	lua_pop(_L, 2);
}

void LuaEnvironment::call_global(const char* func, uint8_t argc, ...)
{
	CE_ASSERT_NOT_NULL(func);

	LuaStack stack(_L);

	va_list vl;
	va_start(vl, argc);

	lua_pushcfunction(_L, lua_globals::error_handler);
	lua_getglobal(_L, func);

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
	lua_pcall(_L, argc, 0, -argc - 2);
	lua_pop(_L, -1);
}

void LuaEnvironment::call_physics_callback(Actor* actor_0, Actor* actor_1, Unit* unit_0, Unit* unit_1, const Vector3& where, const Vector3& normal, const char* type)
{
	LuaStack stack(_L);

	lua_pushcfunction(_L, lua_globals::error_handler);
	lua_getglobal(_L, "g_physics_callback");

	stack.push_table();
	stack.push_key_begin("actor_0"); (actor_0 ? stack.push_actor(actor_0) : stack.push_nil()); stack.push_key_end();
	stack.push_key_begin("actor_1"); (actor_1 ? stack.push_actor(actor_1) : stack.push_nil()); stack.push_key_end();
	stack.push_key_begin("unit_0"); (unit_0 ? stack.push_unit(unit_0) : stack.push_nil()); stack.push_key_end();
	stack.push_key_begin("unit_1"); (unit_1 ? stack.push_unit(unit_1) : stack.push_nil()); stack.push_key_end();
	stack.push_key_begin("where"); stack.push_vector3(where); stack.push_key_end();
	stack.push_key_begin("normal"); stack.push_vector3(normal); stack.push_key_end();
	stack.push_key_begin("type"); stack.push_string(type); stack.push_key_end();

	lua_pcall(_L, 1, 0, -3);
	lua_pop(_L, -1);
}

void LuaEnvironment::call_trigger_callback(Actor* trigger, Actor* other, const char* type)
{
	LuaStack stack(_L);

	lua_pushcfunction(_L, lua_globals::error_handler);
	lua_getglobal(_L, "g_trigger_callback");

	stack.push_table();
	stack.push_key_begin("trigger"); (trigger ? stack.push_actor(trigger) : stack.push_nil()); stack.push_key_end();
	stack.push_key_begin("other"); (other ? stack.push_actor(other) : stack.push_nil()); stack.push_key_end();
	stack.push_key_begin("type"); stack.push_string(type); stack.push_key_end();

	lua_pcall(_L, 1, 0, -3);
	lua_pop(_L, -1);
}

} // namespace crown
