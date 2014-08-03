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

#include "matrix4x4.h"
#include "vector3.h"
#include "lua_stack.h"
#include "lua_environment.h"
#include "os.h"

namespace crown
{

//-----------------------------------------------------------------------------
static int matrix4x4box_new(lua_State* L)
{
	LuaStack stack(L);

	const Matrix4x4& tm = stack.get_matrix4x4(1);

	stack.push_matrix4x4box(tm);
	return 1;
}

//-----------------------------------------------------------------------------
static int matrix4x4box_ctor(lua_State* L)
{
	LuaStack stack(L);
	stack.remove(1); // Remove table
	return matrix4x4box_new(L);
}

//-----------------------------------------------------------------------------
static int matrix4x4box_store(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& m = stack.get_matrix4x4box(1);
	const Matrix4x4 tm = stack.get_matrix4x4(2);

	m = tm;
	return 0;
}

//-----------------------------------------------------------------------------
static int matrix4x4box_unbox(lua_State* L)
{
	LuaStack stack(L);

	Matrix4x4& m = stack.get_matrix4x4box(1);

	stack.push_matrix4x4(m);
	return 1;
}

//-----------------------------------------------------------------------------
static int matrix4x4box_tostring(lua_State* L)
{
	LuaStack stack(L);
	Matrix4x4& m = stack.get_matrix4x4box(1);
	stack.push_fstring("Matrix4x4Box (%p)", &m);
	return 1;
}

//-----------------------------------------------------------------------------
void load_matrix4x4box(LuaEnvironment& env)
{
	env.load_module_function("Matrix4x4Box", "new",			matrix4x4box_new);
	env.load_module_function("Matrix4x4Box", "store",		matrix4x4box_store);
	env.load_module_function("Matrix4x4Box", "unbox",		matrix4x4box_unbox);
	env.load_module_function("Matrix4x4Box", "__tostring",	matrix4x4box_tostring);

	env.load_module_constructor("Matrix4x4Box",				matrix4x4box_ctor);
}

} // namespace crown
