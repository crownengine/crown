/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "quaternion.h"
#include "lua_stack.h"
#include "lua_environment.h"

namespace crown
{

static int color4_new(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(Quaternion(stack.get_float(1),
							stack.get_float(2),
							stack.get_float(3),
							stack.get_float(4)));
	return 1;
}

static int color4_ctor(lua_State* L)
{
	LuaStack stack(L);
	stack.remove(1); // Remove table
	return color4_new(L);
}

void load_color4(LuaEnvironment& env)
{
	env.load_module_function("Color4", "new", color4_new);
	env.load_module_constructor("Color4",     color4_ctor);
}

} // namespace crown
