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

#include <stdarg.h>

#include "LuaEnvironment.h"
#include "Assert.h"
#include "StringUtils.h"
#include "LuaStack.h"
#include "Device.h"
#include "LuaResource.h"
#include "ResourceManager.h"
#include "Config.h"

namespace crown
{

namespace lua_system { extern int error_handler(lua_State*); }

//-----------------------------------------------------------------------------
LuaEnvironment::LuaEnvironment(lua_State* L)
	: m_L(L)
{
}

//-----------------------------------------------------------------------------
void LuaEnvironment::load_and_execute(const char* res_name)
{
	ResourceManager* resman = device()->resource_manager();

	// Load the resource
	ResourceId res_id = resman->load("lua", res_name);
	resman->flush();
	LuaResource* lr = (LuaResource*) resman->data(res_id);
	
	lua_pushcfunction(m_L, lua_system::error_handler);
	luaL_loadbuffer(m_L, (const char*) lr->program(), lr->size(), res_name);
	lua_pcall(m_L, 0, 0, -2);

	// Unloading is OK since the script data has been copied to Lua
	resman->unload(res_id);
}

//-----------------------------------------------------------------------------
void LuaEnvironment::execute_string(const char* s)
{
	lua_pushcfunction(m_L, lua_system::error_handler);
	luaL_loadstring(m_L, s);
	lua_pcall(m_L, 0, 0, -2);
}

//-----------------------------------------------------------------------------
void LuaEnvironment::load_module_function(const char* module, const char* name, const lua_CFunction func)
{
	luaL_newmetatable(m_L, module);
	luaL_Reg entry[2];

	entry[0].name = name;
	entry[0].func = func;
	entry[1].name = NULL;
	entry[1].func = NULL;

	luaL_register(m_L, NULL, entry);
	lua_setglobal(m_L, module);
	lua_pop(m_L, -1);
}

//-----------------------------------------------------------------------------
void LuaEnvironment::load_module_function(const char* module, const char* name, const char* value)
{
	luaL_newmetatable(m_L, module);
	lua_getglobal(m_L, value);
	lua_setfield(m_L, -2, name);
	lua_setglobal(m_L, module);
}

//-----------------------------------------------------------------------------
void LuaEnvironment::load_module_constructor(const char* module, const lua_CFunction func)
{
	// Create dummy tables to be used as module's metatable
	lua_createtable(m_L, 0, 1);
	lua_pushstring(m_L, "__call");
	lua_pushcfunction(m_L, func);
	lua_settable(m_L, 1); // dummy.__call = func
	lua_getglobal(m_L, module);
	lua_pushvalue(m_L, -2); // Duplicate dummy metatable
	lua_setmetatable(m_L, -2); // setmetatable(module, dummy)
	lua_pop(m_L, -1);
}

//-----------------------------------------------------------------------------
void LuaEnvironment::load_module_enum(const char* module, const char* name, uint32_t value)
{
	// Checks table existance
	lua_pushstring(m_L, module);
	lua_rawget(m_L, LUA_GLOBALSINDEX);
	if (!lua_istable(m_L, -1)) // If not exixts
	{
		// Creates table
		lua_newtable(m_L);
		lua_setglobal(m_L, module);
	}

	// Adds field to table
	lua_getglobal(m_L, module);
	lua_pushinteger(m_L, value);
	lua_setfield(m_L, -2, name);
}

//-----------------------------------------------------------------------------
void LuaEnvironment::call_global(const char* func, uint8_t argc, ...)
{
	CE_ASSERT_NOT_NULL(func);

	LuaStack stack(m_L);

	va_list vl;
	va_start(vl, argc);

	lua_pushcfunction(m_L, lua_system::error_handler);
	lua_getglobal(m_L, func);

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
	lua_pcall(m_L, argc, 0, -argc - 2);
}

} // namespace crown