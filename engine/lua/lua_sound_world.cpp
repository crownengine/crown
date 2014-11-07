/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "lua_stack.h"
#include "lua_environment.h"
#include "sound_world.h"

namespace crown
{

static int sound_world_stop_all(lua_State* L)
{
	LuaStack stack(L);
	stack.get_sound_world(1)->stop_all();
	return 0;
}

static int sound_world_pause_all(lua_State* L)
{
	LuaStack stack(L);
	stack.get_sound_world(1)->pause_all();
	return 0;
}

static int sound_world_resume_all(lua_State* L)
{
	LuaStack stack(L);
	stack.get_sound_world(1)->resume_all();
	return 0;
}

static int sound_world_is_playing(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_sound_world(1)->is_playing(stack.get_sound_instance_id(2)));
	return 1;
}

static int sound_world_tostring(lua_State* L)
{
	LuaStack stack(L);
	SoundWorld* sw = stack.get_sound_world(1);
	stack.push_fstring("SoundWorld (%p)", sw);
	return 1;
}

void load_sound_world(LuaEnvironment& env)
{
	env.load_module_function("SoundWorld", "stop_all",   sound_world_stop_all);
	env.load_module_function("SoundWorld", "pause_all",  sound_world_pause_all);
	env.load_module_function("SoundWorld", "resume_all", sound_world_resume_all);
	env.load_module_function("SoundWorld", "is_playing", sound_world_is_playing);
	env.load_module_function("SoundWorld", "__index",    "SoundWorld");
	env.load_module_function("SoundWorld", "__tostring", sound_world_tostring);
}

} // namespace crown
