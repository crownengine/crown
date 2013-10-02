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
#include "World.h"
#include "Device.h"

namespace crown
{

//-----------------------------------------------------------------------------
CE_EXPORT int world_spawn_unit(lua_State* L)
{
	LuaStack stack(L);

	World* world = (World*) stack.get_lightdata(1);
	const char* name = stack.get_string(2);

	const Vec3& pos = stack.num_args() > 2 ? stack.get_vec3(3) : Vec3::ZERO;
	const Quat& rot = stack.num_args() > 3 ? stack.get_quat(4) : Quat::IDENTITY;

	world->spawn_unit(name, pos, rot);

	return 0;
}

//-----------------------------------------------------------------------------
void load_world(LuaEnvironment& env)
{
	env.load_module_function("World", "spawn_unit",			world_spawn_unit);
}

} // namespace crown