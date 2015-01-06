/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "resource_package.h"
#include "lua_stack.h"
#include "lua_environment.h"

namespace crown
{

static int resource_package_load(lua_State* L)
{
	LuaStack stack(L);
	stack.get_resource_package(1)->load();
	return 0;
}

static int resource_package_unload(lua_State* L)
{
	LuaStack stack(L);
	stack.get_resource_package(1)->unload();
	return 0;
}

static int resource_package_flush(lua_State* L)
{
	LuaStack stack(L);
	stack.get_resource_package(1)->flush();
	return 0;
}

static int resource_package_has_loaded(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_resource_package(1)->has_loaded());
	return 1;
}

static int resource_package_tostring(lua_State* L)
{
	LuaStack stack(L);
	ResourcePackage* package = stack.get_resource_package(1);
	stack.push_fstring("ResourcePackage (%p)", package);
	return 1;
}

void load_resource_package(LuaEnvironment& env)
{
	env.load_module_function("ResourcePackage", "load",       resource_package_load);
	env.load_module_function("ResourcePackage", "unload",     resource_package_unload);
	env.load_module_function("ResourcePackage", "flush",      resource_package_flush);
	env.load_module_function("ResourcePackage", "has_loaded", resource_package_has_loaded);
	env.load_module_function("ResourcePackage", "__index",    "ResourcePackage");
	env.load_module_function("ResourcePackage", "__tostring", resource_package_tostring);
}

} // namespace crown
