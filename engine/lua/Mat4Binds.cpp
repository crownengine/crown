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

#include "Mat4.h"
#include "Vec3.h"
#include "LuaStack.h"
#include "LuaEnvironment.h"
#include "OS.h"

namespace crown
{

//-----------------------------------------------------------------------------
CE_EXPORT int32_t mat4(lua_State* L)
{
	LuaStack stack(L);

	float m0 = stack.get_float(1);
	float m1 = stack.get_float(2);
	float m2 = stack.get_float(3);
	float m4 = stack.get_float(4);	
	float m5 = stack.get_float(5);
	float m6 = stack.get_float(6);	
	float m8 = stack.get_float(7);
	float m9 = stack.get_float(8);
	float m10 = stack.get_float(9);

	Mat4* mat = next_mat4();

	mat->m[0] = m0;
	mat->m[1] = m1;
	mat->m[2] = m2;
	mat->m[4] = m4;
	mat->m[5] = m5;
	mat->m[6] = m6;
	mat->m[8] = m8;
	mat->m[9] = m9;
	mat->m[10] = m10;

	stack.push_mat4(mat);

	return 1;
}

//-----------------------------------------------------------------------------					
CE_EXPORT int32_t mat4_add(lua_State* L)
{
	LuaStack stack(L);

	Mat4* a = stack.get_mat4(1);
	Mat4* b = stack.get_mat4(2);

	*a += *b;

	stack.push_mat4(a);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t mat4_subtract(lua_State* L)
{
	LuaStack stack(L);

	Mat4* a = (Mat4*)stack.get_mat4(1);
	Mat4* b = (Mat4*)stack.get_mat4(2);

	*a -= *b;

	stack.push_mat4(a);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t mat4_multiply(lua_State* L)
{
	LuaStack stack(L);

	Mat4* a = stack.get_mat4(1);
	Mat4* b = stack.get_mat4(2);

	*a *= *b;

	stack.push_mat4(a);

	return 1;
}	

//-----------------------------------------------------------------------------
CE_EXPORT int32_t mat4_multiply_by_scalar(lua_State* L)
{
	LuaStack stack(L);

	Mat4* a = (Mat4*)stack.get_mat4(1);
	float k = stack.get_float(2);

	*a *= k;

	stack.push_mat4(a);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t mat4_divide_by_scalar(lua_State* L)
{
	LuaStack stack(L);

	Mat4* a = (Mat4*)stack.get_mat4(1);
	float k = stack.get_float(2);

	*a /= k;

	stack.push_mat4(a);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t mat4_build_rotation_x(lua_State* L)
{
	LuaStack stack(L);

	Mat4* a = (Mat4*)stack.get_mat4(1);
	float k = stack.get_float(2);

	a->build_rotation_x(k);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t mat4_build_rotation_y(lua_State* L)
{
	LuaStack stack(L);

	Mat4* a = stack.get_mat4(1);
	float k = stack.get_float(2);

	a->build_rotation_y(k);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t mat4_build_rotation_z(lua_State* L)
{
	LuaStack stack(L);

	Mat4* a = (Mat4*)stack.get_mat4(1);
	float k = stack.get_float(2);

	a->build_rotation_z(k);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t mat4_build_rotation(lua_State* L)
{
	LuaStack stack(L);

	Mat4* a = (Mat4*)stack.get_mat4(1);
	Vec3* d = (Vec3*)stack.get_vec3(2);
	float k = stack.get_float(3);

	a->build_rotation(*d, k);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t mat4_build_projection_perspective_rh(lua_State* L)
{
	LuaStack stack(L);

	Mat4* a = stack.get_mat4(1);
	float fovy = stack.get_float(2);
	float aspect = stack.get_float(3);
	float near = stack.get_float(4);
	float far = stack.get_float(5);

	a->build_projection_perspective_rh(fovy, aspect, near, far);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t mat4_build_projection_perspective_lh(lua_State* L)
{
	LuaStack stack(L);

	Mat4* a = stack.get_mat4(1);
	float fovy = stack.get_float(2);
	float aspect = stack.get_float(3);
	float near = stack.get_float(4);
	float far = stack.get_float(5);

	a->build_projection_perspective_lh(fovy, aspect, near, far);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t mat4_build_projection_ortho_rh(lua_State* L)
{
	LuaStack stack(L);

	Mat4* a = stack.get_mat4(1);
	float width = stack.get_float(2);
	float height = stack.get_float(3);
	float near = stack.get_float(4);
	float far = stack.get_float(5);

	a->build_projection_ortho_rh(width, height, near, far);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t mat4_build_projection_ortho_lh(lua_State* L)
{
	LuaStack stack(L);

	Mat4* a = stack.get_mat4(1);
	float width = stack.get_float(2);
	float height = stack.get_float(3);
	float near = stack.get_float(4);
	float far = stack.get_float(5);

	a->build_projection_ortho_lh(width, height, near, far);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t mat4_build_projection_ortho_2d_rh(lua_State* L)
{
	LuaStack stack(L);

	Mat4* a = stack.get_mat4(1);
	float width = stack.get_float(2);
	float height = stack.get_float(3);
	float near = stack.get_float(4);
	float far = stack.get_float(5);

	a->build_projection_ortho_2d_rh(width, height, near, far);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t mat4_build_look_at_rh(lua_State* L)
{
	LuaStack stack(L);

	Mat4* a = stack.get_mat4(1);
	Vec3* pos = stack.get_vec3(2);
	Vec3* target = stack.get_vec3(3);
	Vec3* up = stack.get_vec3(4);

	a->build_look_at_rh(*pos, *target, *up);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t mat4_build_look_at_lh(lua_State* L)
{
	LuaStack stack(L);

	Mat4* a = stack.get_mat4(1);
	Vec3* pos = stack.get_vec3(2);
	Vec3* target = stack.get_vec3(3);
	Vec3* up = stack.get_vec3(4);

	a->build_look_at_lh(*pos, *target, *up);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t mat4_build_viewpoint_billboard(lua_State* L)
{
	LuaStack stack(L);

	Mat4* a = stack.get_mat4(1);
	Vec3* pos = stack.get_vec3(2);
	Vec3* target = stack.get_vec3(3);
	Vec3* up = stack.get_vec3(4);

	a->build_viewpoint_billboard(*pos, *target, *up);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t mat4_build_axis_billboard(lua_State* L)
{
	LuaStack stack(L);

	Mat4* a = stack.get_mat4(1);
	Vec3* pos = stack.get_vec3(2);
	Vec3* target = stack.get_vec3(3);
	Vec3* up = stack.get_vec3(4);

	a->build_axis_billboard(*pos, *target, *up);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t mat4_transpose(lua_State* L)
{
	LuaStack stack(L);

	Mat4* a = stack.get_mat4(1);

	a->transpose();

	stack.push_mat4(a);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t mat4_determinant(lua_State* L)
{
	LuaStack stack(L);

	Mat4* a = stack.get_mat4(1);

	stack.push_float(a->get_determinant());

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t mat4_invert(lua_State* L)
{
	LuaStack stack(L);

	Mat4* a = stack.get_mat4(1);

	a->invert();

	stack.push_mat4(a);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t mat4_load_identity(lua_State* L)
{
	LuaStack stack(L);

	Mat4* a = stack.get_mat4(1);

	a->load_identity();

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t mat4_get_translation(lua_State* L)
{	
	LuaStack stack(L);

	Mat4* a = stack.get_mat4(1);

	Vec3* translation = next_vec3();
	*translation = a->get_translation();

	stack.push_vec3(translation);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t mat4_set_translation(lua_State* L)
{
	LuaStack stack(L);

	Mat4* a = stack.get_mat4(1);
	Vec3* trans = stack.get_vec3(2);

	a->set_translation(*trans);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t mat4_get_scale(lua_State* L)
{
	LuaStack stack(L);

	Mat4* a = stack.get_mat4(1);

	Vec3* scale = next_vec3();
	*scale = a->get_scale();

	stack.push_vec3(scale);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t mat4_set_scale(lua_State* L)
{
	LuaStack stack(L);

	Mat4* a = stack.get_mat4(1);
	Vec3* scale = stack.get_vec3(2);

	a->set_scale(*scale);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t mat4_print(lua_State* L)
{
	LuaStack stack(L);

	Mat4* a = stack.get_mat4(1);

	os::printf("|%.1f|%.1f|%.1f|%.1f|\n", a->m[0], a->m[4], a->m[8], a->m[12]);
	os::printf("|%.1f|%.1f|%.1f|%.1f|\n", a->m[1], a->m[5], a->m[9], a->m[13]);
	os::printf("|%.1f|%.1f|%.1f|%.1f|\n", a->m[2], a->m[6], a->m[10], a->m[14]);
	os::printf("|%.1f|%.1f|%.1f|%.1f|\n", a->m[3], a->m[7], a->m[11], a->m[15]);

	return 0;
}

//-----------------------------------------------------------------------------
void load_mat4(LuaEnvironment& env)
{
	env.load_module_function("Mat4", "new", 							mat4);
	env.load_module_function("Mat4", "add", 							mat4_add);
	env.load_module_function("Mat4", "sub", 							mat4_subtract);
	env.load_module_function("Mat4", "mul", 							mat4_multiply);
	env.load_module_function("Mat4", "muls", 							mat4_multiply_by_scalar);
	env.load_module_function("Mat4", "divs", 							mat4_divide_by_scalar);
	env.load_module_function("Mat4", "build_rotation_x", 				mat4_build_rotation_x);
	env.load_module_function("Mat4", "build_rotation_y", 				mat4_build_rotation_y);
	env.load_module_function("Mat4", "build_rotation_z", 				mat4_build_rotation_z);
	env.load_module_function("Mat4", "build_rotation", 					mat4_build_rotation);
	env.load_module_function("Mat4", "build_projection_perspective_rh", mat4_build_projection_perspective_rh);
	env.load_module_function("Mat4", "build_projection_perspective_lh", mat4_build_projection_perspective_lh);
	env.load_module_function("Mat4", "build_projection_ortho_rh", 		mat4_build_projection_ortho_rh);
	env.load_module_function("Mat4", "build_projection_ortho_lh", 		mat4_build_projection_ortho_lh);
	env.load_module_function("Mat4", "build_projection_ortho_2d_rh", 	mat4_build_projection_ortho_2d_rh);
	env.load_module_function("Mat4", "build_look_at_rh", 				mat4_build_look_at_rh);
	env.load_module_function("Mat4", "build_look_at_lh", 				mat4_build_look_at_rh);
	env.load_module_function("Mat4", "build_viewpoint_billboard", 		mat4_build_viewpoint_billboard);
	env.load_module_function("Mat4", "build_axis_billboard", 			mat4_build_axis_billboard);
	env.load_module_function("Mat4", "transpose", 						mat4_transpose);
	env.load_module_function("Mat4", "determinant", 					mat4_determinant);
	env.load_module_function("Mat4", "invert", 							mat4_invert);
	env.load_module_function("Mat4", "load_identity", 					mat4_load_identity);
	env.load_module_function("Mat4", "get_translation", 				mat4_get_translation);
	env.load_module_function("Mat4", "set_translation", 				mat4_set_translation);
	env.load_module_function("Mat4", "get_scale", 						mat4_get_scale);
	env.load_module_function("Mat4", "set_scale", 						mat4_set_scale);
	env.load_module_function("Mat4", "print", 							mat4_print);

}


} //namespace crown