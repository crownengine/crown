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

#include "LuaStack.h"
#include "LuaEnvironment.h"
#include "Raycast.h"

namespace crown
{

//-----------------------------------------------------------------------------
CE_EXPORT int raycast_cast(lua_State* L)
{
	LuaStack stack(L);

	Raycast* raycast = stack.get_raycast(1);
	Vector3 from = stack.get_vector3(2);
	Vector3 dir = stack.get_vector3(3);
	float length = stack.get_float(4);
	
	raycast->cast(from, dir, length);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int raycast_sync_cast(lua_State* L)
{
	LuaStack stack(L);

	Raycast* raycast = stack.get_raycast(1);
	Vector3 from = stack.get_vector3(2);
	Vector3 dir = stack.get_vector3(3);
	float length = stack.get_float(4);
	
	Actor* actor = raycast->sync_cast(from, dir, length);

	if (actor) stack.push_actor(actor);
	else stack.push_nil();

	return 1;
}

//-----------------------------------------------------------------------------
void load_raycast(LuaEnvironment& env)
{
	env.load_module_function("Raycast", "cast",	raycast_cast);
	env.load_module_function("Raycast", "sync_cast", raycast_sync_cast);
}

} // namespace crown