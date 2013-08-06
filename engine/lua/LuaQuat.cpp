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

#include "Quat.h"
#include "Vec3.h"
#include "LuaStack.h"
#include "LuaEnvironment.h"


namespace crown
{

//-----------------------------------------------------------------------------
CE_EXPORT int32_t quat(lua_State* L)
{
	LuaStack stack(L);

	Vec3& v = stack.get_vec3(1);
	float w = stack.get_float(2);

	stack.push_quat(Quat(v, w));

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t quat_negate(lua_State* L)
{
	LuaStack stack(L);

	Quat& q = stack.get_quat(1);

	q.negate();

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t quat_load_identity(lua_State* L)
{
	LuaStack stack(L);

	Quat& q = stack.get_quat(1);

	q.load_identity();

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t quat_length(lua_State* L)
{
	LuaStack stack(L);

	Quat& q = stack.get_quat(1);

	stack.push_float(q.length());

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t quat_conjugate(lua_State* L)
{
	LuaStack stack(L);

	Quat& q = stack.get_quat(1);

	stack.push_quat(q.get_conjugate());

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t quat_inverse(lua_State* L)
{
	LuaStack stack(L);

	Quat& q = stack.get_quat(1);

	stack.push_quat(q.get_inverse());

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t quat_cross(lua_State* L)
{
	LuaStack stack(L);

	Quat& q1 = stack.get_quat(1);
	Quat& q2 = stack.get_quat(2);

	stack.push_quat(q1 * q2);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t quat_multiply(lua_State* L)
{
	LuaStack stack(L);

	Quat& q = stack.get_quat(1);
	float k = stack.get_float(2);

	stack.push_quat(q * k);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t quat_power(lua_State* L)
{
	LuaStack stack(L);

	Quat& q = stack.get_quat(1);
	float k = stack.get_float(2);

	stack.push_quat(q.power(k));

	return 1;
}

//-----------------------------------------------------------------------------
void load_quat(LuaEnvironment& env)
{
	env.load_module_function("Quat", "new",				quat);
	env.load_module_function("Quat", "negate",			quat_negate);
	env.load_module_function("Quat", "load_identity",	quat_load_identity);
	env.load_module_function("Quat", "length",			quat_length);
	env.load_module_function("Quat", "conjugate",		quat_conjugate);
	env.load_module_function("Quat", "inverse",			quat_inverse);
	env.load_module_function("Quat", "cross",			quat_cross);
	env.load_module_function("Quat", "mul",				quat_multiply);
	env.load_module_function("Quat", "pow",				quat_power);
}

} //namespace crown