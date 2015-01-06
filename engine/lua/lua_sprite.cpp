/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "sprite.h"
#include "quaternion.h"
#include "lua_stack.h"
#include "lua_environment.h"

namespace crown
{

static int sprite_local_position(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_sprite(1)->local_position());
	return 1;
}

static int sprite_local_rotation(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(stack.get_sprite(1)->local_rotation());
	return 1;
}

static int sprite_local_pose(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(stack.get_sprite(1)->local_pose());
	return 1;
}

static int sprite_set_local_position(lua_State* L)
{
	LuaStack stack(L);
	stack.get_sprite(1)->set_local_position(stack.get_unit(2), stack.get_vector3(3));
	return 0;
}

static int sprite_set_local_rotation(lua_State* L)
{
	LuaStack stack(L);
	stack.get_sprite(1)->set_local_rotation(stack.get_unit(2), stack.get_quaternion(3));
	return 0;
}

static int sprite_set_local_pose(lua_State* L)
{
	LuaStack stack(L);
	stack.get_sprite(1)->set_local_pose(stack.get_unit(2), stack.get_matrix4x4(3));
	return 0;
}

static int sprite_set_frame(lua_State* L)
{
	LuaStack stack(L);
	stack.get_sprite(1)->set_frame(stack.get_int(2));
	return 0;
}

static int sprite_set_depth(lua_State* L)
{
	LuaStack stack(L);
	stack.get_sprite(1)->set_depth(stack.get_int(2));
	return 0;
}

void load_sprite(LuaEnvironment& env)
{
	env.load_module_function("Sprite", "local_position",     sprite_local_position);
	env.load_module_function("Sprite", "local_rotation",     sprite_local_rotation);
	env.load_module_function("Sprite", "local_pose",         sprite_local_pose);
	env.load_module_function("Sprite", "set_local_position", sprite_set_local_position);
	env.load_module_function("Sprite", "set_local_rotation", sprite_set_local_rotation);
	env.load_module_function("Sprite", "set_local_pose",     sprite_set_local_pose);
	env.load_module_function("Sprite", "set_frame",          sprite_set_frame);
	env.load_module_function("Sprite", "set_depth",          sprite_set_depth);
}

} // namespace crown
