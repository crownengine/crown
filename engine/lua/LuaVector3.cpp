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

#include "Vector3.h"
#include "LuaStack.h"
#include "LuaEnvironment.h"

namespace crown
{

//-----------------------------------------------------------------------------
CE_EXPORT int vector3(lua_State* L)
{
	LuaStack stack(L);

	float x = stack.get_float(1);
	float y = stack.get_float(2);
	float z = stack.get_float(3);

	stack.push_vector3(Vector3(x, y, z));

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int vector3_x(lua_State* L)
{
	LuaStack stack(L);

	const Vector3& a = stack.get_vector3(1);

	stack.push_float(a.x);
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int vector3_y(lua_State* L)
{
	LuaStack stack(L);

	const Vector3& a = stack.get_vector3(1);

	stack.push_float(a.y);
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int vector3_z(lua_State* L)
{
	LuaStack stack(L);

	const Vector3& a = stack.get_vector3(1);

	stack.push_float(a.z);
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int vector3_set_x(lua_State* L)
{
	LuaStack stack(L);

	Vector3& a = stack.get_vector3(1);
	const float val = stack.get_float(2);
	
	a.x = val;
	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int vector3_set_y(lua_State* L)
{
	LuaStack stack(L);

	Vector3& a = stack.get_vector3(1);
	const float val = stack.get_float(2);
	
	a.y = val;
	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int vector3_set_z(lua_State* L)
{
	LuaStack stack(L);

	Vector3& a = stack.get_vector3(1);
	const float val = stack.get_float(2);
	
	a.z = val;
	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int vector3_values(lua_State* L)
{
	LuaStack stack(L);

	Vector3& a = stack.get_vector3(1);

	stack.push_float(a.x);
	stack.push_float(a.y);
	stack.push_float(a.z);

	return 3;
}

//-----------------------------------------------------------------------------
CE_EXPORT int vector3_add(lua_State* L)
{
	LuaStack stack(L);

	Vector3& a = stack.get_vector3(1);
	Vector3& b = stack.get_vector3(2);

	stack.push_vector3(a + b);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int vector3_subtract(lua_State* L)
{
	LuaStack stack(L);

	Vector3& a = stack.get_vector3(1);
	Vector3& b = stack.get_vector3(2);

	stack.push_vector3(a - b);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int vector3_multiply(lua_State* L)
{
	LuaStack stack(L);

	Vector3& a = stack.get_vector3(1);
	float b = stack.get_float(2);

	stack.push_vector3(a * b);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int vector3_divide(lua_State* L)
{
	LuaStack stack(L);

	Vector3& a = stack.get_vector3(1);
	float b = stack.get_float(2);

	stack.push_vector3(a / b);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int vector3_dot(lua_State* L)
{
	LuaStack stack(L);
	
	Vector3& a = stack.get_vector3(1);
	Vector3& b = stack.get_vector3(2);

	stack.push_float(a.dot(b));

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int vector3_cross(lua_State* L)
{
	LuaStack stack(L);
	
	Vector3& a = stack.get_vector3(1);
	Vector3& b = stack.get_vector3(2);

	stack.push_vector3(a.cross(b));

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int vector3_equal(lua_State* L)
{
	LuaStack stack(L);

	Vector3& a = stack.get_vector3(1);
	Vector3& b = stack.get_vector3(2);

	stack.push_bool(a == b);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int vector3_lower(lua_State* L)
{
	LuaStack stack(L);

	Vector3& a = stack.get_vector3(1);
	Vector3& b = stack.get_vector3(2);

	stack.push_bool(a < b);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int vector3_greater(lua_State* L)
{
	LuaStack stack(L);

	Vector3& a = stack.get_vector3(1);
	Vector3& b = stack.get_vector3(2);

	stack.push_bool(a > b);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int vector3_length(lua_State* L)
{
	LuaStack stack(L);

	Vector3& a = stack.get_vector3(1);

	stack.push_float(a.length());

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int vector3_squared_length(lua_State* L)
{
	LuaStack stack(L);

	Vector3& a = stack.get_vector3(1);

	stack.push_float(a.squared_length());

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int vector3_set_length(lua_State* L)
{
	LuaStack stack(L);

	Vector3& a = stack.get_vector3(1);
	float len = stack.get_float(2);

	a.set_length(len);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int vector3_normalize(lua_State* L)
{
	LuaStack stack(L);

	Vector3& a = stack.get_vector3(1);

	a.normalize();

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int vector3_get_distance_to(lua_State* L)
{
	LuaStack stack(L);

	Vector3& a = stack.get_vector3(1);
	Vector3& b = stack.get_vector3(2);

	stack.push_float(a.get_distance_to(b));

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int vector3_get_angle_between(lua_State* L)
{
	LuaStack stack(L);

	Vector3& a = stack.get_vector3(1);
	Vector3& b = stack.get_vector3(2);

	stack.push_float(a.get_angle_between(b));

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int vector3_zero(lua_State* L)
{
	LuaStack stack(L);

	Vector3& a = stack.get_vector3(1);

	a.zero();

	return 0;
}	

//-----------------------------------------------------------------------------
void load_vector3(LuaEnvironment& env)
{
	env.load_module_function("Vector3", "new", 				vector3);
	env.load_module_function("Vector3", "x", 				vector3_x);
	env.load_module_function("Vector3", "y", 				vector3_y);
	env.load_module_function("Vector3", "z", 				vector3_z);
	env.load_module_function("Vector3", "set_x", 			vector3_set_x);
	env.load_module_function("Vector3", "set_y", 			vector3_set_y);
	env.load_module_function("Vector3", "set_z", 			vector3_set_z);
	env.load_module_function("Vector3", "values", 			vector3_values);
	env.load_module_function("Vector3", "add", 				vector3_add);
	env.load_module_function("Vector3", "subtract", 		vector3_subtract);
	env.load_module_function("Vector3", "multiply", 		vector3_multiply);
	env.load_module_function("Vector3", "divide", 			vector3_divide);
	env.load_module_function("Vector3", "dot", 				vector3_dot);
	env.load_module_function("Vector3", "cross", 			vector3_cross);
	env.load_module_function("Vector3", "equal", 			vector3_equal);
	env.load_module_function("Vector3", "lower", 			vector3_lower);
	env.load_module_function("Vector3", "greater", 			vector3_greater);
	env.load_module_function("Vector3", "length", 			vector3_length);
	env.load_module_function("Vector3", "squared_length", 	vector3_squared_length);
	env.load_module_function("Vector3", "set_length", 		vector3_set_length);
	env.load_module_function("Vector3", "normalize", 		vector3_normalize);
	env.load_module_function("Vector3", "get_distance_to", 	vector3_get_distance_to);
	env.load_module_function("Vector3", "get_angle_between",vector3_get_angle_between);
	env.load_module_function("Vector3", "zero", 			vector3_zero);	
}

} // namespace crown