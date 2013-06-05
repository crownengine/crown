#include "LuaStack.h"
#include "Device.h"
#include "LuaEnvironment.h"
#include "Keyboard.h"

namespace crown
{

extern "C"
{

int32_t keyboard_modifier_pressed(lua_State* L)
{
	LuaStack stack(L);

	int32_t modifier = stack.get_int(1);

	stack.push_bool(device()->keyboard()->modifier_pressed((ModifierKey) modifier));

	return 1;
}

int32_t keyboard_key_pressed(lua_State* L)
{
	LuaStack stack(L);

	int32_t key = stack.get_int(1);

	stack.push_bool(device()->keyboard()->key_pressed((KeyCode) key));

	return 1;
}

int32_t keyboard_key_released(lua_State* L)
{
	LuaStack stack(L);

	int32_t key = stack.get_int(1);

	stack.push_bool(device()->keyboard()->key_released((KeyCode) key));

	return 1;
}

} // extern "C"

void load_keyboard(LuaEnvironment& env)
{
	env.load_module_function("Keyboard", "modifier_pressed",	keyboard_modifier_pressed);
	env.load_module_function("Keyboard", "key_pressed",			keyboard_key_pressed);
	env.load_module_function("Keyboard", "key_released",		keyboard_key_released);
}

} // namespace crown
