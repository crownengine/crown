/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "string_setting.h"
#include "lua_stack.h"
#include "lua_environment.h"

namespace crown
{

static int string_setting_value(lua_State* L)
{
	LuaStack stack(L);

	const char* setting_name = stack.get_string(1);

	StringSetting* setting = StringSetting::find_setting(setting_name);

	if (setting != NULL)
	{
		stack.push_string(setting->value());
	}
	else
	{
		stack.push_string("");
	}

	return 1;
}

static int string_setting_synopsis(lua_State* L)
{
	LuaStack stack(L);

	const char* setting_name = stack.get_string(1);

	StringSetting* setting = StringSetting::find_setting(setting_name);

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

static int string_setting_update(lua_State* L)
{
	LuaStack stack(L);

	const char* setting_name = stack.get_string(1);
	const char* setting_value = stack.get_string(2);

	StringSetting* setting = StringSetting::find_setting(setting_name);

	if (setting != NULL)
	{
		(*setting) = setting_value;
	}

	return 0;
}

void load_string_setting(LuaEnvironment& env)
{
	env.load_module_function("StringSetting", "value",    string_setting_value);
	env.load_module_function("StringSetting", "synopsis", string_setting_synopsis);
	env.load_module_function("StringSetting", "update",   string_setting_update);
}

} // namespace crown
