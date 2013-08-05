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

#include "Vec2.h"
#include "LuaStack.h"
#include "LuaEnvironment.h"

namespace crown
{

//-----------------------------------------------------------------------------
CE_EXPORT int32_t vec2(lua_State* L)
{
	LuaStack stack(L);

	float x = stack.get_float(1);
	float y = stack.get_float(2);

	stack.push_vec2(Vec2(x, y));

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t vec2_values(lua_State* L)
{
	LuaStack stack(L);

	Vec2& a = stack.get_vec2(1);

	stack.push_float(a.x);
	stack.push_float(a.y);

	return 2;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t vec2_add(lua_State* L)
{
	LuaStack stack(L);

	Vec2& a = stack.get_vec2(1);
	Vec2& b = stack.get_vec2(2);

	stack.push_vec2(a + b);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t vec2_subtract(lua_State* L)
{
	LuaStack stack(L);

	Vec2& a = stack.get_vec2(1);
	Vec2& b = stack.get_vec2(2);

	stack.push_vec2(a - b);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t vec2_multiply(lua_State* L)
{
	LuaStack stack(L);

	Vec2& a = stack.get_vec2(1);
	float k = stack.get_float(2);

	stack.push_vec2(a * k);

	return 1;
}			

//-----------------------------------------------------------------------------
CE_EXPORT int32_t vec2_divide(lua_State* L)
{
	LuaStack stack(L);
	
	Vec2& a = stack.get_vec2(1);
	float k = stack.get_float(2);

	stack.push_vec2(a / k);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t vec2_dot(lua_State* L)
{
	LuaStack stack(L);
	
	Vec2& a = stack.get_vec2(1);
	Vec2& b = stack.get_vec2(2);

	stack.push_float(a.dot(b));

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t vec2_equals(lua_State* L)
{
	LuaStack stack(L);
	
	Vec2& a = stack.get_vec2(1);
	Vec2& b = stack.get_vec2(2);

	stack.push_bool(a == b);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t vec2_lower(lua_State* L)
{
	LuaStack stack(L);
	
	Vec2& a = stack.get_vec2(1);
	Vec2& b = stack.get_vec2(2);

	stack.push_bool(a < b);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t vec2_greater(lua_State* L)
{
	LuaStack stack(L);
	
	Vec2& a = stack.get_vec2(1);
	Vec2& b = stack.get_vec2(2);
	
	stack.push_bool(a > b);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t vec2_length(lua_State* L)
{
	LuaStack stack(L);

	Vec2& a = stack.get_vec2(1);

	stack.push_float(a.length());

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t vec2_squared_length(lua_State* L)
{
	LuaStack stack(L);

	Vec2& a = stack.get_vec2(1);

	stack.push_float(a.squared_length());

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t vec2_set_length(lua_State* L)
{
	LuaStack stack(L);

	Vec2& a = stack.get_vec2(1);
	float len = stack.get_float(2);

	a.set_length(len);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t vec2_normalize(lua_State* L)
{
	LuaStack stack(L);

	Vec2& a = stack.get_vec2(1);

	stack.push_vec2(a.normalize());

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t vec2_negate(lua_State* L)
{
	LuaStack stack(L);

	Vec2& a = stack.get_vec2(1);

	stack.push_vec2(a.negate());

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t vec2_get_distance_to(lua_State* L)
{
	LuaStack stack(L);
	
	Vec2& a = stack.get_vec2(1);
	Vec2& b = stack.get_vec2(2);

	stack.push_float(a.get_distance_to(b));

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t vec2_get_angle_between(lua_State* L)
{
	LuaStack stack(L);
	
	Vec2& a = stack.get_vec2(1);
	Vec2& b = stack.get_vec2(2);

	stack.push_float(a.get_angle_between(b));

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t vec2_zero(lua_State* L)
{
	LuaStack stack(L);

	Vec2& a = stack.get_vec2(1);

	a.zero();

	return 0;
}

//-----------------------------------------------------------------------------
void load_vec2(LuaEnvironment& env)
{
	env.load_module_function("Vec2", "new",					vec2);
	env.load_module_function("Vec2", "val",					vec2_values);
	env.load_module_function("Vec2", "add",					vec2_add);
	env.load_module_function("Vec2", "sub",					vec2_subtract);
	env.load_module_function("Vec2", "mul",					vec2_multiply);
	env.load_module_function("Vec2", "div",					vec2_divide);
	env.load_module_function("Vec2", "dot",					vec2_dot);
	env.load_module_function("Vec2", "equals",				vec2_equals);
	env.load_module_function("Vec2", "lower",				vec2_lower);
	env.load_module_function("Vec2", "greater",				vec2_greater);
	env.load_module_function("Vec2", "length",				vec2_length);
	env.load_module_function("Vec2", "squared_length",		vec2_squared_length);
	env.load_module_function("Vec2", "set_length",			vec2_set_length);
	env.load_module_function("Vec2", "normalize",			vec2_normalize);
	env.load_module_function("Vec2", "negate",				vec2_negate);
	env.load_module_function("Vec2", "get_distance_to",		vec2_get_distance_to);
	env.load_module_function("Vec2", "get_angle_between",	vec2_get_angle_between);
	env.load_module_function("Vec2", "zero",				vec2_zero);
}

} // namespace crown