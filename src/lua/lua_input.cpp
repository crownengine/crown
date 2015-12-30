/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "lua_stack.h"
#include "lua_environment.h"
#include "device.h"
#include "input_manager.h"
#include "input_device.h"

namespace crown
{

static int input_device_name(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_string(id.name());
	return 1;
}

static int input_device_connected(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_bool(id.connected());
	return 1;
}

static int input_device_num_buttons(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_int(id.num_buttons());
	return 1;
}

static int input_device_num_axes(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_int(id.num_axes());
	return 1;
}

static int input_device_pressed(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_bool(id.pressed(stack.get_int(1)));
	return 1;
}

static int input_device_released(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_bool(id.released(stack.get_int(1)));
	return 1;
}

static int input_device_any_pressed(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_bool(id.any_pressed());
	return 1;
}

static int input_device_any_released(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_bool(id.any_released());
	return 1;
}

static int input_device_axis(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_vector3(id.axis(stack.get_int(1)));
	return 1;
}

static int input_device_button_name(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_string(id.button_name(stack.get_int(1)));
	return 1;
}

static int input_device_axis_name(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_string(id.axis_name(stack.get_int(1)));
	return 1;
}

static int input_device_button_id(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_int(id.button_id(stack.get_string_id(1)));
	return 1;
}

static int input_device_axis_id(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_int(id.axis_id(stack.get_string_id(1)));
	return 1;
}

#define KEYBOARD_FN(name) keyboard_##name
#define MOUSE_FN(name) mouse_##name
#define TOUCH_FN(name) touch_##name
#define JOYPAD_FN(index, name) joypad_##name##index

#define KEYBOARD(name) static int KEYBOARD_FN(name)(lua_State* L)\
	{ return input_device_##name(L, *device()->input_manager()->keyboard()); }
#define MOUSE(name) static int MOUSE_FN(name)(lua_State* L)\
	{ return input_device_##name(L, *device()->input_manager()->mouse()); }
#define TOUCH(name) static int TOUCH_FN(name)(lua_State* L)\
	{ return input_device_##name(L, *device()->input_manager()->touch()); }
#define JOYPAD(index, name) static int JOYPAD_FN(index, name)(lua_State* L)\
	{ return input_device_##name(L, *device()->input_manager()->joypad(index)); }

KEYBOARD(name)
KEYBOARD(connected)
KEYBOARD(num_buttons)
KEYBOARD(num_axes)
KEYBOARD(pressed)
KEYBOARD(released)
KEYBOARD(any_pressed)
KEYBOARD(any_released)
// KEYBOARD(axis)
KEYBOARD(button_name)
// KEYBOARD(axis_name)
KEYBOARD(button_id)
// KEYBOARD(axis_id)

MOUSE(name)
MOUSE(connected)
MOUSE(num_buttons)
MOUSE(num_axes)
MOUSE(pressed)
MOUSE(released)
MOUSE(any_pressed)
MOUSE(any_released)
MOUSE(axis)
MOUSE(button_name)
MOUSE(axis_name)
MOUSE(button_id)
MOUSE(axis_id)

TOUCH(name)
TOUCH(connected)
TOUCH(num_buttons)
TOUCH(num_axes)
TOUCH(pressed)
TOUCH(released)
TOUCH(any_pressed)
TOUCH(any_released)
TOUCH(axis)
TOUCH(button_name)
TOUCH(axis_name)
TOUCH(button_id)
TOUCH(axis_id)

JOYPAD(0, name)
JOYPAD(0, connected)
JOYPAD(0, num_buttons)
JOYPAD(0, num_axes)
JOYPAD(0, pressed)
JOYPAD(0, released)
JOYPAD(0, any_pressed)
JOYPAD(0, any_released)
JOYPAD(0, axis)
JOYPAD(0, button_name)
JOYPAD(0, axis_name)
JOYPAD(0, button_id)
JOYPAD(0, axis_id)

JOYPAD(1, name)
JOYPAD(1, connected)
JOYPAD(1, num_buttons)
JOYPAD(1, num_axes)
JOYPAD(1, pressed)
JOYPAD(1, released)
JOYPAD(1, any_pressed)
JOYPAD(1, any_released)
JOYPAD(1, axis)
JOYPAD(1, button_name)
JOYPAD(1, axis_name)
JOYPAD(1, button_id)
JOYPAD(1, axis_id)

JOYPAD(2, name)
JOYPAD(2, connected)
JOYPAD(2, num_buttons)
JOYPAD(2, num_axes)
JOYPAD(2, pressed)
JOYPAD(2, released)
JOYPAD(2, any_pressed)
JOYPAD(2, any_released)
JOYPAD(2, axis)
JOYPAD(2, button_name)
JOYPAD(2, axis_name)
JOYPAD(2, button_id)
JOYPAD(2, axis_id)

JOYPAD(3, name)
JOYPAD(3, connected)
JOYPAD(3, num_buttons)
JOYPAD(3, num_axes)
JOYPAD(3, pressed)
JOYPAD(3, released)
JOYPAD(3, any_pressed)
JOYPAD(3, any_released)
JOYPAD(3, axis)
JOYPAD(3, button_name)
JOYPAD(3, axis_name)
JOYPAD(3, button_id)
JOYPAD(3, axis_id)

void load_input(LuaEnvironment& env)
{
	env.load_module_function("Keyboard", "name",         KEYBOARD_FN(name));
	env.load_module_function("Keyboard", "connected",    KEYBOARD_FN(connected));
	env.load_module_function("Keyboard", "num_buttons",  KEYBOARD_FN(num_buttons));
	env.load_module_function("Keyboard", "num_axes",     KEYBOARD_FN(num_axes));
	env.load_module_function("Keyboard", "pressed",      KEYBOARD_FN(pressed));
	env.load_module_function("Keyboard", "released",     KEYBOARD_FN(released));
	env.load_module_function("Keyboard", "any_pressed",  KEYBOARD_FN(any_pressed));
	env.load_module_function("Keyboard", "any_released", KEYBOARD_FN(any_released));
	env.load_module_function("Keyboard", "button_name",  KEYBOARD_FN(button_name));
	env.load_module_function("Keyboard", "button_id",    KEYBOARD_FN(button_id));

	env.load_module_function("Mouse", "name",         MOUSE_FN(name));
	env.load_module_function("Mouse", "connected",    MOUSE_FN(connected));
	env.load_module_function("Mouse", "num_buttons",  MOUSE_FN(num_buttons));
	env.load_module_function("Mouse", "num_axes",     MOUSE_FN(num_axes));
	env.load_module_function("Mouse", "pressed",      MOUSE_FN(pressed));
	env.load_module_function("Mouse", "released",     MOUSE_FN(released));
	env.load_module_function("Mouse", "any_pressed",  MOUSE_FN(any_pressed));
	env.load_module_function("Mouse", "any_released", MOUSE_FN(any_released));
	env.load_module_function("Mouse", "axis",         MOUSE_FN(axis));
	env.load_module_function("Mouse", "button_name",  MOUSE_FN(button_name));
	env.load_module_function("Mouse", "axis_name",    MOUSE_FN(axis_name));
	env.load_module_function("Mouse", "button_id",    MOUSE_FN(button_id));
	env.load_module_function("Mouse", "axis_id",      MOUSE_FN(axis_id));

	env.load_module_function("Touch", "name",         TOUCH_FN(name));
	env.load_module_function("Touch", "connected",    TOUCH_FN(connected));
	env.load_module_function("Touch", "num_buttons",  TOUCH_FN(num_buttons));
	env.load_module_function("Touch", "num_axes",     TOUCH_FN(num_axes));
	env.load_module_function("Touch", "pressed",      TOUCH_FN(pressed));
	env.load_module_function("Touch", "released",     TOUCH_FN(released));
	env.load_module_function("Touch", "any_pressed",  TOUCH_FN(any_pressed));
	env.load_module_function("Touch", "any_released", TOUCH_FN(any_released));
	env.load_module_function("Touch", "axis",         TOUCH_FN(axis));
	env.load_module_function("Touch", "button_name",  TOUCH_FN(button_name));
	env.load_module_function("Touch", "axis_name",    TOUCH_FN(axis_name));
	env.load_module_function("Touch", "button_id",    TOUCH_FN(button_id));
	env.load_module_function("Touch", "axis_id",      TOUCH_FN(axis_id));

	env.load_module_function("Pad1", "name",         JOYPAD_FN(0, name));
	env.load_module_function("Pad1", "connected",    JOYPAD_FN(0, connected));
	env.load_module_function("Pad1", "num_buttons",  JOYPAD_FN(0, num_buttons));
	env.load_module_function("Pad1", "num_axes",     JOYPAD_FN(0, num_axes));
	env.load_module_function("Pad1", "pressed",      JOYPAD_FN(0, pressed));
	env.load_module_function("Pad1", "released",     JOYPAD_FN(0, released));
	env.load_module_function("Pad1", "any_pressed",  JOYPAD_FN(0, any_pressed));
	env.load_module_function("Pad1", "any_released", JOYPAD_FN(0, any_released));
	env.load_module_function("Pad1", "axis",         JOYPAD_FN(0, axis));
	env.load_module_function("Pad1", "button_name",  JOYPAD_FN(0, button_name));
	env.load_module_function("Pad1", "axis_name",    JOYPAD_FN(0, axis_name));
	env.load_module_function("Pad1", "button_id",    JOYPAD_FN(0, button_id));
	env.load_module_function("Pad1", "axis_id",      JOYPAD_FN(0, axis_id));

	env.load_module_function("Pad2", "name",         JOYPAD_FN(1, name));
	env.load_module_function("Pad2", "connected",    JOYPAD_FN(1, connected));
	env.load_module_function("Pad2", "num_buttons",  JOYPAD_FN(1, num_buttons));
	env.load_module_function("Pad2", "num_axes",     JOYPAD_FN(1, num_axes));
	env.load_module_function("Pad2", "pressed",      JOYPAD_FN(1, pressed));
	env.load_module_function("Pad2", "released",     JOYPAD_FN(1, released));
	env.load_module_function("Pad2", "any_pressed",  JOYPAD_FN(1, any_pressed));
	env.load_module_function("Pad2", "any_released", JOYPAD_FN(1, any_released));
	env.load_module_function("Pad2", "axis",         JOYPAD_FN(1, axis));
	env.load_module_function("Pad2", "button_name",  JOYPAD_FN(1, button_name));
	env.load_module_function("Pad2", "axis_name",    JOYPAD_FN(1, axis_name));
	env.load_module_function("Pad2", "button_id",    JOYPAD_FN(1, button_id));
	env.load_module_function("Pad2", "axis_id",      JOYPAD_FN(1, axis_id));

	env.load_module_function("Pad3", "name",         JOYPAD_FN(2, name));
	env.load_module_function("Pad3", "connected",    JOYPAD_FN(2, connected));
	env.load_module_function("Pad3", "num_buttons",  JOYPAD_FN(2, num_buttons));
	env.load_module_function("Pad3", "num_axes",     JOYPAD_FN(2, num_axes));
	env.load_module_function("Pad3", "pressed",      JOYPAD_FN(2, pressed));
	env.load_module_function("Pad3", "released",     JOYPAD_FN(2, released));
	env.load_module_function("Pad3", "any_pressed",  JOYPAD_FN(2, any_pressed));
	env.load_module_function("Pad3", "any_released", JOYPAD_FN(2, any_released));
	env.load_module_function("Pad3", "axis",         JOYPAD_FN(2, axis));
	env.load_module_function("Pad3", "button_name",  JOYPAD_FN(2, button_name));
	env.load_module_function("Pad3", "axis_name",    JOYPAD_FN(2, axis_name));
	env.load_module_function("Pad3", "button_id",    JOYPAD_FN(2, button_id));
	env.load_module_function("Pad3", "axis_id",      JOYPAD_FN(2, axis_id));

	env.load_module_function("Pad4", "name",         JOYPAD_FN(3, name));
	env.load_module_function("Pad4", "connected",    JOYPAD_FN(3, connected));
	env.load_module_function("Pad4", "num_buttons",  JOYPAD_FN(3, num_buttons));
	env.load_module_function("Pad4", "num_axes",     JOYPAD_FN(3, num_axes));
	env.load_module_function("Pad4", "pressed",      JOYPAD_FN(3, pressed));
	env.load_module_function("Pad4", "released",     JOYPAD_FN(3, released));
	env.load_module_function("Pad4", "any_pressed",  JOYPAD_FN(3, any_pressed));
	env.load_module_function("Pad4", "any_released", JOYPAD_FN(3, any_released));
	env.load_module_function("Pad4", "axis",         JOYPAD_FN(3, axis));
	env.load_module_function("Pad4", "button_name",  JOYPAD_FN(3, button_name));
	env.load_module_function("Pad4", "axis_name",    JOYPAD_FN(3, axis_name));
	env.load_module_function("Pad4", "button_id",    JOYPAD_FN(3, button_id));
	env.load_module_function("Pad4", "axis_id",      JOYPAD_FN(3, axis_id));
}

} // namespace crown
