/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "matrix4x4.h"
#include "vector3.h"
#include "lua_stack.h"
#include "lua_environment.h"
#include "os.h"

namespace crown
{

static int matrix4x4box_new(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4box(stack.get_matrix4x4(1));
	return 1;
}

static int matrix4x4box_ctor(lua_State* L)
{
	LuaStack stack(L);
	stack.remove(1); // Remove table
	return matrix4x4box_new(L);
}

static int matrix4x4box_store(lua_State* L)
{
	LuaStack stack(L);
	stack.get_matrix4x4box(1) = stack.get_matrix4x4(2);
	return 0;
}

static int matrix4x4box_unbox(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(stack.get_matrix4x4box(1));
	return 1;
}

static int matrix4x4box_tostring(lua_State* L)
{
	LuaStack stack(L);
	Matrix4x4& m = stack.get_matrix4x4box(1);
	stack.push_fstring("Matrix4x4Box (%p)", &m);
	return 1;
}

void load_matrix4x4box(LuaEnvironment& env)
{
	env.load_module_function("Matrix4x4Box", "new",        matrix4x4box_new);
	env.load_module_function("Matrix4x4Box", "store",      matrix4x4box_store);
	env.load_module_function("Matrix4x4Box", "unbox",      matrix4x4box_unbox);
	env.load_module_function("Matrix4x4Box", "__tostring", matrix4x4box_tostring);

	env.load_module_constructor("Matrix4x4Box", matrix4x4box_ctor);
}

} // namespace crown
