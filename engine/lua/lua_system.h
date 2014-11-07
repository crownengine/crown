/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "types.h"
#include "math_types.h"
#include "lua.hpp"

namespace crown
{

/// Global lua-related functions
namespace lua_globals
{
	/// Initializes the lua system.
	/// This is the place where to create and initialize per-application objects.
	void init();

	/// It should reverse the actions performed by lua_globals::init().
	void shutdown();

	lua_State* state();

	/// Clears temporary objects (Vector3, Matrix4x4, ...).
	void clear_temporaries();

	/// Returns a new temporary Vector2, Vector3, Matrix4x4 or Quaternion
	Vector3* next_vector3(const Vector3& v);
	Matrix4x4* next_matrix4x4(const Matrix4x4& m);
	Quaternion* next_quaternion(const Quaternion& q);

	/// Returns whether the object at stack @a index is a Vector3, Matrix4x4 or Quaternion
	bool is_vector3(int32_t index);
	bool is_matrix4x4(int32_t index);
	bool is_quaternion(int32_t index);
} // namespace lua_globals
} // namespace crown
