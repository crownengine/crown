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

#include "Quaternion.h"
#include "Vector3.h"
#include "LuaStack.h"
#include "LuaEnvironment.h"

namespace crown
{

//-----------------------------------------------------------------------------
CE_EXPORT int quaternionbox(lua_State* L)
{
	LuaStack stack(L);

	Quaternion& q = stack.push_quaternionbox();

	stack.get_global_metatable("Quaternion_i_mt");
	stack.set_metatable();

	if (stack.num_args() == 3)
	{
		const Vector3& v = stack.get_vector3(1);
		q.x = v.x;
		q.y = v.y;
		q.z = v.z;
		q.w = stack.get_float(2);
	}
	else if (stack.num_args() == 5)
	{
		q.x = stack.get_float(1);
		q.y = stack.get_float(2);
		q.z = stack.get_float(3);
		q.w = stack.get_float(4);
	}
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int quaternionbox_store(lua_State* L)
{
	LuaStack stack(L);

	Quaternion& q = stack.get_quaternionbox(1);
	
	if (stack.num_args() == 3)
	{
		const Vector3& v = stack.get_vector3(1);
		q.x = v.x;
		q.y = v.y;
		q.z = v.z;
		q.w = stack.get_float(2);
	}
	else if (stack.num_args() == 5)
	{
		q.x = stack.get_float(1);
		q.y = stack.get_float(2);
		q.z = stack.get_float(3);
		q.w = stack.get_float(4);
	}
	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int quaternionbox_unbox(lua_State* L)
{
	LuaStack stack(L);

	Quaternion& q = stack.get_quaternionbox(1);

	stack.push_quaternion(q);
	return 1;
}

//-----------------------------------------------------------------------------
void load_quaternionbox(LuaEnvironment& env)
{
	env.load_module_function("QuaternionBox", "new", quaternionbox);
	env.load_module_function("QuaternionBox", "store", quaternionbox_store);
	env.load_module_function("QuaternionBox", "unbox", quaternionbox_unbox);
}

} // namespace crown