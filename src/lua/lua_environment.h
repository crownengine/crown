/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "config.h"
#include "core/math/random.h"
#include "core/math/types.h"
#include "core/types.h"
#include "device/types.h"
#include "lua/lua_stack.h"
#include "resource/types.h"
struct lua_State;

namespace crown
{
int report (lua_State *L, int status);

/// Wraps a subset of Lua functions and provides utilities for extending Lua.
///
/// @ingroup Lua
struct LuaEnvironment
{
	lua_State* L;

#define LUA_MAX_VECTOR3 (CROWN_LUA_MAX_VECTOR3_SIZE / sizeof(Vector3))
CE_STATIC_ASSERT(CROWN_LUA_MAX_VECTOR3_SIZE % sizeof(Vector3) == 0);

#define LUA_MAX_QUATERNION (CROWN_LUA_MAX_QUATERNION_SIZE / sizeof(Quaternion))
CE_STATIC_ASSERT(CROWN_LUA_MAX_QUATERNION_SIZE % sizeof(Quaternion) == 0);

#define LUA_MAX_MATRIX4X4 (CROWN_LUA_MAX_MATRIX4X4_SIZE / sizeof(Matrix4x4))
CE_STATIC_ASSERT(CROWN_LUA_MAX_MATRIX4X4_SIZE % sizeof(Matrix4x4) == 0);

	u32 _num_vec3;
	CE_ALIGN_DECL(4, Vector3 _vec3[LUA_MAX_VECTOR3]);
	CE_STATIC_ASSERT( 4 == 1 + LUA_VECTOR3_MARKER_MASK);
	u32 _num_quat;
	CE_ALIGN_DECL(16, Quaternion _quat[LUA_MAX_QUATERNION]);
	CE_STATIC_ASSERT(16 == 1 + LUA_QUATERNION_MARKER_MASK);
	u32 _num_mat4;
	CE_ALIGN_DECL(64, Matrix4x4 _mat4[LUA_MAX_MATRIX4X4]);
	CE_STATIC_ASSERT(64 == 1 + LUA_MATRIX4X4_MARKER_MASK);
#if CROWN_DEBUG
	uintptr_t _vec3_marker;
	uintptr_t _quat_marker;
	uintptr_t _mat4_marker;
	Random _random;
#endif

	///
	LuaEnvironment();

	///
	~LuaEnvironment();

	///
	LuaEnvironment(const LuaEnvironment&) = delete;

	///
	LuaEnvironment& operator=(const LuaEnvironment&) = delete;

	/// Loads lua libraries.
	void load_libs();

	/// Equivalent to calling `dofile "name"` from Lua.
	void do_file(const char* name);

	/// Equivalent to calling `require "name"` from Lua.
	void require(const char* name);

	/// Executes the lua resource @a lr.
	LuaStack execute(const LuaResource* lr, int nres);

	/// Executes the @a lua string.
	LuaStack execute_string(const char* lua);

	/// Adds the function with the given @a name and @a func to the table @a module.
	void add_module_function(const char* module, const char* name, const lua_CFunction func);

	/// Adds the function with the given @a name and @a func to the table @a module.
	void add_module_function(const char* module, const char* name, const char* func);

	///
	void add_module_metafunction(const char* module, const char* name, const lua_CFunction func);

	/// Interface to lua_pcall/lua_call.
	int call(int narg, int nres);

	/// Calls the global function @a func.
	void call_global(const char* func, int narg = 0, int nres = 0);

	///
	LuaStack get_global(const char* global);

	/// Returns the number of temporary objects in use.
	void temp_count(u32& num_vec3, u32& num_quat, u32& num_mat4);

	/// Sets the number of temporary objects in use.
	void set_temp_count(u32 num_vec3, u32 num_quat, u32 num_mat4);

	/// Resets temporary types.
	void reset_temporaries();

	/// Returns a new temporary Vector3.
	Vector3* next_vector3(const Vector3& v);

	/// Returns a new temporary Quaternion.
	Quaternion* next_quaternion(const Quaternion& q);

	/// Returns a new temporary Matrix4x4.
	Matrix4x4* next_matrix4x4(const Matrix4x4& m);

	/// Returns whether @a ptr is a temporary Vector3.
	bool is_vector3(const void* ptr);

	/// Returns whether @a ptr is a temporary Quaternion.
	bool is_quaternion(const void* ptr);

	/// Returns whether @a ptr is a temporary Matrix4x4.
	bool is_matrix4x4(const void* ptr);

	/// Returns the actual address of @a ptr if it is not stale,
	/// otherwise it generates a Lua error.
	Vector3* check_valid(const Vector3* ptr);

	/// Returns the actual address of @a ptr if it is not stale,
	/// otherwise it generates a Lua error.
	Quaternion* check_valid(const Quaternion* ptr);

	/// Returns the actual address of @a ptr if it is not stale,
	/// otherwise it generates a Lua error.
	Matrix4x4* check_valid(const Matrix4x4* ptr);

	/// Reloads (executes) all lua files that has been loaded since the program
	/// started.
	void reload();

	///
	void register_console_commands(ConsoleServer& cs);
};

} // namespace crown
