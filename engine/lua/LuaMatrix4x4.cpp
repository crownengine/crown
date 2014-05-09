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

namespace crown
{

using namespace matrix4x4;

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
static int matrix4x4_ctor(lua_State* L)
{
	LuaStack stack(L);
	stack.remove(1); // Remove table
	return matrix4x4_new(L);
}

//-----------------------------------------------------------------------------
static int matrix4x4_from_quaternion(lua_State* L)
{
	LuaStack stack(L);
	const Quaternion& q = stack.get_quaternion(1);
	stack.push_matrix4x4(Matrix4x4(q, Vector3(0, 0, 0)));
	return 1;
}

//-----------------------------------------------------------------------------
static int matrix4x4_from_translation(lua_State* L)
{
	LuaStack stack(L);
	const Vector3& t = stack.get_vector3(1);
	stack.push_matrix4x4(Matrix4x4(quaternion::IDENTITY, t));
	return 1;
}

//-----------------------------------------------------------------------------
static int matrix4x4_from_quaternion_translation(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(Matrix4x4(stack.get_quaternion(1), stack.get_vector3(2)));
	return 1;
}

//-----------------------------------------------------------------------------
static int matrix4x4_from_axes(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(Matrix4x4(stack.get_vector3(1), stack.get_vector3(2), stack.get_vector3(3), stack.get_vector3(4)));
	return 1;
}

//-----------------------------------------------------------------------------					
static int matrix4x4_add(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);
	Matrix4x4& b = stack.get_matrix4x4(2);

	stack.push_matrix4x4(a + b);

	return 1;
}

//-----------------------------------------------------------------------------
static int matrix4x4_subtract(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);
	Matrix4x4& b = stack.get_matrix4x4(2);

	stack.push_matrix4x4(a - b);

	return 1;
}

//-----------------------------------------------------------------------------
static int matrix4x4_multiply(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);
	Matrix4x4& b = stack.get_matrix4x4(2);

	stack.push_matrix4x4(a * b);

	return 1;
}

//-----------------------------------------------------------------------------
static int matrix4x4_transpose(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);

	stack.push_matrix4x4(transpose(a));

	return 1;
}

//-----------------------------------------------------------------------------
static int matrix4x4_determinant(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);

	stack.push_float(determinant(a));

	return 1;
}

//-----------------------------------------------------------------------------
static int matrix4x4_invert(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);

	stack.push_matrix4x4(invert(a));

	return 1;
}

//-----------------------------------------------------------------------------
static int matrix4x4_x(lua_State* L)
{	
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);

	stack.push_vector3(x(a));

	return 1;
}

//-----------------------------------------------------------------------------
static int matrix4x4_y(lua_State* L)
{	
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);

	stack.push_vector3(y(a));

	return 1;
}

//-----------------------------------------------------------------------------
static int matrix4x4_z(lua_State* L)
{	
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);

	stack.push_vector3(z(a));

	return 1;
}

//-----------------------------------------------------------------------------
static int matrix4x4_set_x(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);
	Vector3& x = stack.get_vector3(2);

	set_x(a, x);

	return 0;
}

//-----------------------------------------------------------------------------
static int matrix4x4_set_y(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);
	Vector3& y = stack.get_vector3(2);

	set_y(a, y);

	return 0;
}

//-----------------------------------------------------------------------------
static int matrix4x4_set_z(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);
	Vector3& z = stack.get_vector3(2);

	set_z(a, z);

	return 0;
}

//-----------------------------------------------------------------------------
static int matrix4x4_translation(lua_State* L)
{	
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);

	stack.push_vector3(translation(a));

	return 1;
}

//-----------------------------------------------------------------------------
static int matrix4x4_set_translation(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);
	Vector3& trans = stack.get_vector3(2);

	set_translation(a, trans);

	return 0;
}

//-----------------------------------------------------------------------------
static int matrix4x4_identity(lua_State* L)
{
	LuaStack stack(L);

	stack.push_matrix4x4(matrix4x4::IDENTITY);

	return 1;
}

//-----------------------------------------------------------------------------
static int matrix4x4_to_string(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& a = stack.get_matrix4x4(1);

	stack.push_fstring("%.1f, %.1f, %.1f, %.1f\n%.1f, %.1f, %.1f, %.1f\n%.1f, %.1f, %.1f, %.1f\n%.1f, %.1f, %.1f, %.1f\n",
						a[0], a[4], a[8], a[12], a[1], a[5], a[9], a[13], a[2], a[6], a[10], a[14], a[3], a[7], a[11], a[15]);

	return 1;
}

//-----------------------------------------------------------------------------
void load_matrix4x4(LuaEnvironment& env)
{
	env.load_module_function("Matrix4x4", "new", 							matrix4x4_new);
	env.load_module_function("Matrix4x4", "from_quaternion",				matrix4x4_from_quaternion);
	env.load_module_function("Matrix4x4", "from_translation",				matrix4x4_from_translation);
	env.load_module_function("Matrix4x4", "from_quaternion_translation",	matrix4x4_from_quaternion_translation);
	env.load_module_function("Matrix4x4", "from_axes",						matrix4x4_from_axes);
	env.load_module_function("Matrix4x4", "add", 							matrix4x4_add);
	env.load_module_function("Matrix4x4", "subtract", 						matrix4x4_subtract);
	env.load_module_function("Matrix4x4", "multiply", 						matrix4x4_multiply);
	env.load_module_function("Matrix4x4", "transpose", 						matrix4x4_transpose);
	env.load_module_function("Matrix4x4", "determinant", 					matrix4x4_determinant);
	env.load_module_function("Matrix4x4", "invert", 						matrix4x4_invert);
	env.load_module_function("Matrix4x4", "x",								matrix4x4_x);
	env.load_module_function("Matrix4x4", "y",								matrix4x4_y);
	env.load_module_function("Matrix4x4", "z",								matrix4x4_z);
	env.load_module_function("Matrix4x4", "set_x",							matrix4x4_set_x);
	env.load_module_function("Matrix4x4", "set_y",							matrix4x4_set_y);
	env.load_module_function("Matrix4x4", "set_z",							matrix4x4_set_z);
	env.load_module_function("Matrix4x4", "translation", 					matrix4x4_translation);
	env.load_module_function("Matrix4x4", "set_translation", 				matrix4x4_set_translation);
	env.load_module_function("Matrix4x4", "identity", 						matrix4x4_identity);
	env.load_module_function("Matrix4x4", "to_string",						matrix4x4_to_string);

	env.load_module_constructor("Matrix4x4",								matrix4x4_ctor);
}

} //namespace crown
