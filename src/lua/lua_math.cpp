/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "math_utils.h"
#include "math_types.h"
#include "vector3.h"
#include "matrix4x4.h"
#include "quaternion.h"
#include "color4.h"
#include "intersection.h"
#include "lua_stack.h"
#include "lua_environment.h"

namespace crown
{

static int math_to_rad(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(to_rad(stack.get_float(1)));
	return 1;
}

static int math_to_deg(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(to_deg(stack.get_float(1)));
	return 1;
}

static int math_next_pow_2(lua_State* L)
{
	LuaStack stack(L);
	stack.push_uint32(next_pow_2(stack.get_int(1)));
	return 1;
}

static int math_is_pow_2(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(is_pow_2(stack.get_int(1)));
	return 1;
}

static int math_ceil(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(ceil(stack.get_float(1)));
	return 1;
}

static int math_floor(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(floor(stack.get_float(1)));
	return 1;
}

static int math_sqrt(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(sqrt(stack.get_float(1)));
	return 1;
}

static int math_inv_sqrt(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(inv_sqrt(stack.get_float(1)));
	return 1;
}

static int math_sin(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(sin(stack.get_float(1)));
	return 1;
}

static int math_cos(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(cos(stack.get_float(1)));
	return 1;
}

static int math_asin(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(asin(stack.get_float(1)));
	return 1;
}

static int math_acos(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(acos(stack.get_float(1)));
	return 1;
}

static int math_tan(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(tan(stack.get_float(1)));
	return 1;
}

static int math_atan2(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(atan2(stack.get_float(1), stack.get_float(2)));
	return 1;
}

static int math_abs(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(abs(stack.get_float(1)));
	return 1;
}

static int math_fmod(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(fmod(stack.get_float(1), stack.get_float(2)));
	return 1;
}

static int math_ray_obb_intersection(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(ray_obb_intersection(stack.get_vector3(1)
		, stack.get_vector3(2)
		, stack.get_matrix4x4(3)
		, stack.get_vector3(4)));
	return 1;
}

static int vector3_new(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(vector3(stack.get_float(1), stack.get_float(2), stack.get_float(3)));
	return 1;
}

static int vector3_ctor(lua_State* L)
{
	LuaStack stack(L);
	stack.remove(1); // Remove table
	return vector3_new(L);
}

static int vector3_x(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_vector3(1).x);
	return 1;
}

static int vector3_y(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_vector3(1).y);
	return 1;
}

static int vector3_z(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_vector3(1).z);
	return 1;
}

static int vector3_set_x(lua_State* L)
{
	LuaStack stack(L);
	stack.get_vector3(1).x = stack.get_float(2);
	return 0;
}

static int vector3_set_y(lua_State* L)
{
	LuaStack stack(L);
	stack.get_vector3(1).y = stack.get_float(2);
	return 0;
}

static int vector3_set_z(lua_State* L)
{
	LuaStack stack(L);
	stack.get_vector3(1).z = stack.get_float(2);
	return 0;
}

static int vector3_values(lua_State* L)
{
	LuaStack stack(L);
	Vector3& a = stack.get_vector3(1);
	stack.push_float(a.x);
	stack.push_float(a.y);
	stack.push_float(a.z);
	return 3;
}

static int vector3_add(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_vector3(1) + stack.get_vector3(2));
	return 1;
}

static int vector3_subtract(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_vector3(1) - stack.get_vector3(2));
	return 1;
}

static int vector3_multiply(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_vector3(1) * stack.get_float(2));
	return 1;
}

static int vector3_divide(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_vector3(1) / stack.get_float(2));
	return 1;
}

static int vector3_dot(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(dot(stack.get_vector3(1), stack.get_vector3(2)));
	return 1;
}

static int vector3_cross(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(cross(stack.get_vector3(1), stack.get_vector3(2)));
	return 1;
}

static int vector3_equal(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_vector3(1) == stack.get_vector3(2));
	return 1;
}

static int vector3_length(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(length(stack.get_vector3(1)));
	return 1;
}

static int vector3_squared_length(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(squared_length(stack.get_vector3(1)));
	return 1;
}

static int vector3_set_length(lua_State* L)
{
	LuaStack stack(L);
	set_length(stack.get_vector3(1), stack.get_float(2));
	return 0;
}

static int vector3_normalize(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(normalize(stack.get_vector3(1)));
	return 1;
}

