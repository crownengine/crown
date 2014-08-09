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

#include "string_setting.h"
#include "lua_stack.h"
#include "lua_environment.h"

namespace crown
{

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
void load_string_setting(LuaEnvironment& env)
{
	env.load_module_function("StringSetting", "value",		string_setting_value);
	env.load_module_function("StringSetting", "synopsis",	string_setting_synopsis);
	env.load_module_function("StringSetting", "update",		string_setting_update);
}

} // namespace crown
