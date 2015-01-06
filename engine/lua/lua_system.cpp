/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "lua_system.h"
#include "config.h"
#include "lua_stack.h"
#include "lua_assert.h"
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
#include "lua.hpp"

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
	static lua_State* L;

	static uint32_t _vec3_used = 0;
	static Vector3 _vec3_buffer[CROWN_MAX_LUA_VECTOR3];
	static uint32_t _mat4_used = 0;
	static Matrix4x4 s_mat4_buffer[CROWN_MAX_LUA_MATRIX4X4];
	static uint32_t _quat_used = 0;
	static Quaternion _quat_buffer[CROWN_MAX_LUA_QUATERNION];

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

		switch (s[0])
		{
			case 'x': stack.push_float(v.x); return 1;
			case 'y': stack.push_float(v.y); return 1;
			case 'z': stack.push_float(v.z); return 1;
			default: LUA_ASSERT(false, stack, "Bad index: '%c'", s[0]); break;
		}

		return 0;
	}

	static int lightuserdata_newindex(lua_State* L)
	{
		LuaStack stack(L);
		Vector3& v = stack.get_vector3(1);
		const char* s = stack.get_string(2);
		const float value = stack.get_float(3);

		switch (s[0])
		{
			case 'x': v.x = value; break;
			case 'y': v.y = value; break;
			case 'z': v.z = value; break;
			default: LUA_ASSERT(false, stack, "Bad index: '%c'", s[0]); break;
		}

		return 0;
	}

	// Initializes lua subsystem
	void init()
	{
		L = luaL_newstate();
		CE_ASSERT(L, "Unable to create lua state");

		// Open default libraries
		luaL_openlibs(L);

		// Register crown libraries
		LuaEnvironment env(L);
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
		lua_getfield(L, LUA_GLOBALSINDEX, "package");
		lua_getfield(L, -1, "loaders");
		lua_remove(L, -2);

		int num_loaders = 0;
		lua_pushnil(L);
		while (lua_next(L, -2) != 0)
		{
			lua_pop(L, 1);
			num_loaders++;
		}
		lua_pushinteger(L, num_loaders + 1);
		lua_pushcfunction(L, require);
		lua_rawset(L, -3);
		lua_pop(L, 1);

		// Create metatable for lightuserdata
		luaL_newmetatable(L, "Lightuserdata_mt");
		lua_pushstring(L, "__add");
		lua_pushcfunction(L, lightuserdata_add);
		lua_settable(L, 1);

		lua_pushstring(L, "__sub");
		lua_pushcfunction(L, lightuserdata_sub);
		lua_settable(L, 1);

		lua_pushstring(L, "__mul");
		lua_pushcfunction(L, lightuserdata_mul);
		lua_settable(L, 1);

		lua_pushstring(L, "__div");
		lua_pushcfunction(L, lightuserdata_div);
		lua_settable(L, 1);

		lua_pushstring(L, "__unm");
		lua_pushcfunction(L, lightuserdata_unm);
		lua_settable(L, 1);

		lua_pushstring(L, "__index");
		lua_pushcfunction(L, lightuserdata_index);
		lua_settable(L, 1);

		lua_pushstring(L, "__newindex");
		lua_pushcfunction(L, lightuserdata_newindex);
		lua_settable(L, 1);

		lua_pop(L, 1); // Pop Lightuserdata_mt

		// Ensure stack is clean
		CE_ASSERT(lua_gettop(L) == 0, "Stack not clean");
	}

	void shutdown()
	{
		lua_close(L);
	}

	lua_State* state()
	{
		return L;
	}

	Vector3* next_vector3(const Vector3& v)
	{
		CE_ASSERT(_vec3_used < CROWN_MAX_LUA_VECTOR3, "Maximum number of Vector3 reached");

		return &(_vec3_buffer[_vec3_used++] = v);
	}

	Matrix4x4* next_matrix4x4(const Matrix4x4& m)
	{
		CE_ASSERT(_mat4_used < CROWN_MAX_LUA_MATRIX4X4, "Maximum number of Matrix4x4 reached");

		return &(s_mat4_buffer[_mat4_used++] = m);
	}

	Quaternion* next_quaternion(const Quaternion& q)
	{
		CE_ASSERT(_quat_used < CROWN_MAX_LUA_QUATERNION, "Maximum number of Quaternion reached");
		return &(_quat_buffer[_quat_used++] = q);
	}

	bool is_vector3(int32_t index)
	{
		void* type = lua_touserdata(L, index);
		return (type >= &_vec3_buffer[0] && type <= &_vec3_buffer[CROWN_MAX_LUA_VECTOR3 - 1]);
	}

	bool is_matrix4x4(int32_t index)
	{
		void* type = lua_touserdata(L, index);
		return (type >= &s_mat4_buffer[0] && type <= &s_mat4_buffer[CROWN_MAX_LUA_MATRIX4X4 - 1]);
	}

	bool is_quaternion(int32_t index)
	{
		void* type = lua_touserdata(L, index);
		return (type >= &_quat_buffer[0] && type <= &_quat_buffer[CROWN_MAX_LUA_QUATERNION - 1]);
	}

	void clear_temporaries()
	{
		_vec3_used = 0;
		_mat4_used = 0;
		_quat_used = 0;
	}

} // namespace lua_globals
} // namespace crown
