/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "config.h"
#include "types.h"
#include "resource_types.h"
#include "math_types.h"
#include <lua.hpp>

namespace crown
{
enum LuaArgumentType
{
	ARGUMENT_FLOAT
};

/// Wraps a subset of Lua functions and provides utilities for extending Lua.
///
/// @ingroup Lua
struct LuaEnvironment
{
	lua_State* L;

	u32 _vec3_used;
	Vector3 _vec3_buffer[CROWN_MAX_LUA_VECTOR3];
	u32 _quat_used;
	Quaternion _quat_buffer[CROWN_MAX_LUA_QUATERNION];
	u32 _mat4_used;
	Matrix4x4 _mat4_buffer[CROWN_MAX_LUA_MATRIX4X4];

	LuaEnvironment();
	~LuaEnvironment();

	/// Loads lua libraries.
	void load_libs();

	/// Executes the lua resource @a lr.
	void execute(const LuaResource* lr);

	/// Executes the @a lua string.
	void execute_string(const char* lua);

	/// Adds the function with the given @a name and @a func to the table @a module.
	void add_module_function(const char* module, const char* name, const lua_CFunction func);

	/// Adds the function with the given @a name and @a func to the table @a module.
	void add_module_function(const char* module, const char* name, const char* func);

	/// Sets the constructor for the table @a module to the given function.
	void set_module_constructor(const char* module, const lua_CFunction func);

	/// Calls the global function @a func with @a argc argument number.
	/// Each argument is a pair (type, value).
	/// Example call:
	/// call_global("myfunc", 1, ARGUMENT_FLOAT, 3.14f)
	/// Returns true if success, false otherwise
	void call_global(const char* func, u8 argc, ...);

	/// Resets temporary types.
	void reset_temporaries();

	/// Returns a new temporary Vector3.
	Vector3* next_vector3(const Vector3& v);

	/// Returns a new temporary Quaternion.
	Quaternion* next_quaternion(const Quaternion& q);

	/// Returns a new temporary Matrix4x4.
	Matrix4x4* next_matrix4x4(const Matrix4x4& m);

	/// Returns whether @a p is a temporary Vector3.
	bool is_vector3(const Vector3* p) const;

	/// Returns whether @a p is a temporary Quaternion.
	bool is_quaternion(const Quaternion* p) const;

	/// Returns whether @a p is a temporary Matrix4x4.
	bool is_matrix4x4(const Matrix4x4* p) const;

private:

	// Disable copying
	LuaEnvironment(const LuaEnvironment&);
	LuaEnvironment& operator=(const LuaEnvironment&);
};

} // namespace crown
