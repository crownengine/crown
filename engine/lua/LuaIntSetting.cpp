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

#include "IntSetting.h"
#include "LuaStack.h"
#include "LuaEnvironment.h"

namespace crown
{

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
void load_int_setting(LuaEnvironment& env)
{
	env.load_module_function("IntSetting", "value",		int_setting_query_value);
	env.load_module_function("IntSetting", "synopsis",	int_setting_query_synopsis);
	env.load_module_function("IntSetting", "min",		int_setting_query_min);
	env.load_module_function("IntSetting", "max",		int_setting_query_max);
	env.load_module_function("IntSetting", "update",	int_setting_update);
}

} // namespace crown