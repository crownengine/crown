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

#include "lua_system.h"
#include "lua.hpp"
#include "config.h"
#include "lua_stack.h"
#include "assert.h"
#include "vector2.h"
#include "vector3.h"
#include "matrix4x4.h"
#include "quaternion.h"
#include "device.h"
#include "resource_manager.h"
#include "lua_resource.h"
#include "lua_environment.h"
#include "lua_stack.h"
#include "log.h"

namespace crown
{

// Lua modules
extern void load_actor(LuaEnvironment& env);
extern void load_camera(LuaEnvironment& env);
extern void load_controller(LuaEnvironment& env);
extern void load_debug_line(LuaEnvironment& env);
extern void load_device(LuaEnvironment& env);
extern void load_float_setting(LuaEnvironment& env);
extern void load_gui(LuaEnvironment& env);
extern void load_int_setting(LuaEnvironment& env);
extern void load_keyboard(LuaEnvironment& env);
extern void load_math(LuaEnvironment& env);
extern void load_matrix4x4(LuaEnvironment& env);
extern void load_matrix4x4box(LuaEnvironment& env);
extern void load_mesh(LuaEnvironment& env);
extern void load_mouse(LuaEnvironment& env);
extern void load_physics_world(LuaEnvironment& env);
extern void load_quaternion(LuaEnvironment& env);
extern void load_quaternionbox(LuaEnvironment& env);
extern void load_raycast(LuaEnvironment& env);
extern void load_resource_package(LuaEnvironment& env);
extern void load_sound_world(LuaEnvironment& env);
extern void load_sprite(LuaEnvironment& env);
extern void load_string_setting(LuaEnvironment& env);
extern void load_touch(LuaEnvironment& env);
extern void load_unit(LuaEnvironment& env);
extern void load_vector3(LuaEnvironment& env);
extern void load_vector3box(LuaEnvironment& env);
extern void load_window(LuaEnvironment& env);
extern void load_world(LuaEnvironment& env);
extern void load_color4(LuaEnvironment& env);
extern void load_material(LuaEnvironment& env);

namespace lua_globals
{
	static lua_State* s_L;

	static uint32_t 		s_vec3_used = 0;
	static Vector3 			s_vec3_buffer[CE_MAX_LUA_VECTOR3];
	static uint32_t 		s_mat4_used = 0;
	static Matrix4x4 		s_mat4_buffer[CE_MAX_LUA_MATRIX4X4];
	static uint32_t 		s_quat_used = 0;
	static Quaternion 		s_quat_buffer[CE_MAX_LUA_QUATERNION];

	// When an error occurs, logs the error message and pauses the engine.
	int error_handler(lua_State* L)
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

		CE_LOGE(lua_tostring(L, -1)); // Print error message
		lua_pop(L, 1); // Remove error message from stack
		lua_pop(L, 1); // Remove debug.traceback from stack

		device()->pause();
		return 0;
	}

	// Redirects require to the resource manager.
	static int require(lua_State* L)
	{
		using namespace lua_resource;
		LuaStack stack(L);
		const LuaResource* lr = (LuaResource*)device()->resource_manager()->get("lua", stack.get_string(1));
		luaL_loadbuffer(L, program(lr), size(lr), "");
		return 1;
	}

	static int lightuserdata_add(lua_State* L)
	{
		LuaStack stack(L);
		const Vector3& a = stack.get_vector3(1);
		const Vector3& b = stack.get_vector3(2);
		stack.push_vector3(a + b);
		return 1;
	}

	static int lightuserdata_sub(lua_State* L)
	{
		LuaStack stack(L);
		const Vector3& a = stack.get_vector3(1);
		const Vector3& b = stack.get_vector3(2);
		stack.push_vector3(a - b);
		return 1;
	}

	static int lightuserdata_mul(lua_State* L)
	{
		LuaStack stack(L);
		const Vector3& a = stack.get_vector3(1);
		const float b = stack.get_float(2);
		stack.push_vector3(a * b);
		return 1;
	}

