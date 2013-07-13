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

#include "LuaStack.h"
#include "Device.h"
#include "LuaEnvironment.h"
#include "Keyboard.h"

namespace crown
{

//-----------------------------------------------------------------------------
CE_EXPORT int32_t keyboard_modifier_pressed(lua_State* L)
{
	LuaStack stack(L);

	int32_t modifier = stack.get_int(1);

	stack.push_bool(device()->keyboard()->modifier_pressed((ModifierKey) modifier));

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t keyboard_key_pressed(lua_State* L)
{
	LuaStack stack(L);

	int32_t key = stack.get_int(1);

	stack.push_bool(device()->keyboard()->key_pressed((KeyCode) key));

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t keyboard_key_released(lua_State* L)
{
	LuaStack stack(L);

	int32_t key = stack.get_int(1);

	stack.push_bool(device()->keyboard()->key_released((KeyCode) key));

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t keyboard_any_pressed(lua_State* L)
{
	LuaStack stack(L);

	stack.push_bool(device()->keyboard()->any_pressed());

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t keyboard_any_released(lua_State* L)
{
	LuaStack stack(L);

	stack.push_bool(device()->keyboard()->any_released());

	return 1;
}

//-----------------------------------------------------------------------------
void load_keyboard(LuaEnvironment& env)
{
	env.load_module_function("Keyboard", "modifier_pressed",	keyboard_modifier_pressed);
	env.load_module_function("Keyboard", "key_pressed",			keyboard_key_pressed);
	env.load_module_function("Keyboard", "key_released",		keyboard_key_released);
	env.load_module_function("Keyboard", "any_pressed",			keyboard_any_pressed);
	env.load_module_function("Keyboard", "any_released",		keyboard_any_released);
}

} // namespace crown
