#include "LuaStack.h"
#include "Device.h"
#include "LuaEnvironment.h"
#include "Mouse.h"
#include "Log.h"

namespace crown
{

extern "C"
{

extern Vec2* next_vec2();

//-----------------------------------------------------------------------------
int32_t mouse_button_pressed(lua_State* L)
{
	LuaStack stack(L);

	int32_t button = stack.get_int(1);

	stack.push_bool(device()->mouse()->button_pressed((MouseButton) button));

	return 1;
}

//-----------------------------------------------------------------------------
int32_t mouse_button_released(lua_State* L)
{
	LuaStack stack(L);

	int32_t button = stack.get_int(1);

	stack.push_bool(device()->mouse()->button_released((MouseButton) button));

	return 1;
}

//-----------------------------------------------------------------------------
int32_t mouse_cursor_xy(lua_State* L)
{
}

//-----------------------------------------------------------------------------
int32_t mouse_set_cursor_xy(lua_State* L)
{
}

//-----------------------------------------------------------------------------
int32_t mouse_cursor_relative_xy(lua_State* L)
{
	LuaStack stack(L);

	Vec2* xy = next_vec2();
	*xy = device()->mouse()->cursor_relative_xy();

	stack.push_lightudata(xy);

	return 1;
}

//-----------------------------------------------------------------------------
int32_t mouse_set_cursor_relative_xy(lua_State* L)
{
	LuaStack stack(L);

	Vec2* xy = (Vec2*) stack.get_lightudata(1);

	device()->mouse()->set_cursor_relative_xy(*xy);

	return 0;
}

} // extern "C"

//-----------------------------------------------------------------------------
void load_mouse(LuaEnvironment& env)
{
	env.load_module_function("Mouse", "button_pressed",			mouse_button_pressed);
	env.load_module_function("Mouse", "button_released",		mouse_button_released);
	env.load_module_function("Mouse", "cursor_relative_xy",		mouse_cursor_relative_xy);
	env.load_module_function("Mouse", "set_cursor_relative_xy",	mouse_set_cursor_relative_xy);
}

} // namespace crown
