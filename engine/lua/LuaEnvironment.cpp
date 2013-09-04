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
#include "Assert.h"
#include "LuaEnvironment.h"
#include "StringUtils.h"
#include "LuaStack.h"
#include "Device.h"
#include "LuaResource.h"
#include "ResourceManager.h"

namespace crown
{

//-----------------------------------------------------------------------------
CE_EXPORT int luaopen_libcrown(lua_State* /*L*/)
{
	LuaEnvironment* env = device()->lua_environment();

	load_int_setting(*env);
	load_float_setting(*env);
	load_string_setting(*env);
	load_vec2(*env);
	load_vec3(*env);
	load_mat4(*env);
	load_quat(*env);
	load_math(*env);
	load_mouse(*env);
	load_keyboard(*env);
	load_accelerometer(*env);
	load_device(*env);
	load_window(*env);
	load_resource_package(*env);

	return 1;
}

//-----------------------------------------------------------------------------
static int crown_lua_require(lua_State* L)
{
	LuaStack stack(L);

	const char* filename = stack.get_string(1);

	const ResourceId lua_res = device()->resource_manager()->load("lua", filename);
	device()->resource_manager()->flush();

	const LuaResource* lr = (LuaResource*) device()->resource_manager()->data(lua_res);
	luaL_loadbuffer(L, (const char*) lr->code(), lr->size(), "");

	device()->resource_manager()->unload(lua_res);

	return 1;
}

//-----------------------------------------------------------------------------
LuaEnvironment::LuaEnvironment() :
	m_state(luaL_newstate()),
	m_is_used(false)
{
	// Open Lua default libraries
	string::strncpy(m_error_buffer, "", 1024);
	string::strncpy(m_tmp_buffer, "", 1024);
}

//-----------------------------------------------------------------------------
void LuaEnvironment::init()
{
	// Open default libraries
	luaL_openlibs(m_state);
	// Open Crown library
	lua_cpcall(m_state, luaopen_libcrown, NULL);

	// Register custom loader
	lua_getfield(m_state, LUA_GLOBALSINDEX, "package");
	lua_getfield(m_state, -1, "loaders");
	lua_remove(m_state, -2);

	int num_loaders = 0;
	lua_pushnil(m_state);
	while (lua_next(m_state, -2) != 0)
	{
		lua_pop(m_state, 1);
		num_loaders++;
	}

	lua_pushinteger(m_state, num_loaders + 1);
	lua_pushcfunction(m_state, crown_lua_require);
	lua_rawset(m_state, -3);

	lua_pop(m_state, 1);

	// load_buffer(class_system, string::strlen(class_system));
	// execute(0, 0);
	// load_buffer(commands_list, string::strlen(commands_list));
	// execute(0, 0);
	// load_buffer(get_cmd_by_name, string::strlen(get_cmd_by_name));
	// execute(0, 0);
	// load_buffer(tmp_print_table, string::strlen(tmp_print_table));
	// execute(0, 0);
	// load_buffer(count_all, string::strlen(count_all));
	// execute(0, 0);

	m_is_used = true;
}

//-----------------------------------------------------------------------------
void LuaEnvironment::shutdown()
{
	lua_close(m_state);

	m_is_used = false;
}

//-----------------------------------------------------------------------------
const char* LuaEnvironment::error()
{	
	string::strncpy(m_tmp_buffer, m_error_buffer, 1024);
	string::strncpy(m_error_buffer, "", 1024);

	return m_tmp_buffer;
}

//-----------------------------------------------------------------------------
void LuaEnvironment::load(const LuaResource* lr)
{
	CE_ASSERT_NOT_NULL(lr);

	if (luaL_loadbuffer(m_state, (const char*) lr->code(), lr->size(), "") != 0)
	{
		lua_error();
	}

	if (lua_pcall(m_state, 0, 0, 0) != 0)
	{
		lua_error();
	}
}

//-----------------------------------------------------------------------------
void LuaEnvironment::call_global(const char* func, uint8_t argc, ...)
{
	CE_ASSERT_NOT_NULL(func);

	LuaStack stack(m_state);

	va_list vl;
	va_start(vl, argc);

	lua_getglobal(m_state, func);

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

	if (lua_pcall(m_state, argc, 0, 0) != 0)
	{
		lua_error();
	}
}

//-----------------------------------------------------------------------------
void LuaEnvironment::lua_error()
{
	string::strncpy(m_error_buffer, "", 1024);
	string::strncpy(m_error_buffer, lua_tostring(m_state, -1), 1024);
}

//-----------------------------------------------------------------------------
void LuaEnvironment::load_module_function(const char* module, const char* name, const lua_CFunction func)
{
	luaL_Reg entry[2];

	entry[0].name = name;
	entry[0].func = func;
	entry[1].name = NULL;
	entry[1].func = NULL;

	luaL_register(m_state, module, entry);
}

//-----------------------------------------------------------
void LuaEnvironment::load_module_enum(const char* /*module*/, const char* name, uint32_t value)
{
	lua_pushinteger(m_state, value);
	lua_setfield(m_state, -2, name);
}

const char* LuaEnvironment::class_system =  "function class(klass, super) "
    										"	if not klass then "
        									"		klass = {} "
                							"		local meta = {} "
        									"		meta.__call = function(self, ...) "
            								"			local object = {} "
            								"			setmetatable(object, klass) "
            								"			if object.init then object:init(...) end "
            								"			return object "
       										"		end "
        									"		setmetatable(klass, meta) "
    										"	end "  
    										"	if super then "
        									"		for k,v in pairs(super) do "
            								"			klass[k] = v "
        									"		end "
    										"	end "
    										"	klass.__index = klass "
    										"	return klass "
											"end";


const char* LuaEnvironment::commands_list = "function get_all_commands() "
											"	local cmds = {}; "
											"	for class_name,class in pairs(_G) do "
											"		if type(class) == 'table' then "
			 								"			for func_name,func in pairs(class) do "
			 								"				if type(func) == 'function' then "
											"					cmds[#cmds+1] = class_name .. '.' .. func_name "
											"				end "
											"			end "
											"		end "
											"	end "
											"	return cmds "
											"end";

const char* LuaEnvironment::get_cmd_by_name = 	"function get_command_by_name(text) "
												"	local cmds = get_all_commands() "
												"	local results = {} "
												"	local index = 0 "
												"	for i,cmd in pairs(cmds) do "
												"		if string.find(cmd, text) then "
												"			results[index] = cmds[i] "
												"			index = index + 1 "
												"		end "
												"	end "
												"	return results "
												"end";

const char* LuaEnvironment::tmp_print_table =	"function print_table(table) "												
												"	for k,v in pairs(table) do "
												"		print(v) "
												"	end "
												"end";

const char* LuaEnvironment::count_all = 	"function count_all(f) "
											"	local seen = {} "
											"	local count_table "
											"	count_table = function(t) "
											"		if seen[t] then return end "
											"		f(t) "
											"		seen[t] = true "
											"		for k,v in pairs(t) do "
											"			if type(v) == 'table' then "
											"				count_table(v) "
											"			elseif type(v) == 'userdata' then "
											"				f(v) "
											"			end "
											"		end "
											"	end "
											"	count_table(_G) "
											"end";

} // namespace crown