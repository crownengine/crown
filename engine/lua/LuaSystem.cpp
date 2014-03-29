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

#include "LuaSystem.h"
#include "lua.hpp"
#include "Config.h"
#include "LuaStack.h"
#include "Assert.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Matrix4x4.h"
#include "Quaternion.h"
#include "Device.h"
#include "ResourceManager.h"
#include "LuaResource.h"
#include "LuaEnvironment.h"

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
extern int vector2_new(lua_State* L);
extern int vector3_new(lua_State* L);
extern int matrix4x4(lua_State* L);
extern int quaternion_new(lua_State* L);

//-----------------------------------------------------------------------------
static int crown_lua_vector2_call(lua_State* L)
{
	lua_remove(L, 1);
	return vector2_new(L);
}

//-----------------------------------------------------------------------------
static int crown_lua_vector3_call(lua_State* L)
{
	lua_remove(L, 1);
	return vector3_new(L);
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
	return quaternion_new(L);
}

//-----------------------------------------------------------------------------
static int crown_lua_lightuserdata_add(lua_State* L)
{
	LuaStack stack(L);

	if (lua_system::is_vector3(1))
	{
		return vector3_add(L);
	}
	return vector2_add(L);
}

//-----------------------------------------------------------------------------
static int crown_lua_lightuserdata_sub(lua_State* L)
{
	LuaStack stack(L);

	if (lua_system::is_vector3(1))
	{
		return vector3_subtract(L);
	}
	return vector2_subtract(L);
}

//-----------------------------------------------------------------------------
static int crown_lua_lightuserdata_mul(lua_State* L)
{
	LuaStack stack(L);

	if (lua_system::is_vector3(1))
	{
		return vector3_multiply(L);
	}
	return vector2_multiply(L);
}

//-----------------------------------------------------------------------------
static int crown_lua_lightuserdata_div(lua_State* L)
{
	LuaStack stack(L);

	if (lua_system::is_vector3(1))
	{
		return vector3_divide(L);
	}
	return vector2_divide(L);
}

