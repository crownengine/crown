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

#include "lua_stack.h"
#include "lua_environment.h"
#include "material.h"

namespace crown
{

//-----------------------------------------------------------------------------
static int material_set_float(lua_State* L)
{
	LuaStack stack(L);
	stack.get_material(1)->set_float(stack.get_string(2), stack.get_float(3));
	return 0;
}

//-----------------------------------------------------------------------------
static int material_set_vector2(lua_State* L)
{
	LuaStack stack(L);
	stack.get_material(1)->set_vector2(stack.get_string(2), stack.get_vector2(3));
	return 0;
}

//-----------------------------------------------------------------------------
static int material_set_vector3(lua_State* L)
{
	LuaStack stack(L);
	stack.get_material(1)->set_vector3(stack.get_string(2), stack.get_vector3(3));
	return 0;
}

//-----------------------------------------------------------------------------
void load_material(LuaEnvironment& env)
{
	env.load_module_function("Material", "set_float",   material_set_float);
	env.load_module_function("Material", "set_vector2", material_set_vector2);
	env.load_module_function("Material", "set_vector3", material_set_vector3);
}

} // namespace crown
