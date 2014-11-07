/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "int_setting.h"
#include "lua_stack.h"
#include "lua_environment.h"

namespace crown
{

static int int_setting_query_value(lua_State* L)
{
	LuaStack stack(L);

	const char* setting_name = stack.get_string(1);

	IntSetting* setting = IntSetting::find_setting(setting_name);

	if (setting != NULL)
	{
		stack.push_int32(setting->value());
	}
	else
	{
		stack.push_int32(0);
	}

	return 1;
}

static int int_setting_query_synopsis(lua_State* L)
{
	LuaStack stack(L);

	const char* setting_name = stack.get_string(1);

	IntSetting* setting = IntSetting::find_setting(setting_name);

	if (setting != NULL)
	{
		stack.push_string(setting->synopsis());
	}
	else
	{
		stack.push_string("");
	}

	return 1;
}

static int int_setting_query_min(lua_State* L)
{
	LuaStack stack(L);

	const char* setting_name = stack.get_string(1);

	IntSetting* setting = IntSetting::find_setting(setting_name);

	if (setting != NULL)
	{
		stack.push_int32(setting->min());
	}
	else
	{
		stack.push_int32(0);
	}

	return 1;
}

static int int_setting_query_max(lua_State* L)
{
	LuaStack stack(L);

	const char* setting_name = stack.get_string(1);

	IntSetting* setting = IntSetting::find_setting(setting_name);

	if (setting != NULL)
	{
		stack.push_int32(setting->max());
	}
	else
	{
		stack.push_int32(0);
	}

	return 1;
}

static int int_setting_update(lua_State* L)
{
	LuaStack stack(L);

	const char* setting_name = stack.get_string(1);
	const int32_t setting_value = stack.get_int(2);

	IntSetting* setting = IntSetting::find_setting(setting_name);

	if (setting != NULL)
	{
		(*setting) = setting_value;
	}

	return 0;
}

void load_int_setting(LuaEnvironment& env)
{
	env.load_module_function("IntSetting", "value",    int_setting_query_value);
	env.load_module_function("IntSetting", "synopsis", int_setting_query_synopsis);
	env.load_module_function("IntSetting", "min",      int_setting_query_min);
	env.load_module_function("IntSetting", "max",      int_setting_query_max);
	env.load_module_function("IntSetting", "update",   int_setting_update);
}

} // namespace crown