//-----------------------------------------------------------------------------
static int crown_lua_lightuserdata_unm(lua_State* L)
{
	LuaStack stack(L);

	if (lua_system::is_vector3(1))
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

namespace lua_system
{
	static lua_State* s_L;

	static uint32_t 		s_vec2_used = 0;
	static Vector2 			s_vec2_buffer[CE_MAX_LUA_VECTOR2];
	static uint32_t 		s_vec3_used = 0;
	static Vector3 			s_vec3_buffer[CE_MAX_LUA_VECTOR3];
	static uint32_t 		s_mat4_used = 0;
	static Matrix4x4 		s_mat4_buffer[CE_MAX_LUA_MATRIX4X4];
	static uint32_t 		s_quat_used = 0;
	static Quaternion 		s_quat_buffer[CE_MAX_LUA_QUATERNION];

	void init()
	{
		s_L = luaL_newstate();
		CE_ASSERT(s_L, "Unable to create lua state");

		// Open default libraries
		luaL_openlibs(s_L);

		// Register crown libraries
		LuaEnvironment env(s_L);

		load_int_setting(env);
		load_float_setting(env);
		load_string_setting(env);
		load_vector2(env);
		load_vector3(env);
		load_matrix4x4(env);
		load_quaternion(env);
		load_math(env);
		load_window(env);
		load_mouse(env);
		load_keyboard(env);
		load_touch(env);
		load_accelerometer(env);
		load_device(env);
		load_resource_package(env);
		load_unit(env);
		load_camera(env);
		load_world(env);
		load_mesh(env);
		load_sprite(env);
		load_actor(env);
		load_controller(env);
		load_physics_world(env);
		load_sound_world(env);
		load_gui(env);
		load_debug_line(env);
		load_raycast(env);

		// Register custom loader
		lua_getfield(s_L, LUA_GLOBALSINDEX, "package");
		lua_getfield(s_L, -1, "loaders");
		lua_remove(s_L, -2);

		int num_loaders = 0;
		lua_pushnil(s_L);
		while (lua_next(s_L, -2) != 0)
		{
			lua_pop(s_L, 1);
			num_loaders++;
		}

		lua_pushinteger(s_L, num_loaders + 1);
		lua_pushcfunction(s_L, crown_lua_require);
		lua_rawset(s_L, -3);
		lua_pop(s_L, 1);

		// Create metatable for lightuserdata
		lua_pushlightuserdata(s_L, (void*)0x0); // Just dummy userdata
		luaL_newmetatable(s_L, "Lightuserdata_mt");
		lua_setmetatable(s_L, -2);
		lua_pop(s_L, -1);

		luaL_newmetatable(s_L, "Lightuserdata_mt");

		lua_pushstring(s_L, "__add");
		lua_pushcfunction(s_L, crown_lua_lightuserdata_add);
		lua_settable(s_L, 1);

		lua_pushstring(s_L, "__sub");
		lua_pushcfunction(s_L, crown_lua_lightuserdata_sub);
		lua_settable(s_L, 1);

		lua_pushstring(s_L, "__mul");
		lua_pushcfunction(s_L, crown_lua_lightuserdata_mul);
		lua_settable(s_L, 1);

		lua_pushstring(s_L, "__div");
		lua_pushcfunction(s_L, crown_lua_lightuserdata_div);
		lua_settable(s_L, 1);

		lua_pushstring(s_L, "__unm");
		lua_pushcfunction(s_L, crown_lua_lightuserdata_unm);
		lua_settable(s_L, 1);

		//lua_pop(s_L, -1); // pop Lightuserdata_mt

		Log::d("Stacksize = %d", lua_gettop(s_L));

		// Vector2 metatable
		luaL_newmetatable(s_L, "Vector2_mt");
		lua_pushvalue(s_L, -1);
		lua_setfield(s_L, -2, "__index");
		lua_pushcfunction(s_L, crown_lua_vector2_call);
		lua_setfield(s_L, -2, "__call");
		//lua_pop(s_L, -1); // pop Vector2_mt

		lua_getglobal(s_L, "Vector2");
		lua_pushvalue(s_L, -2); // Duplicate metatable
		lua_setmetatable(s_L, -2); // setmetatable(Vector2, Vector2_mt)
		//lua_pop(s_L, -1); // pop Vector2

		// Vector3 metatable
		luaL_newmetatable(s_L, "Vector3_mt");
		lua_pushvalue(s_L, -1);
		lua_setfield(s_L, -2, "__index");
		lua_pushcfunction(s_L, crown_lua_vector3_call);
		lua_setfield(s_L, -2, "__call");
		//lua_pop(s_L, -1);

		lua_getglobal(s_L, "Vector3");
		lua_pushvalue(s_L, -2); // Duplicate metatable
		lua_setmetatable(s_L, -2); // setmetatable(Vector3, Vector3_mt)
		//lua_pop(s_L, -1);

		// Matrix4x4 metatable
		luaL_newmetatable(s_L, "Matrix4x4_mt");
		lua_pushvalue(s_L, -1);
		lua_setfield(s_L, -2, "__index");
		lua_pushcfunction(s_L, crown_lua_matrix4x4_call);
		lua_setfield(s_L, -2, "__call");
		lua_pop(s_L, -1);

		lua_getglobal(s_L, "Matrix4x4");
		lua_pushvalue(s_L, -2); // Duplicate metatable
		lua_setmetatable(s_L, -2); // setmetatable(Matrix4x4, Matrix4x4_mt)
		//lua_pop(s_L, -1);

		// Quaternion metatable
		luaL_newmetatable(s_L, "Quaternion_mt");
		lua_pushvalue(s_L, -1);
		lua_setfield(s_L, -2, "__index");
		lua_pushcfunction(s_L, crown_lua_quaternion_call);
		lua_setfield(s_L, -2, "__call");
		//lua_pop(s_L, -1);

		lua_getglobal(s_L, "Quaternion");
		lua_pushvalue(s_L, -2); // Duplicate metatable
		lua_setmetatable(s_L, -2); // setmetatable(Quaternion, Quaternion_mt)
		//lua_pop(s_L, -1);

		Log::d("Stacksize = %d", lua_gettop(s_L));
	}

	void shutdown()
	{
		lua_close(s_L);
	}

	lua_State* state()
	{
		return s_L;
	}

	Vector2* next_vector2(const Vector2& v)
	{
		CE_ASSERT(s_vec2_used < CE_MAX_LUA_VECTOR2, "Maximum number of Vector2 reached");

		return &(s_vec2_buffer[s_vec2_used++] = v);
	}

	Vector3* next_vector3(const Vector3& v)
	{
		CE_ASSERT(s_vec3_used < CE_MAX_LUA_VECTOR3, "Maximum number of Vector3 reached");

		return &(s_vec3_buffer[s_vec3_used++] = v);
	}

	Matrix4x4* next_matrix4x4(const Matrix4x4& m)
	{
		CE_ASSERT(s_mat4_used < CE_MAX_LUA_MATRIX4X4, "Maximum number of Matrix4x4 reached");

		return &(s_mat4_buffer[s_mat4_used++] = m);
	}

	Quaternion* next_quaternion(const Quaternion& q)
	{
		CE_ASSERT(s_quat_used < CE_MAX_LUA_QUATERNION, "Maximum number of Quaternion reached");
		return &(s_quat_buffer[s_quat_used++] = q);
	}

	bool is_vector2(int32_t index)
	{
		void* type = lua_touserdata(s_L, index);
		return (type >= &s_vec2_buffer[0] && type <= &s_vec2_buffer[CE_MAX_LUA_VECTOR2 - 1]);
	}

	bool is_vector3(int32_t index)
	{
		void* type = lua_touserdata(s_L, index);
		return (type >= &s_vec3_buffer[0] && type <= &s_vec3_buffer[CE_MAX_LUA_VECTOR3 - 1]);
	}

	bool is_matrix4x4(int32_t index)
	{
		void* type = lua_touserdata(s_L, index);
		return (type >= &s_mat4_buffer[0] && type <= &s_mat4_buffer[CE_MAX_LUA_MATRIX4X4 - 1]);
	}

	bool is_quaternion(int32_t index)
	{
		void* type = lua_touserdata(s_L, index);
		return (type >= &s_quat_buffer[0] && type <= &s_quat_buffer[CE_MAX_LUA_QUATERNION - 1]);
	}

	void clear_temporaries()
	{
		s_vec2_used = 0;
		s_vec3_used = 0;
		s_mat4_used = 0;
		s_quat_used = 0;
	}

} // namespace lua_system
} // namespace crown
