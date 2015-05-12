/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "config.h"
#include "types.h"
#include "macros.h"
#include "resource_types.h"
#include "math_types.h" // HACK
#include "lua.hpp"

namespace crown
{

enum LuaArgumentType
{
	ARGUMENT_FLOAT
};

// HACK
struct Actor;
struct Unit;

/// LuaEnvironment is a wrapper of a subset of Lua functions and
/// provides utilities for extending Lua
struct LuaEnvironment
{
	LuaEnvironment();
	~LuaEnvironment();

	void load_libs();

	void execute(const LuaResource* lr);

	/// Loads and executes the given @a s lua string.
	void execute_string(const char* s);

	/// Loads the function with the given @a name and @a func into the table @a module.
	void load_module_function(const char* module, const char* name, const lua_CFunction func);
	void load_module_function(const char* module, const char* name, const char* value);
	void load_module_constructor(const char* module, const lua_CFunction func);

	/// Loads the enum with the given @a name and @a value into the table @a module.
	void load_module_enum(const char* module, const char* name, uint32_t value);

	/// Calls the global function @a func with @a argc argument number.
	/// Each argument is a pair (type, value).
	/// Example call:
	/// call_global("myfunc", 1, ARGUMENT_FLOAT, 3.14f)
	/// Returns true if success, false otherwise
	void call_global(const char* func, uint8_t argc, ...);

	void clear_temporaries();

	Vector3* next_vector3(const Vector3& v);
	Quaternion* next_quaternion(const Quaternion& q);
	Matrix4x4* next_matrix4x4(const Matrix4x4& m);
	bool is_vector3(int i);
	bool is_quaternion(int i);
	bool is_matrix4x4(int i);

	// FIXME
	void call_physics_callback(Actor* actor_0, Actor* actor_1, Unit* unit_0, Unit* unit_1, const Vector3& where, const Vector3& normal, const char* type);
	void call_trigger_callback(Actor* trigger, Actor* other, const char* type);

private:

	lua_State* L;

	uint32_t _vec3_used;
	Vector3 _vec3_buffer[CROWN_MAX_LUA_VECTOR3];
	uint32_t _quat_used;
	Quaternion _quat_buffer[CROWN_MAX_LUA_QUATERNION];
	uint32_t _mat4_used;
	Matrix4x4 s_mat4_buffer[CROWN_MAX_LUA_MATRIX4X4];

private:

	// Disable copying
	LuaEnvironment(const LuaEnvironment&);
	LuaEnvironment& operator=(const LuaEnvironment&);
};

} // namespace crown
