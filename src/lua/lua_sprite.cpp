/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "sprite.h"
#include "quaternion.h"
#include "lua_stack.h"
#include "lua_environment.h"

namespace crown
{

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
	env.load_module_function("Sprite", "set_frame",          sprite_set_frame);
	env.load_module_function("Sprite", "set_depth",          sprite_set_depth);
}

} // namespace crown
