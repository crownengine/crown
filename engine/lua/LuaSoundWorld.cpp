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
#include "SoundWorld.h"

namespace crown
{

//-----------------------------------------------------------------------------
CE_EXPORT int sound_world_stop_all(lua_State* L)
{
	LuaStack stack(L);

	SoundWorld* sw = stack.get_sound_world(1);
	sw->stop_all();
	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int sound_world_pause_all(lua_State* L)
{
	LuaStack stack(L);

	SoundWorld* sw = stack.get_sound_world(1);
	sw->pause_all();
	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int sound_world_resume_all(lua_State* L)
{
	LuaStack stack(L);

	SoundWorld* sw = stack.get_sound_world(1);
	sw->resume_all();
	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int sound_world_is_playing(lua_State* L)
{
	LuaStack stack(L);

	SoundWorld* sw = stack.get_sound_world(1);
	SoundInstanceId id = stack.get_sound_instance_id(2);
	stack.push_bool(sw->is_playing(id));
	return 1;
}

//-----------------------------------------------------------------------------
void load_sound_world(LuaEnvironment& env)
{
	env.load_module_function("SoundWorld", "stop_all",    sound_world_stop_all);
	env.load_module_function("SoundWorld", "pause_all",   sound_world_pause_all);
	env.load_module_function("SoundWorld", "resume_all",  sound_world_resume_all);
	env.load_module_function("SoundWorld", "is_playing",  sound_world_is_playing);
}

} // namespace crown
