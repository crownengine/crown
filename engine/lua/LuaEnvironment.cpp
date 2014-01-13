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

namespace crown
{

extern int vector2_add(lua_State* L);
extern int vector2_subtract(lua_State* L);
extern int vector2_multiply(lua_State* L);
extern int vector2_divide(lua_State* L);
extern int vector2_negate(lua_State* L);
extern int vector3_add(lua_State* L);
extern int vector3_subtract(lua_State* L);
extern int vector3_multiply(lua_State* L);
extern int vector3_divide(lua_State* L);
extern int vector3_negate(lua_State* L);
extern int vector2(lua_State* L);
extern int vector3(lua_State* L);
extern int matrix4x4(lua_State* L);
extern int quaternion(lua_State* L);

//-----------------------------------------------------------------------------
CE_EXPORT int luaopen_libcrown(lua_State* /*L*/)
{
	LuaEnvironment* env = device()->lua_environment();

	load_int_setting(*env);
	load_float_setting(*env);
	load_string_setting(*env);

	load_vector2(*env);
	load_vector3(*env);
	load_matrix4x4(*env);
	load_quaternion(*env);
	load_math(*env);
	load_window(*env);
	load_mouse(*env);
	load_keyboard(*env);
	load_touch(*env);
	load_accelerometer(*env);
	load_device(*env);
	load_resource_package(*env);

	load_unit(*env);
	load_camera(*env);
	load_world(*env);
	load_mesh(*env);
	load_sprite(*env);
	load_actor(*env);
	load_controller(*env);
	load_physics_world(*env);

	load_gui(*env);

	return 1;
}

//-----------------------------------------------------------------------------
static int crown_lua_vector2_call(lua_State* L)
{
	lua_remove(L, 1);
	return vector2(L);
}

//-----------------------------------------------------------------------------
static int crown_lua_vector3_call(lua_State* L)
{
	lua_remove(L, 1);
	return vector3(L);
}

//-----------------------------------------------------------------------------
static int crown_lua_matrix4x4_call(lua_State* L)
{
	lua_remove(L, 1);
	return matrix4x4(L);
}

//-----------------------------------------------------------------------------
static int crown_lua_quaternion_call(lua_State* L)
{
	lua_remove(L, 1);
	return quaternion(L);
}

//-----------------------------------------------------------------------------
static int crown_lua_lightuserdata_add(lua_State* L)
{
	LuaStack stack(L);

	if (stack.is_vector3(1))
	{
		return vector3_add(L);
	}
	return vector2_add(L);
}

//-----------------------------------------------------------------------------
static int crown_lua_lightuserdata_sub(lua_State* L)
{
	LuaStack stack(L);

	if (stack.is_vector3(1))
	{
		return vector3_subtract(L);
	}
	return vector2_subtract(L);
}

//-----------------------------------------------------------------------------
static int crown_lua_lightuserdata_mul(lua_State* L)
{
	LuaStack stack(L);

	if (stack.is_vector3(1))
	{
		return vector3_multiply(L);
	}
	return vector2_multiply(L);
}

//-----------------------------------------------------------------------------
static int crown_lua_lightuserdata_div(lua_State* L)
{
	LuaStack stack(L);

	if (stack.is_vector3(1))
	{
		return vector3_divide(L);
	}
	return vector2_divide(L);
}

//-----------------------------------------------------------------------------
static int crown_lua_lightuserdata_unm(lua_State* L)
{
	LuaStack stack(L);

	if (stack.is_vector3(1))
	{
		return vector3_negate(L);
	}
	return vector2_negate(L);
}

//-----------------------------------------------------------------------------
static int crown_lua_require(lua_State* L)
{
	LuaStack stack(L);

	const char* filename = stack.get_string(1);

	const ResourceId lua_res = device()->resource_manager()->load("lua", filename);
	device()->resource_manager()->flush();

	const LuaResource* lr = (LuaResource*) device()->resource_manager()->data(lua_res);
	luaL_loadbuffer(L, (const char*) lr->program(), lr->size(), "");

	device()->resource_manager()->unload(lua_res);

	return 1;
}

//-----------------------------------------------------------------------------
LuaEnvironment::LuaEnvironment()
	: m_state(luaL_newstate())
{
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

	// Create metatable for lightuserdata
	lua_pushlightuserdata(m_state, (void*)0x0); // Just dummy userdata
	luaL_newmetatable(m_state, "Lightuserdata_mt");
	lua_setmetatable(m_state, -2);
	lua_pop(m_state, 1);

	luaL_newmetatable(m_state, "Lightuserdata_mt");

	lua_pushstring(m_state, "__add");
	lua_pushcfunction(m_state, crown_lua_lightuserdata_add);
	lua_settable(m_state, 1);

	lua_pushstring(m_state, "__sub");
	lua_pushcfunction(m_state, crown_lua_lightuserdata_sub);
	lua_settable(m_state, 1);

	lua_pushstring(m_state, "__mul");
	lua_pushcfunction(m_state, crown_lua_lightuserdata_mul);
	lua_settable(m_state, 1);

	lua_pushstring(m_state, "__div");
	lua_pushcfunction(m_state, crown_lua_lightuserdata_div);
	lua_settable(m_state, 1);

	lua_pushstring(m_state, "__unm");
	lua_pushcfunction(m_state, crown_lua_lightuserdata_unm);
	lua_settable(m_state, 1);

	// Vector2 metatable
	luaL_newmetatable(m_state, "Vector2_mt");
	lua_pushvalue(m_state, -1);
	lua_setfield(m_state, -2, "__index");
	lua_pushcfunction(m_state, crown_lua_vector2_call);
	lua_setfield(m_state, -2, "__call");

	lua_getglobal(m_state, "Vector2");
	lua_pushvalue(m_state, -2); // Duplicate metatable
	lua_setmetatable(m_state, -2); // setmetatable(Vector2, Vector2_mt)

	// Vector3 metatable
	luaL_newmetatable(m_state, "Vector3_mt");
	lua_pushvalue(m_state, -1);
	lua_setfield(m_state, -2, "__index");
	lua_pushcfunction(m_state, crown_lua_vector3_call);
	lua_setfield(m_state, -2, "__call");

	lua_getglobal(m_state, "Vector3");
	lua_pushvalue(m_state, -2); // Duplicate metatable
	lua_setmetatable(m_state, -2); // setmetatable(Vector3, Vector3_mt)

	// Matrix4x4 metatable
	luaL_newmetatable(m_state, "Matrix4x4_mt");
	lua_pushvalue(m_state, -1);
	lua_setfield(m_state, -2, "__index");
	lua_pushcfunction(m_state, crown_lua_matrix4x4_call);
	lua_setfield(m_state, -2, "__call");

	lua_getglobal(m_state, "Matrix4x4");
	lua_pushvalue(m_state, -2); // Duplicate metatable
	lua_setmetatable(m_state, -2); // setmetatable(Matrix4x4, Matrix4x4_mt)

	// Quaternion metatable
	luaL_newmetatable(m_state, "Quaternion_mt");
	lua_pushvalue(m_state, -1);
	lua_setfield(m_state, -2, "__index");
	lua_pushcfunction(m_state, crown_lua_quaternion_call);
	lua_setfield(m_state, -2, "__call");

	lua_getglobal(m_state, "Quaternion");
	lua_pushvalue(m_state, -2); // Duplicate metatable
	lua_setmetatable(m_state, -2); // setmetatable(Quaternion, Quaternion_mt)
}

//-----------------------------------------------------------------------------
void LuaEnvironment::shutdown()
{
	lua_close(m_state);
}

//-----------------------------------------------------------------------------
bool LuaEnvironment::load_and_execute(const char* res_name)
{
	CE_ASSERT_NOT_NULL(res_name);

	ResourceManager* resman = device()->resource_manager();

	// Load the resource
	ResourceId res_id = resman->load("lua", res_name);
	resman->flush();
	LuaResource* lr = (LuaResource*) resman->data(res_id);
	
	lua_getglobal(m_state, "debug");
	lua_getfield(m_state, -1, "traceback");
	if (luaL_loadbuffer(m_state, (const char*) lr->program(), lr->size(), res_name) == 0)
	{
		if (lua_pcall(m_state, 0, 0, -2) == 0)
		{
			// Unloading is OK since the script data has been copied to Lua
			resman->unload(res_id);
			lua_pop(m_state, 2); // Pop debug.traceback
			return true;
		}
	}

	error();
	lua_pop(m_state, 2); // Pop debug.traceback
	return false;
}

//-----------------------------------------------------------------------------
bool LuaEnvironment::execute_string(const char* s)
{
	CE_ASSERT_NOT_NULL(s);

	lua_getglobal(m_state, "debug");
	lua_getfield(m_state, -1, "traceback");
	if (luaL_loadstring(m_state, s) == 0)
	{
		if (lua_pcall(m_state, 0, 0, -2) == 0)
		{
			// Unloading is OK since the script data has been copied to Lua
			lua_pop(m_state, 2); // Pop debug.traceback
			return true;
		}
	}

	error();
	lua_pop(m_state, 2); // Pop debug.traceback
	return false;
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
bool LuaEnvironment::call_global(const char* func, uint8_t argc, ...)
{
	CE_ASSERT_NOT_NULL(func);

	LuaStack stack(m_state);

	va_list vl;
	va_start(vl, argc);

	lua_getglobal(m_state, "debug");
	lua_getfield(m_state, -1, "traceback");
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

	if (lua_pcall(m_state, argc, 0, -argc - 2) != 0)
	{
		error();
		lua_pop(m_state, 2); // Pop debug.traceback
		return false;
	}

	lua_pop(m_state, 2); // Pop debug.traceback

	return true;
}

//-----------------------------------------------------------------------------
void LuaEnvironment::error()
{
	const char* msg = lua_tostring(m_state, -1);
	Log::e(msg);
	lua_pop(m_state, 1);
}

} // namespace crown