static int vector3_distance(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(distance(stack.get_vector3(1), stack.get_vector3(2)));
	return 1;
}

static int vector3_angle(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(angle(stack.get_vector3(1), stack.get_vector3(2)));
	return 1;
}

static int vector3_forward(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(VECTOR3_FORWARD);
	return 1;
}

static int vector3_backward(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(VECTOR3_BACKWARD);
	return 1;
}

static int vector3_left(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(VECTOR3_LEFT);
	return 1;
}

static int vector3_right(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(VECTOR3_RIGHT);
	return 1;
}

static int vector3_up(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(VECTOR3_UP);
	return 1;
}

static int vector3_down(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(VECTOR3_DOWN);
	return 1;
}

static int vector2_new(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector2(vector2(stack.get_float(1), stack.get_float(2)));
	return 1;
}

static int vector2_ctor(lua_State* L)
{
	LuaStack stack(L);
	stack.remove(1); // Remove table
	return vector2_new(L);
}

static int vector3box_new(lua_State* L)
{
	LuaStack stack(L);

	if (stack.num_args() == 0)
		stack.push_vector3box(Vector3());
	else if (stack.num_args() == 1)
		stack.push_vector3box(stack.get_vector3(1));
	else
		stack.push_vector3box(vector3(stack.get_float(1)
			, stack.get_float(2)
			, stack.get_float(3)));

	return 1;
}

static int vector3box_ctor(lua_State* L)
{
	LuaStack stack(L);
	stack.remove(1); // Remove table
	return vector3box_new(L);
}

static int vector3box_store(lua_State* L)
{
	LuaStack stack(L);

	Vector3& v = stack.get_vector3box(1);

	if (stack.num_args() == 2)
		v = stack.get_vector3(2);
	else
		v = vector3(stack.get_float(2)
			, stack.get_float(3)
			, stack.get_float(4));

	return 0;
}

static int vector3box_unbox(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_vector3box(1));
	return 1;
}