	static int lightuserdata_div(lua_State* L)
	{
		LuaStack stack(L);
		const Vector3& a = stack.get_vector3(1);
		const float b = stack.get_float(2);
		stack.push_vector3(a / b);
		return 1;
	}

	static int lightuserdata_unm(lua_State* L)
	{
		LuaStack stack(L);
		stack.push_vector3(-stack.get_vector3(1));
		return 1;
	}

	static int lightuserdata_index(lua_State* L)
	{
		LuaStack stack(L);
		Vector3& v = stack.get_vector3(1);
		const char* s = stack.get_string(2);

		if (string::strcmp(s, "x") == 0)
		{
			stack.push_float(v.x);
			return 1;
		}
		else if (string::strcmp(s, "y") == 0)
		{
			stack.push_float(v.y);
			return 1;
		}
		else if (string::strcmp(s, "z") == 0)
		{
			stack.push_float(v.z);
			return 1;
		}

		return 0;
	}

	static int lightuserdata_newindex(lua_State* L)
	{
		LuaStack stack(L);
		Vector3& v = stack.get_vector3(1);
		const char* s = stack.get_string(2);
		const float value = stack.get_float(3);

		if (string::strcmp(s, "x") == 0) v.x = value;
		else if (string::strcmp(s, "y") == 0) v.y = value;
		else if (string::strcmp(s, "z") == 0) v.z = value;

		return 0;
	}

	// Initializes lua subsystem
	void init()
	{
		s_L = luaL_newstate();
		CE_ASSERT(s_L, "Unable to create lua state");

		// Open default libraries
		luaL_openlibs(s_L);

		// Register crown libraries
		LuaEnvironment env(s_L);
		load_actor(env);
		load_camera(env);
		load_controller(env);
		load_debug_line(env);
		load_device(env);
		load_float_setting(env);
		load_gui(env);
		load_int_setting(env);
		load_keyboard(env);
		load_math(env);
		load_matrix4x4(env);
		load_mesh(env);
		load_mouse(env);
		load_physics_world(env);
		load_quaternion(env);
		load_quaternionbox(env);
		load_raycast(env);
		load_resource_package(env);
		load_sound_world(env);
		load_sprite(env);
		load_string_setting(env);
		load_touch(env);
		load_unit(env);
		load_vector3(env);
		load_vector3box(env);
		load_window(env);
		load_world(env);
		load_color4(env);
		load_material(env);

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
		lua_pushcfunction(s_L, require);
		lua_rawset(s_L, -3);
		lua_pop(s_L, 1);

		// Create metatable for lightuserdata
		luaL_newmetatable(s_L, "Lightuserdata_mt");
		lua_pushstring(s_L, "__add");
		lua_pushcfunction(s_L, lightuserdata_add);
		lua_settable(s_L, 1);

		lua_pushstring(s_L, "__sub");
		lua_pushcfunction(s_L, lightuserdata_sub);
		lua_settable(s_L, 1);

		lua_pushstring(s_L, "__mul");
		lua_pushcfunction(s_L, lightuserdata_mul);
		lua_settable(s_L, 1);

		lua_pushstring(s_L, "__div");
		lua_pushcfunction(s_L, lightuserdata_div);
		lua_settable(s_L, 1);

		lua_pushstring(s_L, "__unm");
		lua_pushcfunction(s_L, lightuserdata_unm);
		lua_settable(s_L, 1);

		lua_pushstring(s_L, "__index");
		lua_pushcfunction(s_L, lightuserdata_index);
		lua_settable(s_L, 1);

		lua_pushstring(s_L, "__newindex");
		lua_pushcfunction(s_L, lightuserdata_newindex);
		lua_settable(s_L, 1);

		lua_pop(s_L, 1); // Pop Lightuserdata_mt

		// Ensure stack is clean
		CE_ASSERT(lua_gettop(s_L) == 0, "Stack not clean");
	}

	void shutdown()
	{
		lua_close(s_L);
	}

	lua_State* state()
	{
		return s_L;
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
		s_vec3_used = 0;
		s_mat4_used = 0;
		s_quat_used = 0;
	}

} // namespace lua_globals
} // namespace crown
