/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "controller.h"
#include "vector3.h"
#include "lua_stack.h"
#include "lua_environment.h"

namespace crown
{

static int controller_move(lua_State* L)
{
	LuaStack stack(L);
	stack.get_controller(1)->move(stack.get_vector3(2));
	return 0;
}

static int controller_position(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_controller(1)->position());
	return 1;
}

static int controller_collides_up(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_controller(1)->collides_up());
	return 1;
}

static int controller_collides_down(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_controller(1)->collides_down());
	return 1;
}

static int controller_collides_sides(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_controller(1)->collides_sides());
	return 1;
}

void load_controller(LuaEnvironment& env)
{
	env.load_module_function("Controller", "move",           controller_move);
	env.load_module_function("Controller", "position",       controller_position);
	env.load_module_function("Controller", "collides_up",    controller_collides_up);
	env.load_module_function("Controller", "collides_down",  controller_collides_down);
	env.load_module_function("Controller", "collides_sides", controller_collides_sides);
}

} // namespace crown