static int vector3box_get_value(lua_State* L)
{
	LuaStack stack(L);

	Vector3& v = stack.get_vector3box(1);
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

static int vector3box_set_value(lua_State* L)
{
	LuaStack stack(L);

	Vector3& v = stack.get_vector3box(1);
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

static int vector3box_tostring(lua_State* L)
{
	LuaStack stack(L);
	Vector3& v = stack.get_vector3box(1);
	stack.push_fstring("Vector3Box (%p)", &v);
	return 1;
}

static int matrix4x4_new(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(matrix4x4(stack.get_float(1)
		, stack.get_float(2)
		, stack.get_float(3)
		, stack.get_float(4)
		, stack.get_float(5)
		, stack.get_float(6)
		, stack.get_float(7)
		, stack.get_float(8)
		, stack.get_float(9)
		, stack.get_float(10)
		, stack.get_float(11)
		, stack.get_float(12)
		, stack.get_float(13)
		, stack.get_float(14)
		, stack.get_float(15)
		, stack.get_float(16)));
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
	stack.push_matrix4x4(matrix4x4(stack.get_quaternion(1), vector3(0, 0, 0)));
	return 1;
}

static int matrix4x4_from_translation(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(matrix4x4(QUATERNION_IDENTITY, stack.get_vector3(1)));
	return 1;
}

static int matrix4x4_from_quaternion_translation(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(matrix4x4(stack.get_quaternion(1), stack.get_vector3(2)));
	return 1;
}

static int matrix4x4_from_axes(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(matrix4x4(stack.get_vector3(1), stack.get_vector3(2), stack.get_vector3(3), stack.get_vector3(4)));
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
	stack.push_matrix4x4(MATRIX4X4_IDENTITY);
	return 1;
}

static int matrix4x4_to_string(lua_State* L)
{
	LuaStack stack(L);
	Matrix4x4& a = stack.get_matrix4x4(1);
	stack.push_fstring("%.1f, %.1f, %.1f, %.1f\n%.1f, %.1f, %.1f, %.1f\n%.1f, %.1f, %.1f, %.1f\n%.1f, %.1f, %.1f, %.1f\n",
						a.x.x, a.x.y, a.x.z, a.y.w, a.y.x, a.y.y, a.y.z, a.y.w, a.z.x, a.z.y, a.z.z, a.z.w, a.t.x, a.t.y, a.t.z, a.t.w);
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

static int quaternion_new(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(quaternion(stack.get_vector3(1), stack.get_float(2)));
	return 1;
}

static int quaternion_ctor(lua_State* L)
{
	LuaStack stack(L);
	stack.remove(1); // Remove table
	return quaternion_new(L);
}

static int quaternion_negate(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(-stack.get_quaternion(1));
	return 1;
}

static int quaternion_identity(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(QUATERNION_IDENTITY);
	return 1;
}

static int quaternion_length(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(length(stack.get_quaternion(1)));
	return 1;
}

static int quaternion_normalize(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(normalize(stack.get_quaternion(1)));
	return 1;
}

static int quaternion_conjugate(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(conjugate(stack.get_quaternion(1)));
	return 1;
}

static int quaternion_inverse(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(inverse(stack.get_quaternion(1)));
	return 1;
}

static int quaternion_multiply(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(stack.get_quaternion(1) * stack.get_quaternion(2));
	return 1;
}

static int quaternion_multiply_by_scalar(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(stack.get_quaternion(1) * stack.get_float(2));
	return 1;
}

static int quaternion_power(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(power(stack.get_quaternion(1), stack.get_float(2)));
	return 1;
}

static int quaternion_elements(lua_State* L)
{
	LuaStack stack(L);
	const Quaternion& q = stack.get_quaternion(1);
	stack.push_float(q.x);
	stack.push_float(q.y);
	stack.push_float(q.z);
	stack.push_float(q.w);
	return 4;
}

static int quaternionbox_new(lua_State* L)
{
	LuaStack stack(L);

	if (stack.num_args() == 1)
		stack.push_quaternionbox(stack.get_quaternion(1));
	else
		stack.push_quaternionbox(quaternion(stack.get_float(1)
			, stack.get_float(2)
			, stack.get_float(3)
			, stack.get_float(4)));

	return 1;
}

static int quaternionbox_ctor(lua_State* L)
{
	LuaStack stack(L);
	stack.remove(1); // Remove table
	return quaternionbox_new(L);
}

static int quaternionbox_store(lua_State* L)
{
	LuaStack stack(L);

	Quaternion& q = stack.get_quaternionbox(1);

	if (stack.num_args() == 2)
		q = stack.get_quaternion(2);
	else
		q = quaternion(stack.get_float(2)
			, stack.get_float(3)
			, stack.get_float(4)
			, stack.get_float(5));

	return 0;
}

static int quaternionbox_unbox(lua_State* L)
{
	LuaStack stack(L);

	Quaternion& q = stack.get_quaternionbox(1);

	stack.push_quaternion(q);
	return 1;
}

static int quaternionbox_tostring(lua_State* L)
{
	LuaStack stack(L);
	Quaternion& q = stack.get_quaternionbox(1);
	stack.push_fstring("QuaternionBox (%p)", &q);
	return 1;
}

static int color4_new(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(quaternion(stack.get_float(1)
		, stack.get_float(2)
		, stack.get_float(3)
		, stack.get_float(4)));
	return 1;
}

static int color4_ctor(lua_State* L)
{
	LuaStack stack(L);
	stack.remove(1); // Remove table
	return color4_new(L);
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

void load_math(LuaEnvironment& env)
{
	env.load_module_function("Math", "to_rad",               math_to_rad);
	env.load_module_function("Math", "to_deg",               math_to_deg);
	env.load_module_function("Math", "next_pow_2",           math_next_pow_2);
	env.load_module_function("Math", "is_pow_2",             math_is_pow_2);
	env.load_module_function("Math", "ceil",                 math_ceil);
	env.load_module_function("Math", "floor",                math_floor);
	env.load_module_function("Math", "sqrt",                 math_sqrt);
	env.load_module_function("Math", "inv_sqrt",             math_inv_sqrt);
	env.load_module_function("Math", "sin",                  math_sin);
	env.load_module_function("Math", "cos",                  math_cos);
	env.load_module_function("Math", "asin",                 math_asin);
	env.load_module_function("Math", "acos",                 math_acos);
	env.load_module_function("Math", "tan",                  math_tan);
	env.load_module_function("Math", "atan2",                math_atan2);
	env.load_module_function("Math", "abs",                  math_abs);
	env.load_module_function("Math", "fmod",                 math_fmod);
	env.load_module_function("Math", "ray_obb_intersection", math_ray_obb_intersection);

	env.load_module_function("Vector3", "new",            vector3_new);
	env.load_module_function("Vector3", "x",              vector3_x);
	env.load_module_function("Vector3", "y",              vector3_y);
	env.load_module_function("Vector3", "z",              vector3_z);
	env.load_module_function("Vector3", "set_x",          vector3_set_x);
	env.load_module_function("Vector3", "set_y",          vector3_set_y);
	env.load_module_function("Vector3", "set_z",          vector3_set_z);
	env.load_module_function("Vector3", "values",         vector3_values);
	env.load_module_function("Vector3", "add",            vector3_add);
	env.load_module_function("Vector3", "subtract",       vector3_subtract);
	env.load_module_function("Vector3", "multiply",       vector3_multiply);
	env.load_module_function("Vector3", "divide",         vector3_divide);
	env.load_module_function("Vector3", "dot",            vector3_dot);
	env.load_module_function("Vector3", "cross",          vector3_cross);
	env.load_module_function("Vector3", "equal",          vector3_equal);
	env.load_module_function("Vector3", "length",         vector3_length);
	env.load_module_function("Vector3", "squared_length", vector3_squared_length);
	env.load_module_function("Vector3", "set_length",     vector3_set_length);
	env.load_module_function("Vector3", "normalize",      vector3_normalize);
	env.load_module_function("Vector3", "distance",       vector3_distance);
	env.load_module_function("Vector3", "angle",          vector3_angle);
	env.load_module_function("Vector3", "forward",        vector3_forward);
	env.load_module_function("Vector3", "backward",       vector3_backward);
	env.load_module_function("Vector3", "left",           vector3_left);
	env.load_module_function("Vector3", "right",          vector3_right);
	env.load_module_function("Vector3", "up",             vector3_up);
	env.load_module_function("Vector3", "down",           vector3_down);

	env.load_module_constructor("Vector3", vector3_ctor);

	env.load_module_function("Vector2", "new",            vector2_new);
	env.load_module_constructor("Vector2", vector2_ctor);

	env.load_module_function("Vector3Box", "new",        vector3box_new);
	env.load_module_function("Vector3Box", "store",      vector3box_store);
	env.load_module_function("Vector3Box", "unbox",      vector3box_unbox);
	env.load_module_function("Vector3Box", "__index",    vector3box_get_value);
	env.load_module_function("Vector3Box", "__newindex", vector3box_set_value);
	env.load_module_function("Vector3Box", "__tostring", vector3box_tostring);

	env.load_module_constructor("Vector3Box", vector3box_ctor);

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

	env.load_module_function("Quaternion", "new",                quaternion_new);
	env.load_module_function("Quaternion", "negate",             quaternion_negate);
	env.load_module_function("Quaternion", "identity",           quaternion_identity);
	env.load_module_function("Quaternion", "multiply",           quaternion_multiply);
	env.load_module_function("Quaternion", "multiply_by_scalar", quaternion_multiply_by_scalar);
	env.load_module_function("Quaternion", "length",             quaternion_length);
	env.load_module_function("Quaternion", "normalize",          quaternion_normalize);
	env.load_module_function("Quaternion", "conjugate",          quaternion_conjugate);
	env.load_module_function("Quaternion", "inverse",            quaternion_inverse);
	env.load_module_function("Quaternion", "power",              quaternion_power);
	env.load_module_function("Quaternion", "elements",           quaternion_elements);

	env.load_module_constructor("Quaternion", quaternion_ctor);

	env.load_module_function("QuaternionBox", "new",        quaternionbox_new);
	env.load_module_function("QuaternionBox", "store",      quaternionbox_store);
	env.load_module_function("QuaternionBox", "unbox",      quaternionbox_unbox);
	env.load_module_function("QuaternionBox", "__tostring", quaternionbox_tostring);

	env.load_module_constructor("QuaternionBox", quaternionbox_ctor);

	env.load_module_function("Color4", "new", color4_new);
	env.load_module_constructor("Color4",     color4_ctor);

	env.load_module_function("Lightuserdata_mt", "__add", lightuserdata_add);
	env.load_module_function("Lightuserdata_mt", "__sub", lightuserdata_sub);
	env.load_module_function("Lightuserdata_mt", "__mul", lightuserdata_mul);
	env.load_module_function("Lightuserdata_mt", "__div", lightuserdata_div);
	env.load_module_function("Lightuserdata_mt", "__unm", lightuserdata_unm);
	env.load_module_function("Lightuserdata_mt", "__index", lightuserdata_index);
	env.load_module_function("Lightuserdata_mt", "__newindex", lightuserdata_newindex);
}

} // namespace crown
