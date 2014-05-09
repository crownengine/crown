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
#include "LuaStack.h"
#include "LuaEnvironment.h"

namespace crown
{

//-----------------------------------------------------------------------------
static int color4_new(lua_State* L)
{
	LuaStack stack(L);

	const float r = stack.get_float(1);
	const float g = stack.get_float(2);
	const float b = stack.get_float(3);
	const float a = stack.get_float(4);

	stack.push_quaternion(Quaternion(r, g, b, a));
	return 1;
}

//-----------------------------------------------------------------------------
static int color4_ctor(lua_State* L)
{
	LuaStack stack(L);
	stack.remove(1); // Remove table
	return color4_new(L);
}

//-----------------------------------------------------------------------------
void load_color4(LuaEnvironment& env)
{
	env.load_module_function("Color4", "new", color4_new);
	env.load_module_constructor("Color4", color4_ctor);
}

} // namespace crown
