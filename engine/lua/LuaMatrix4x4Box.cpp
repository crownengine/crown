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
CE_EXPORT int matrix4x4box(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& m = stack.push_matrix4x4box();

	stack.get_global_metatable("Matrix4x4_i_mt");
	stack.set_metatable();

	Matrix4x4 tm = stack.get_matrix4x4(1);

	m = tm;
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int matrix4x4box_store(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& m = stack.get_matrix4x4box(1);
	Matrix4x4 tm = stack.get_matrix4x4(2);

	m = tm;
	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int matrix4x4box_unbox(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& m = stack.get_matrix4x4box(1);

	stack.push_matrix4x4(m);
	return 1;
}

//-----------------------------------------------------------------------------
void load_matrix4x4box(LuaEnvironment& env)
{
	env.load_module_function("Matrix4x4Box", "new", matrix4x4box);
	env.load_module_function("Matrix4x4Box", "store", matrix4x4box_store);
	env.load_module_function("Matrix4x4Box", "unbox", matrix4x4box_unbox);
}

} // namespace crown