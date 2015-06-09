/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "vector3.h"
#include "lua_stack.h"
#include "lua_environment.h"
#include "lua_assert.h"

namespace crown
{

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

void load_vector3(LuaEnvironment& env)
{
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
}

} // namespace crown
