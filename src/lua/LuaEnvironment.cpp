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
#include "Assert.h"
#include "Log.h"
#include "LuaEnvironment.h"
#include "StringSetting.h"
#include "Filesystem.h"

namespace crown
{

StringSetting g_boot("boot_file", "lua main file", "lua/game.raw");

//-----------------------------------------------------------------------------
LuaEnvironment::LuaEnvironment() :
	m_state(luaL_newstate())
{
	// Open Lua default libraries
	luaL_openlibs(m_state);

	string::strncpy(m_error_buffer, "", 1024);

	string::strncpy(m_tmp_buffer, "", 1024);
}

//-----------------------------------------------------------------------------
void LuaEnvironment::start()
{
	// Open Crown library
	lua_cpcall(m_state, luaopen_libcrown, NULL);
}

//-----------------------------------------------------------------------------
void LuaEnvironment::stop()
{
	lua_close(m_state);
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

//-----------------------------------------------------------------------------
void LuaEnvironment::init()
{
	const char* path = device()->filesystem()->os_path(g_boot.value());

	load_file(path);
	execute(0, 0);

	get_global_symbol("init");
	execute(0, 0);
}

//-----------------------------------------------------------------------------
void LuaEnvironment::shutdown()
{
	get_global_symbol("shutdown");
	execute(0, 0);
}

//-----------------------------------------------------------------------------
void LuaEnvironment::frame(float dt)
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

//-----------------------------------------------------------------------------
CE_EXPORT int32_t luaopen_libcrown(lua_State* L)
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

	return 1;
}

} // namespace crown