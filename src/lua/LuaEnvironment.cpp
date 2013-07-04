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

namespace crown
{

//-----------------------------------------------------------------------------
LuaEnvironment::LuaEnvironment(lua_State* L) :
	m_stack(L)
{
	// Open Lua default libraries
	luaL_openlibs(m_stack.state());
}

//-----------------------------------------------------------------------------
void LuaEnvironment::init()
{
	// Open Crown library
	lua_cpcall(m_stack.state(), luaopen_libcrown, NULL);
}

//-----------------------------------------------------------------------------
void LuaEnvironment::shutdown()
{
	lua_close(m_stack.state());
}

//-----------------------------------------------------------------------------
LuaStack LuaEnvironment::stack()
{
	return m_stack;
}

//-----------------------------------------------------------------------------
void LuaEnvironment::load_buffer(const char* buffer, size_t len)
{
	luaL_loadbuffer(m_stack.state(), buffer, len, "");
}

//-----------------------------------------------------------------------------
void LuaEnvironment::load_file(const char* file)
{
	luaL_loadfile(m_stack.state(), file);
}

//-----------------------------------------------------------------------------
void LuaEnvironment::load_string(const char* str)
{
	luaL_loadstring(m_stack.state(), str);
}

//-----------------------------------------------------------------------------
void LuaEnvironment::get_global_symbol(const char* symbol)
{
	lua_getglobal(m_stack.state(), symbol);
}

//-----------------------------------------------------------------------------
void LuaEnvironment::execute(int32_t args, int32_t results)
{
	lua_pcall(m_stack.state(), args, results, 0);
}

//-----------------------------------------------------------------------------
void LuaEnvironment::load_module_function(const char* module, const char* name, const lua_CFunction func)
{
	luaL_Reg entry[2];

	entry[0].name = name;
	entry[0].func = func;

	entry[1].name = NULL;
	entry[1].func = NULL;

	luaL_register(m_stack.state(), module, entry);
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t luaopen_libcrown(lua_State* L)
{
	LuaEnvironment* env = device()->lua_environment();

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
