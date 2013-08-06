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


#include "Device.h"
#include "OS.h"
#include "Assert.h"
#include "Log.h"
#include "LuaEnvironment.h"
#include "StringSetting.h"
#include "Filesystem.h"

namespace crown
{

StringSetting g_boot("boot_file", "lua main file", "lua/game.raw");

/*
*N.B: Lua garbage collection is actually disabled
*/

//-----------------------------------------------------------------------------
LuaEnvironment::LuaEnvironment() :
	m_state(luaL_newstate()),
	m_is_used(false)
	//m_thread(LuaEnvironment::background_thread, (void*)this, "lua-environment-thread")
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

	load_buffer(class_system, string::strlen(class_system));
	execute(0, 0);
	load_buffer(commands_list, string::strlen(commands_list));
	execute(0, 0);
	load_buffer(get_cmd_by_name, string::strlen(get_cmd_by_name));
	execute(0, 0);
	load_buffer(tmp_print_table, string::strlen(tmp_print_table));
	execute(0, 0);
	load_buffer(count_all, string::strlen(count_all));
	execute(0, 0);

	m_is_used = true;
}

//-----------------------------------------------------------------------------
void LuaEnvironment::shutdown()
{
	lua_close(m_state);

	m_is_used = false;
}

//-----------------------------------------------------------------------------
lua_State* LuaEnvironment::state()
{
	return m_state;
}

//-----------------------------------------------------------------------------
const char* LuaEnvironment::error()
{	
	string::strncpy(m_tmp_buffer, m_error_buffer, 1024);

	string::strncpy(m_error_buffer, "", 1024);

	return m_tmp_buffer;
}

//-----------------------------------------------------------------------------
void LuaEnvironment::load_buffer(const char* buffer, size_t len)
{
	int32_t loaded = luaL_loadbuffer(m_state, buffer, len, "");

	if (loaded != 0)
	{
		lua_error();
	}
}

//-----------------------------------------------------------------------------
void LuaEnvironment::load_file(const char* file)
{
	int32_t loaded = luaL_loadfile(m_state, file);

	if (loaded != 0)
	{
		lua_error();
	}
}

//-----------------------------------------------------------------------------
void LuaEnvironment::load_string(const char* str)
{
	int32_t loaded = luaL_loadstring(m_state, str);

	if (loaded != 0)
	{
		lua_error();
	}
}

//-----------------------------------------------------------------------------
void LuaEnvironment::get_global_symbol(const char* symbol)
{
	lua_getglobal(m_state, symbol);
}

//-----------------------------------------------------------------------------
void LuaEnvironment::execute(int32_t args, int32_t results)
{
	int32_t executed = lua_pcall(m_state, args, results, 0);

	if (executed != 0)
	{
		lua_error();
	}
}

// //-----------------------------------------------------------------------------
// void LuaEnvironment::collect_garbage()
// {
// 	uint64_t start = os::milliseconds();

// 	while ((os::milliseconds() - start) < device()->last_delta_time() && !m_is_used)
// 	{
// 		lua_gc(m_state, LUA_GCSTEP, 0);
// 	}
// }

// //-----------------------------------------------------------------------------
// void* LuaEnvironment::background_thread(void* thiz)
// {
// 	((LuaEnvironment*)thiz)->collect_garbage();	
// }

//-----------------------------------------------------------------------------
void LuaEnvironment::game_init()
{
	const char* path = device()->filesystem()->os_path(g_boot.value());

	load_file(path);
	execute(0, 0);

	get_global_symbol("init");
	execute(0, 0);
}

//-----------------------------------------------------------------------------
void LuaEnvironment::game_shutdown()
{
	get_global_symbol("shutdown");
	execute(0, 0);
}

//-----------------------------------------------------------------------------
void LuaEnvironment::game_frame(float dt)
{
	LuaStack stack(m_state);

	get_global_symbol("frame");
	stack.push_float(dt);
	execute(1, 0);
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

//-----------------------------------------------------------------------------
CE_EXPORT int32_t luaopen_libcrown(lua_State* /*L*/)
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

	load_unit(*env);
	load_world(*env);

	return 1;
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