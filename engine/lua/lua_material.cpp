/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "lua_stack.h"
#include "lua_environment.h"
#include "material.h"

namespace crown
{

static int material_set_float(lua_State* L)
{
	LuaStack stack(L);
	stack.get_material(1)->set_float(stack.get_string(2), stack.get_float(3));
	return 0;
}

static int material_set_vector2(lua_State* L)
{
	LuaStack stack(L);
	stack.get_material(1)->set_vector2(stack.get_string(2), stack.get_vector2(3));
	return 0;
}

static int material_set_vector3(lua_State* L)
{
	LuaStack stack(L);
	stack.get_material(1)->set_vector3(stack.get_string(2), stack.get_vector3(3));
	return 0;
}

void load_material(LuaEnvironment& env)
{
	env.load_module_function("Material", "set_float",   material_set_float);
	env.load_module_function("Material", "set_vector2", material_set_vector2);
	env.load_module_function("Material", "set_vector3", material_set_vector3);
}

} // namespace crown
