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
#include "Device.h"
#include "World.h"
#include "SoundWorld.h"

namespace crown
{

//-----------------------------------------------------------------------------
CE_EXPORT int sound_world_play_sound(lua_State* L)
{
	LuaStack stack(L);

	World* world = (World*) stack.get_lightdata(1);
	const char* name = stack.get_string(2);

	const bool loop = stack.num_args() > 2 ? stack.get_bool(3) : false;
	const Vec3& pos = stack.num_args() > 3 ? stack.get_vec3(4) : Vec3::ZERO;

	SoundInstanceId id = world->sound_world().play_sound(name, loop, pos);

	stack.push_int32(id.value);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int sound_world_pause_sound(lua_State* L)
{
	LuaStack stack(L);

	World* world = (World*) stack.get_lightdata(1);

	SoundInstanceId id = {0};
	id.value = stack.get_int(2);

	world->sound_world().pause_sound(id);

	return 0;
}

// //-----------------------------------------------------------------------------
// CE_EXPORT int sound_world_link_sound(lua_State* L)
// {

// }

// //-----------------------------------------------------------------------------
// CE_EXPORT int sound_world_set_listener(lua_State* L)
// {

// }

//-----------------------------------------------------------------------------
CE_EXPORT int sound_world_set_sound_position(lua_State* /*L*/)
{
	// LuaStack stack
	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int sound_world_set_sound_range(lua_State* L)
{
	LuaStack stack(L);

	World* world = (World*) stack.get_lightdata(1);

	SoundInstanceId id = {0};
	id.value = stack.get_int(2);
	float range = stack.get_float(3);

	world->sound_world().set_sound_range(id, range);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int sound_world_set_sound_volume(lua_State* L)
{
	LuaStack stack(L);

	World* world = (World*) stack.get_lightdata(1);

	SoundInstanceId id = {0};
	id.value = stack.get_int(2);
	float vol = stack.get_float(2);

	world->sound_world().set_sound_volume(id, vol);

	return 0;
}

//-----------------------------------------------------------------------------
void load_sound(LuaEnvironment& env)
{
	env.load_module_function("Sound", "play_sound",	sound_world_play_sound);
	env.load_module_function("Sound", "pause_sound", sound_world_pause_sound);
	env.load_module_function("Sound", "set_sound_range", sound_world_set_sound_range);
	env.load_module_function("Sound", "set_sound_volume", sound_world_set_sound_volume);
}

} // namespace crown