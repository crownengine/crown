/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "matrix4x4.h"
#include "vector3.h"
#include "lua_stack.h"
#include "lua_environment.h"

namespace crown
{

using namespace matrix4x4;

static int matrix4x4_new(lua_State* L)
{
	LuaStack stack(L);
	float m0 = stack.get_float(1);
	float m1 = stack.get_float(2);
	float m2 = stack.get_float(3);
	float m3 = stack.get_float(4);
	float m4 = stack.get_float(5);
	float m5 = stack.get_float(6);
	float m6 = stack.get_float(7);
	float m7 = stack.get_float(8);
	float m8 = stack.get_float(9);
	float m9 = stack.get_float(10);
	float m10 = stack.get_float(11);
	float m11 = stack.get_float(12);
	float m12 = stack.get_float(13);
	float m13 = stack.get_float(14);
	float m14 = stack.get_float(15);
	float m15 = stack.get_float(16);
	stack.push_matrix4x4(Matrix4x4(m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15));
	return 1;
}

static int matrix4x4_ctor(lua_State* L)
{
	LuaStack stack(L);
	stack.remove(1); // Remove table
	return matrix4x4_new(L);
}

static int matrix4x4_from_quaternion(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(Matrix4x4(stack.get_quaternion(1), Vector3(0, 0, 0)));
	return 1;
}

static int matrix4x4_from_translation(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(Matrix4x4(quaternion::IDENTITY, stack.get_vector3(1)));
	return 1;
}

static int matrix4x4_from_quaternion_translation(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(Matrix4x4(stack.get_quaternion(1), stack.get_vector3(2)));
	return 1;
}

static int matrix4x4_from_axes(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(Matrix4x4(stack.get_vector3(1), stack.get_vector3(2), stack.get_vector3(3), stack.get_vector3(4)));
	return 1;
}

static int matrix4x4_add(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(stack.get_matrix4x4(1) + stack.get_matrix4x4(2));
	return 1;
}

static int matrix4x4_subtract(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(stack.get_matrix4x4(1) - stack.get_matrix4x4(2));
	return 1;
}

static int matrix4x4_multiply(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(stack.get_matrix4x4(1) * stack.get_matrix4x4(2));
	return 1;
}

static int matrix4x4_transpose(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(transpose(stack.get_matrix4x4(1)));
	return 1;
}

static int matrix4x4_determinant(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(determinant(stack.get_matrix4x4(1)));
	return 1;
}

static int matrix4x4_invert(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(invert(stack.get_matrix4x4(1)));
	return 1;
}

static int matrix4x4_x(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(x(stack.get_matrix4x4(1)));
	return 1;
}

static int matrix4x4_y(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(y(stack.get_matrix4x4(1)));
	return 1;
}

static int matrix4x4_z(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(z(stack.get_matrix4x4(1)));
	return 1;
}

static int matrix4x4_set_x(lua_State* L)
{
	LuaStack stack(L);
	set_x(stack.get_matrix4x4(1), stack.get_vector3(2));
	return 0;
}

static int matrix4x4_set_y(lua_State* L)
{
	LuaStack stack(L);
	set_y(stack.get_matrix4x4(1), stack.get_vector3(2));
	return 0;
}

static int matrix4x4_set_z(lua_State* L)
{
	LuaStack stack(L);
	set_z(stack.get_matrix4x4(1), stack.get_vector3(2));
	return 0;
}

static int matrix4x4_translation(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(translation(stack.get_matrix4x4(1)));
	return 1;
}

static int matrix4x4_set_translation(lua_State* L)
{
	LuaStack stack(L);
	set_translation(stack.get_matrix4x4(1), stack.get_vector3(2));
	return 0;
}

static int matrix4x4_rotation(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(rotation(stack.get_matrix4x4(1)));
	return 1;
}

static int matrix4x4_set_rotation(lua_State* L)
{
	LuaStack stack(L);
	set_rotation(stack.get_matrix4x4(1), stack.get_quaternion(2));
	return 0;
}

static int matrix4x4_identity(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(matrix4x4::IDENTITY);
	return 1;
}

static int matrix4x4_to_string(lua_State* L)
{
	LuaStack stack(L);
	Matrix4x4& a = stack.get_matrix4x4(1);
	stack.push_fstring("%.1f, %.1f, %.1f, %.1f\n%.1f, %.1f, %.1f, %.1f\n%.1f, %.1f, %.1f, %.1f\n%.1f, %.1f, %.1f, %.1f\n",
						a[0], a[4], a[8], a[12], a[1], a[5], a[9], a[13], a[2], a[6], a[10], a[14], a[3], a[7], a[11], a[15]);
	return 1;
}

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

void load_matrix4x4(LuaEnvironment& env)
{
	env.load_module_function("Matrix4x4", "new",                         matrix4x4_new);
	env.load_module_function("Matrix4x4", "from_quaternion",             matrix4x4_from_quaternion);
	env.load_module_function("Matrix4x4", "from_translation",            matrix4x4_from_translation);
	env.load_module_function("Matrix4x4", "from_quaternion_translation", matrix4x4_from_quaternion_translation);
	env.load_module_function("Matrix4x4", "from_axes",                   matrix4x4_from_axes);
	env.load_module_function("Matrix4x4", "add",                         matrix4x4_add);
	env.load_module_function("Matrix4x4", "subtract",                    matrix4x4_subtract);
	env.load_module_function("Matrix4x4", "multiply",                    matrix4x4_multiply);
	env.load_module_function("Matrix4x4", "transpose",                   matrix4x4_transpose);
	env.load_module_function("Matrix4x4", "determinant",                 matrix4x4_determinant);
	env.load_module_function("Matrix4x4", "invert",                      matrix4x4_invert);
	env.load_module_function("Matrix4x4", "x",                           matrix4x4_x);
	env.load_module_function("Matrix4x4", "y",                           matrix4x4_y);
	env.load_module_function("Matrix4x4", "z",                           matrix4x4_z);
	env.load_module_function("Matrix4x4", "set_x",                       matrix4x4_set_x);
	env.load_module_function("Matrix4x4", "set_y",                       matrix4x4_set_y);
	env.load_module_function("Matrix4x4", "set_z",                       matrix4x4_set_z);
	env.load_module_function("Matrix4x4", "translation",                 matrix4x4_translation);
	env.load_module_function("Matrix4x4", "set_translation",             matrix4x4_set_translation);
	env.load_module_function("Matrix4x4", "rotation",                    matrix4x4_rotation);
	env.load_module_function("Matrix4x4", "set_rotation",                matrix4x4_set_rotation);
	env.load_module_function("Matrix4x4", "identity",                    matrix4x4_identity);
	env.load_module_function("Matrix4x4", "to_string",                   matrix4x4_to_string);

	env.load_module_constructor("Matrix4x4", matrix4x4_ctor);

	env.load_module_function("Matrix4x4Box", "new",        matrix4x4box_new);
	env.load_module_function("Matrix4x4Box", "store",      matrix4x4box_store);
	env.load_module_function("Matrix4x4Box", "unbox",      matrix4x4box_unbox);
	env.load_module_function("Matrix4x4Box", "__tostring", matrix4x4box_tostring);

	env.load_module_constructor("Matrix4x4Box", matrix4x4box_ctor);
}

} //namespace crown
