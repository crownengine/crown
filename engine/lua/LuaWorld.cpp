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
#include "Gui.h"
#include "TempAllocator.h"
#include "Array.h"

namespace crown
{

//-----------------------------------------------------------------------------
static int world_spawn_unit(lua_State* L)
{
	LuaStack stack(L);

	World* world = stack.get_world(1);
	const char* name = stack.get_string(2);

	const Vector3& pos = stack.num_args() > 2 ? stack.get_vector3(3) : vector3::ZERO;
	const Quaternion& rot = stack.num_args() > 3 ? stack.get_quaternion(4) : quaternion::IDENTITY;

	UnitId unit = world->spawn_unit(name, pos, rot);

	stack.push_unit(world->get_unit(unit));
	return 1;
}

//-----------------------------------------------------------------------------
static int world_destroy_unit(lua_State* L)
{
	LuaStack stack(L);

	World* world = stack.get_world(1);
	Unit* unit = stack.get_unit(2);

	world->destroy_unit(unit->id());
	return 0;
}

//-----------------------------------------------------------------------------
static int world_num_units(lua_State* L)
{
	LuaStack stack(L);

	World* world = stack.get_world(1);

	stack.push_uint32(world->num_units());
	return 1;
}

//-----------------------------------------------------------------------------
static int world_units(lua_State* L)
{
	LuaStack stack(L);

	World* world = stack.get_world(1);

	TempAllocator1024 alloc;
	Array<UnitId> all_units(alloc);

	world->units(all_units);

	stack.push_table();
	for (uint32_t i = 0; i < array::size(all_units); i++)
	{
		stack.push_key_begin((int32_t) i + 1);
		stack.push_unit(world->get_unit(all_units[i]));
		stack.push_key_end();
	}

	return 1;
}

//-----------------------------------------------------------------------------
static int world_play_sound(lua_State* L)
{
	LuaStack stack(L);

	World* world = stack.get_world(1);
	const char* name = stack.get_string(2);

	const bool loop = stack.num_args() > 2 ? stack.get_bool(3) : false;
	const float volume = stack.num_args() > 3 ? stack.get_float(4) : 1.0f;
	const Vector3& pos = stack.num_args() > 4 ? stack.get_vector3(5) : vector3::ZERO;
	const float range = stack.num_args() > 5 ? stack.get_float(6) : 1000.0f;

	SoundInstanceId id = world->play_sound(name, loop, volume, pos, range);

	stack.push_sound_instance_id(id);
	return 1;
}

//-----------------------------------------------------------------------------
static int world_stop_sound(lua_State* L)
{
	LuaStack stack(L);

	World* world = stack.get_world(1);
	const SoundInstanceId id = stack.get_sound_instance_id(2);

	world->stop_sound(id);
	return 0;
}

//-----------------------------------------------------------------------------
static int world_link_sound(lua_State* L)
{
	LuaStack stack(L);

	World* world = stack.get_world(1);
	const SoundInstanceId id = stack.get_sound_instance_id(2);
	Unit* unit = stack.get_unit(3);
	const int32_t node = stack.get_int(4);

	world->link_sound(id, unit, node);
	return 0;
}

//-----------------------------------------------------------------------------
static int world_set_listener_pose(lua_State* L)
{
	LuaStack stack(L);

	World* world = stack.get_world(1);

	world->set_listener_pose(stack.get_matrix4x4(2));
	return 0;
}

//-----------------------------------------------------------------------------
static int world_set_sound_position(lua_State* L)
{
	LuaStack stack(L);

	World* world = stack.get_world(1);
	const SoundInstanceId id = stack.get_sound_instance_id(2);
	const Vector3& pos = stack.get_vector3(3);

	world->set_sound_position(id, pos);
	return 0;
}

//-----------------------------------------------------------------------------
static int world_set_sound_range(lua_State* L)
{
	LuaStack stack(L);

	World* world = stack.get_world(1);
	const SoundInstanceId id = stack.get_sound_instance_id(2);
	float range = stack.get_float(3);

	world->set_sound_range(id, range);
	return 0;
}

//-----------------------------------------------------------------------------
static int world_set_sound_volume(lua_State* L)
{
	LuaStack stack(L);

	World* world = stack.get_world(1);
	const SoundInstanceId id = stack.get_sound_instance_id(2);
	float vol = stack.get_float(3);

	world->set_sound_volume(id, vol);
	return 0;
}

//-----------------------------------------------------------------------------
static int world_create_window_gui(lua_State* L)
{
	LuaStack stack(L);

	World* world = stack.get_world(1);
	GuiId id = world->create_window_gui(stack.get_int(2), stack.get_int(3));

	stack.push_gui(world->get_gui(id));
	return 1;
}

//-----------------------------------------------------------------------------
static int world_destroy_gui(lua_State* L)
{
	LuaStack stack(L);

	World* world = stack.get_world(1);
	Gui* gui = stack.get_gui(2);

	world->destroy_gui(gui->id());

	return 0;
}

//-----------------------------------------------------------------------------
static int world_physics_world(lua_State* L)
{
	LuaStack stack(L);

	World* world = stack.get_world(1);

	stack.push_physics_world(world->physics_world());
	return 1;
}

//-----------------------------------------------------------------------------
static int world_sound_world(lua_State* L)
{
	LuaStack stack(L);

	World* world = stack.get_world(1);

	stack.push_sound_world(world->sound_world());
	return 1;
}

//-----------------------------------------------------------------------------
static int world_create_debug_line(lua_State* L)
{
	LuaStack stack(L);

	World* world = stack.get_world(1);
	const bool depth_test = stack.get_bool(2);

	stack.push_debug_line(world->create_debug_line(depth_test));
	return 1;
}

//-----------------------------------------------------------------------------
static int world_destroy_debug_line(lua_State* L)
{
	LuaStack stack(L);

	World* world = stack.get_world(1);
	DebugLine* line = stack.get_debug_line(2);

	world->destroy_debug_line(line);
	return 0;
}

//-----------------------------------------------------------------------------
static int world_load_level(lua_State* L)
{
	LuaStack stack(L);

	World* world = stack.get_world(1);
	world->load_level(stack.get_string(2));
	return 0;
}

//-----------------------------------------------------------------------------
static int world_tostring(lua_State* L)
{
	LuaStack stack(L);
	World* w = stack.get_world(1);
	stack.push_fstring("World (%p)", w);
	return 1;
}

//-----------------------------------------------------------------------------
void load_world(LuaEnvironment& env)
{
	env.load_module_function("World", "spawn_unit",			world_spawn_unit);
	env.load_module_function("World", "destroy_unit",       world_destroy_unit);
	env.load_module_function("World", "num_units",          world_num_units);
	env.load_module_function("World", "units",              world_units);

	env.load_module_function("World", "play_sound",			world_play_sound);
	env.load_module_function("World", "stop_sound", 		world_stop_sound);
	env.load_module_function("World", "link_sound",			world_link_sound);
	env.load_module_function("World", "set_listener_pose", 	world_set_listener_pose);
	env.load_module_function("World", "set_sound_position", world_set_sound_position);
	env.load_module_function("World", "set_sound_range", 	world_set_sound_range);
	env.load_module_function("World", "set_sound_volume", 	world_set_sound_volume);

	env.load_module_function("World", "create_window_gui",	world_create_window_gui);
	env.load_module_function("World", "destroy_gui",		world_destroy_gui);

	env.load_module_function("World", "physics_world",		world_physics_world);
	env.load_module_function("World", "sound_world",        world_sound_world);

	env.load_module_function("World", "create_debug_line",  world_create_debug_line);
	env.load_module_function("World", "destroy_debug_line", world_destroy_debug_line);

	env.load_module_function("World", "load_level",			world_load_level);

	env.load_module_function("World", "__index",			"World");
	env.load_module_function("World", "__tostring",			world_tostring);
}

} // namespace crown
