/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "lua_stack.h"
#include "lua_environment.h"
#include "unit.h"

namespace crown
{

static int unit_local_position(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_unit(1)->local_position());
	return 1;
}

static int unit_local_rotation(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(stack.get_unit(1)->local_rotation());
	return 1;
}

static int unit_local_scale(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_unit(1)->local_scale());
	return 1;
}

static int unit_local_pose(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(stack.get_unit(1)->local_pose());
	return 1;
}

static int unit_world_position(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_unit(1)->world_position());
	return 1;
}

static int unit_world_rotation(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(stack.get_unit(1)->world_rotation());
	return 1;
}

static int unit_world_pose(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(stack.get_unit(1)->world_pose());
	return 1;
}

static int unit_set_local_position(lua_State* L)
{
	LuaStack stack(L);
	stack.get_unit(1)->set_local_position(stack.get_vector3(2));
	return 0;
}

static int unit_set_local_rotation(lua_State* L)
{
	LuaStack stack(L);
	stack.get_unit(1)->set_local_rotation(stack.get_quaternion(2));
	return 0;
}

static int unit_set_local_scale(lua_State* L)
{
	LuaStack stack(L);
	stack.get_unit(1)->set_local_scale(stack.get_vector3(2));
	return 0;
}

static int unit_set_local_pose(lua_State* L)
{
	LuaStack stack(L);
	stack.get_unit(1)->set_local_pose(stack.get_matrix4x4(2));
	return 0;
}

static int unit_camera(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);

	if (stack.is_number(2))
	{
		stack.push_camera(unit->camera((uint32_t) stack.get_int(2)));
		return 1;
	}

	stack.push_camera(unit->camera(stack.get_string(2)));
	return 1;
}

static int unit_material(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);

	if (stack.is_number(2))
	{
		stack.push_material(unit->material((uint32_t) stack.get_int(2)));
		return 1;
	}

	stack.push_material(unit->material(stack.get_string(2)));
	return 1;
}

static int unit_sprite(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);

	if (stack.is_number(2))
	{
		stack.push_sprite(unit->sprite((uint32_t) stack.get_int(2)));
		return 1;
	}

	stack.push_sprite(unit->sprite(stack.get_string(2)));
	return 1;
}

static int unit_actor(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);

	if (stack.is_number(2))
	{
		stack.push_actor(unit->actor((uint32_t) stack.get_int(2)));
		return 1;
	}

	stack.push_actor(unit->actor(stack.get_string(2)));
	return 1;
}

static int unit_controller(lua_State* L)
{
	LuaStack stack(L);
	Controller* ctl = stack.get_unit(1)->controller();
	ctl ? stack.push_controller(ctl) : stack.push_nil();
	return 1;
}

static int unit_is_a(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_unit(1)->is_a(stack.get_resource_id(2)));
	return 1;
}

static int unit_play_sprite_animation(lua_State* L)
{
	LuaStack stack(L);
	stack.get_unit(1)->play_sprite_animation(stack.get_string(2), stack.get_bool(3));
	return 0;
}

static int unit_stop_sprite_animation(lua_State* L)
{
	LuaStack stack(L);
	stack.get_unit(1)->stop_sprite_animation();
	return 0;
}

void load_unit(LuaEnvironment& env)
{
	env.load_module_function("Unit", "local_position",        unit_local_position);
	env.load_module_function("Unit", "local_rotation",        unit_local_rotation);
	env.load_module_function("Unit", "local_scale",           unit_local_scale);
	env.load_module_function("Unit", "local_pose",            unit_local_pose);
	env.load_module_function("Unit", "world_position",        unit_world_position);
	env.load_module_function("Unit", "world_rotation",        unit_world_rotation);
	env.load_module_function("Unit", "world_pose",            unit_world_pose);
	env.load_module_function("Unit", "set_local_position",    unit_set_local_position);
	env.load_module_function("Unit", "set_local_rotation",    unit_set_local_rotation);
	env.load_module_function("Unit", "set_local_scale",       unit_set_local_scale);
	env.load_module_function("Unit", "set_local_pose",        unit_set_local_pose);
	env.load_module_function("Unit", "camera",                unit_camera);
	env.load_module_function("Unit", "material",              unit_material);
	env.load_module_function("Unit", "sprite",                unit_sprite);
	env.load_module_function("Unit", "actor",                 unit_actor);
	env.load_module_function("Unit", "controller",            unit_controller);
	env.load_module_function("Unit", "is_a",                  unit_is_a);
	env.load_module_function("Unit", "play_sprite_animation", unit_play_sprite_animation);
	env.load_module_function("Unit", "stop_sprite_animation", unit_stop_sprite_animation);
}

} // namespace crown
