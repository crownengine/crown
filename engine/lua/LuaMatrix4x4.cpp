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

#include "Matrix4x4.h"
#include "Vector3.h"
#include "LuaStack.h"
#include "LuaEnvironment.h"
#include "OS.h"

namespace crown
{

//-----------------------------------------------------------------------------
CE_EXPORT int matrix4x4(lua_State* L)
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

//-----------------------------------------------------------------------------					
CE_EXPORT int matrix4x4_add(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);
	Matrix4x4& b = stack.get_matrix4x4(2);

	stack.push_matrix4x4(a + b);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int matrix4x4_subtract(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);
	Matrix4x4& b = stack.get_matrix4x4(2);

	stack.push_matrix4x4(a - b);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int matrix4x4_multiply(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);
	Matrix4x4& b = stack.get_matrix4x4(2);

	stack.push_matrix4x4(a * b);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int matrix4x4_build_rotation_x(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);
	float k = stack.get_float(2);

	a.build_rotation_x(k);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int matrix4x4_build_rotation_y(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);
	float k = stack.get_float(2);

	a.build_rotation_y(k);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int matrix4x4_build_rotation_z(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);
	float k = stack.get_float(2);

	a.build_rotation_z(k);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int matrix4x4_build_rotation(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);
	Vector3& d = stack.get_vector3(2);
	float k = stack.get_float(3);

	a.build_rotation(d, k);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int matrix4x4_build_look_at_rh(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);
	Vector3& pos = stack.get_vector3(2);
	Vector3& target = stack.get_vector3(3);
	Vector3& up = stack.get_vector3(4);

	a.build_look_at_rh(pos, target, up);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int matrix4x4_build_viewpoint_billboard(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);
	Vector3& pos = stack.get_vector3(2);
	Vector3& target = stack.get_vector3(3);
	Vector3& up = stack.get_vector3(4);

	a.build_viewpoint_billboard(pos, target, up);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int matrix4x4_build_axis_billboard(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);
	Vector3& pos = stack.get_vector3(2);
	Vector3& target = stack.get_vector3(3);
	Vector3& up = stack.get_vector3(4);

	a.build_axis_billboard(pos, target, up);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int matrix4x4_transpose(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);

	stack.push_matrix4x4(a.transpose());

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int matrix4x4_determinant(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);

	stack.push_float(a.get_determinant());

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int matrix4x4_invert(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);

	stack.push_matrix4x4(a.invert());

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int matrix4x4_load_identity(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);

	a.load_identity();

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int matrix4x4_x(lua_State* L)
{	
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);

	stack.push_vector3(a.x());

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int matrix4x4_y(lua_State* L)
{	
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);

	stack.push_vector3(a.y());

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int matrix4x4_z(lua_State* L)
{	
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);

	stack.push_vector3(a.z());

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int matrix4x4_set_x(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);
	Vector3& x = stack.get_vector3(2);

	a.set_x(x);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int matrix4x4_set_y(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);
	Vector3& y = stack.get_vector3(2);

	a.set_y(y);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int matrix4x4_set_z(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);
	Vector3& z = stack.get_vector3(2);

	a.set_z(z);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int matrix4x4_translation(lua_State* L)
{	
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);

	stack.push_vector3(a.translation());

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int matrix4x4_set_translation(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);
	Vector3& trans = stack.get_vector3(2);

	a.set_translation(trans);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int matrix4x4_get_scale(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);

	stack.push_vector3(a.get_scale());

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int matrix4x4_set_scale(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);
	Vector3& scale = stack.get_vector3(2);

	a.set_scale(scale);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int matrix4x4_print(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);

	os::printf("|%.1f|%.1f|%.1f|%.1f|\n", a.m[0], a.m[4], a.m[8], a.m[12]);
	os::printf("|%.1f|%.1f|%.1f|%.1f|\n", a.m[1], a.m[5], a.m[9], a.m[13]);
	os::printf("|%.1f|%.1f|%.1f|%.1f|\n", a.m[2], a.m[6], a.m[10], a.m[14]);
	os::printf("|%.1f|%.1f|%.1f|%.1f|\n", a.m[3], a.m[7], a.m[11], a.m[15]);

	return 0;
}

//-----------------------------------------------------------------------------
void load_matrix4x4(LuaEnvironment& env)
{
	env.load_module_function("Matrix4x4", "new", 							matrix4x4);
	env.load_module_function("Matrix4x4", "add", 							matrix4x4_add);
	env.load_module_function("Matrix4x4", "subtract", 						matrix4x4_subtract);
	env.load_module_function("Matrix4x4", "multiply", 						matrix4x4_multiply);
	env.load_module_function("Matrix4x4", "build_rotation_x", 				matrix4x4_build_rotation_x);
	env.load_module_function("Matrix4x4", "build_rotation_y", 				matrix4x4_build_rotation_y);
	env.load_module_function("Matrix4x4", "build_rotation_z", 				matrix4x4_build_rotation_z);
	env.load_module_function("Matrix4x4", "build_rotation", 				matrix4x4_build_rotation);
	env.load_module_function("Matrix4x4", "build_look_at_rh", 				matrix4x4_build_look_at_rh);
	env.load_module_function("Matrix4x4", "build_viewpoint_billboard", 		matrix4x4_build_viewpoint_billboard);
	env.load_module_function("Matrix4x4", "build_axis_billboard", 			matrix4x4_build_axis_billboard);
	env.load_module_function("Matrix4x4", "transpose", 						matrix4x4_transpose);
	env.load_module_function("Matrix4x4", "determinant", 					matrix4x4_determinant);
	env.load_module_function("Matrix4x4", "invert", 						matrix4x4_invert);
	env.load_module_function("Matrix4x4", "load_identity", 					matrix4x4_load_identity);
	env.load_module_function("Matrix4x4", "x",								matrix4x4_x);
	env.load_module_function("Matrix4x4", "y",								matrix4x4_y);
	env.load_module_function("Matrix4x4", "z",								matrix4x4_z);
	env.load_module_function("Matrix4x4", "set_x",							matrix4x4_set_x);
	env.load_module_function("Matrix4x4", "set_y",							matrix4x4_set_y);
	env.load_module_function("Matrix4x4", "set_z",							matrix4x4_set_z);
	env.load_module_function("Matrix4x4", "translation", 					matrix4x4_translation);
	env.load_module_function("Matrix4x4", "set_translation", 				matrix4x4_set_translation);
	env.load_module_function("Matrix4x4", "get_scale", 						matrix4x4_get_scale);
	env.load_module_function("Matrix4x4", "set_scale", 						matrix4x4_set_scale);
	env.load_module_function("Matrix4x4", "print", 							matrix4x4_print);
}

} //namespace crown
