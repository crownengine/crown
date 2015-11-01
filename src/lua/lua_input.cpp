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

static int keyboard_name(lua_State* L)
{
	LuaStack stack(L);
	stack.push_string(device()->input_manager()->keyboard()->name());
	return 1;
}

static int keyboard_num_buttons(lua_State* L)
{
	LuaStack stack(L);
	stack.push_uint32(device()->input_manager()->keyboard()->num_buttons());
	return 1;
}

static int keyboard_num_axes(lua_State* L)
{
	LuaStack stack(L);
	stack.push_uint32(device()->input_manager()->keyboard()->num_axes());
	return 1;
}

static int keyboard_pressed(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(device()->input_manager()->keyboard()->pressed((KeyboardButton::Enum) stack.get_int(1)));
	return 1;
}

static int keyboard_released(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(device()->input_manager()->keyboard()->released((KeyboardButton::Enum) stack.get_int(1)));
	return 1;
}

static int keyboard_any_pressed(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(device()->input_manager()->keyboard()->any_pressed());
	return 1;
}

static int keyboard_any_released(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(device()->input_manager()->keyboard()->any_released());
	return 1;
}

static int mouse_name(lua_State* L)
{
	LuaStack stack(L);
	stack.push_string(device()->input_manager()->mouse()->name());
	return 1;
}

static int mouse_num_buttons(lua_State* L)
{
	LuaStack stack(L);
	stack.push_uint32(device()->input_manager()->mouse()->num_buttons());
	return 1;
}

static int mouse_num_axes(lua_State* L)
{
	LuaStack stack(L);
	stack.push_uint32(device()->input_manager()->mouse()->num_axes());
	return 1;
}

static int mouse_pressed(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(device()->input_manager()->mouse()->pressed((MouseButton::Enum) stack.get_int(1)));
	return 1;
}

static int mouse_released(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(device()->input_manager()->mouse()->released((MouseButton::Enum) stack.get_int(1)));
	return 1;
}

static int mouse_any_pressed(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(device()->input_manager()->mouse()->any_pressed());
	return 1;
}

static int mouse_any_released(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(device()->input_manager()->mouse()->any_released());
	return 1;
}

static int mouse_axis(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(device()->input_manager()->mouse()->axis(stack.get_int(1)));
	return 1;
}

static int touch_name(lua_State* L)
{
	LuaStack stack(L);
	stack.push_string(device()->input_manager()->touch()->name());
	return 1;
}

static int touch_num_buttons(lua_State* L)
{
	LuaStack stack(L);
	stack.push_uint32(device()->input_manager()->touch()->num_buttons());
	return 1;
}

static int touch_num_axes(lua_State* L)
{
	LuaStack stack(L);
	stack.push_uint32(device()->input_manager()->touch()->num_axes());
	return 1;
}

static int touch_pressed(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(device()->input_manager()->touch()->pressed(stack.get_int(1)));
	return 1;
}

static int touch_released(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(device()->input_manager()->touch()->released(stack.get_int(1)));
	return 1;
}

static int touch_any_pressed(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(device()->input_manager()->touch()->any_pressed());
	return 1;
}

static int touch_any_released(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(device()->input_manager()->touch()->any_released());
	return 1;
}

static int touch_axis(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(device()->input_manager()->touch()->axis(stack.get_int(1)));
	return 1;
}

void load_input(LuaEnvironment& env)
{
	env.load_module_function("Keyboard", "name",         keyboard_name);
	env.load_module_function("Keyboard", "num_buttons",  keyboard_num_buttons);
	env.load_module_function("Keyboard", "num_axes",     keyboard_num_axes);
	env.load_module_function("Keyboard", "pressed",      keyboard_pressed);
	env.load_module_function("Keyboard", "released",     keyboard_released);
	env.load_module_function("Keyboard", "any_pressed",  keyboard_any_pressed);
	env.load_module_function("Keyboard", "any_released", keyboard_any_released);

	env.load_module_enum("Keyboard", "TAB",       KeyboardButton::TAB);
	env.load_module_enum("Keyboard", "ENTER",     KeyboardButton::ENTER);
	env.load_module_enum("Keyboard", "ESCAPE",    KeyboardButton::ESCAPE);
	env.load_module_enum("Keyboard", "SPACE",     KeyboardButton::SPACE);
	env.load_module_enum("Keyboard", "BACKSPACE", KeyboardButton::BACKSPACE);
	env.load_module_enum("Keyboard", "KP_0",      KeyboardButton::KP_0);
	env.load_module_enum("Keyboard", "KP_1",      KeyboardButton::KP_1);
	env.load_module_enum("Keyboard", "KP_2",      KeyboardButton::KP_2);
	env.load_module_enum("Keyboard", "KP_3",      KeyboardButton::KP_3);
	env.load_module_enum("Keyboard", "KP_4",      KeyboardButton::KP_4);
	env.load_module_enum("Keyboard", "KP_5",      KeyboardButton::KP_5);
	env.load_module_enum("Keyboard", "KP_6",      KeyboardButton::KP_6);
	env.load_module_enum("Keyboard", "KP_7",      KeyboardButton::KP_7);
	env.load_module_enum("Keyboard", "KP_8",      KeyboardButton::KP_8);
	env.load_module_enum("Keyboard", "KP_9",      KeyboardButton::KP_9);
	env.load_module_enum("Keyboard", "F1",        KeyboardButton::F1);
	env.load_module_enum("Keyboard", "F2",        KeyboardButton::F2);
	env.load_module_enum("Keyboard", "F3",        KeyboardButton::F3);
	env.load_module_enum("Keyboard", "F4",        KeyboardButton::F4);
	env.load_module_enum("Keyboard", "F5",        KeyboardButton::F5);
	env.load_module_enum("Keyboard", "F6",        KeyboardButton::F6);
	env.load_module_enum("Keyboard", "F7",        KeyboardButton::F7);
	env.load_module_enum("Keyboard", "F8",        KeyboardButton::F8);
	env.load_module_enum("Keyboard", "F9",        KeyboardButton::F9);
	env.load_module_enum("Keyboard", "F10",       KeyboardButton::F10);
	env.load_module_enum("Keyboard", "F11",       KeyboardButton::F11);
	env.load_module_enum("Keyboard", "F12",       KeyboardButton::F12);
	env.load_module_enum("Keyboard", "HOME",      KeyboardButton::HOME);
	env.load_module_enum("Keyboard", "LEFT",      KeyboardButton::LEFT);
	env.load_module_enum("Keyboard", "UP",        KeyboardButton::UP);
	env.load_module_enum("Keyboard", "RIGHT",     KeyboardButton::RIGHT);
	env.load_module_enum("Keyboard", "DOWN",      KeyboardButton::DOWN);
	env.load_module_enum("Keyboard", "PAGE_UP",   KeyboardButton::PAGE_UP);
	env.load_module_enum("Keyboard", "PAGE_DOWN", KeyboardButton::PAGE_DOWN);
	env.load_module_enum("Keyboard", "LCONTROL",  KeyboardButton::LCONTROL);
	env.load_module_enum("Keyboard", "RCONTROL",  KeyboardButton::RCONTROL);
	env.load_module_enum("Keyboard", "LSHIFT",    KeyboardButton::LSHIFT);
	env.load_module_enum("Keyboard", "RSHIFT",    KeyboardButton::RSHIFT);
	env.load_module_enum("Keyboard", "CAPS_LOCK", KeyboardButton::CAPS_LOCK);
	env.load_module_enum("Keyboard", "LALT",      KeyboardButton::LALT);
	env.load_module_enum("Keyboard", "RALT",      KeyboardButton::RALT);
	env.load_module_enum("Keyboard", "LSUPER",    KeyboardButton::LSUPER);
	env.load_module_enum("Keyboard", "RSUPER",    KeyboardButton::RSUPER);
	env.load_module_enum("Keyboard", "NUM_0",     KeyboardButton::NUM_0);
	env.load_module_enum("Keyboard", "NUM_1",     KeyboardButton::NUM_1);
	env.load_module_enum("Keyboard", "NUM_2",     KeyboardButton::NUM_2);
	env.load_module_enum("Keyboard", "NUM_3",     KeyboardButton::NUM_3);
	env.load_module_enum("Keyboard", "NUM_4",     KeyboardButton::NUM_4);
	env.load_module_enum("Keyboard", "NUM_5",     KeyboardButton::NUM_5);
	env.load_module_enum("Keyboard", "NUM_6",     KeyboardButton::NUM_6);
	env.load_module_enum("Keyboard", "NUM_7",     KeyboardButton::NUM_7);
	env.load_module_enum("Keyboard", "NUM_8",     KeyboardButton::NUM_8);
	env.load_module_enum("Keyboard", "NUM_9",     KeyboardButton::NUM_9);
	env.load_module_enum("Keyboard", "A",         KeyboardButton::A);
	env.load_module_enum("Keyboard", "B",         KeyboardButton::B);
	env.load_module_enum("Keyboard", "C",         KeyboardButton::C);
	env.load_module_enum("Keyboard", "D",         KeyboardButton::D);
	env.load_module_enum("Keyboard", "E",         KeyboardButton::E);
	env.load_module_enum("Keyboard", "F",         KeyboardButton::F);
	env.load_module_enum("Keyboard", "G",         KeyboardButton::G);
	env.load_module_enum("Keyboard", "H",         KeyboardButton::H);
	env.load_module_enum("Keyboard", "I",         KeyboardButton::I);
	env.load_module_enum("Keyboard", "J",         KeyboardButton::J);
	env.load_module_enum("Keyboard", "K",         KeyboardButton::K);
	env.load_module_enum("Keyboard", "L",         KeyboardButton::L);
	env.load_module_enum("Keyboard", "M",         KeyboardButton::M);
	env.load_module_enum("Keyboard", "N",         KeyboardButton::N);
	env.load_module_enum("Keyboard", "O",         KeyboardButton::O);
	env.load_module_enum("Keyboard", "P",         KeyboardButton::P);
	env.load_module_enum("Keyboard", "Q",         KeyboardButton::Q);
	env.load_module_enum("Keyboard", "R",         KeyboardButton::R);
	env.load_module_enum("Keyboard", "S",         KeyboardButton::S);
	env.load_module_enum("Keyboard", "T",         KeyboardButton::T);
	env.load_module_enum("Keyboard", "U",         KeyboardButton::U);
	env.load_module_enum("Keyboard", "V",         KeyboardButton::V);
	env.load_module_enum("Keyboard", "W",         KeyboardButton::W);
	env.load_module_enum("Keyboard", "X",         KeyboardButton::X);
	env.load_module_enum("Keyboard", "Y",         KeyboardButton::Y);
	env.load_module_enum("Keyboard", "Z",         KeyboardButton::Z);

	env.load_module_function("Mouse", "name",         mouse_name);
	env.load_module_function("Mouse", "num_buttons",  mouse_num_buttons);
	env.load_module_function("Mouse", "num_axes",     mouse_num_axes);
	env.load_module_function("Mouse", "pressed",      mouse_pressed);
	env.load_module_function("Mouse", "released",     mouse_released);
	env.load_module_function("Mouse", "any_pressed",  mouse_any_pressed);
	env.load_module_function("Mouse", "any_released", mouse_any_released);
	env.load_module_function("Mouse", "axis",         mouse_axis);

	env.load_module_enum("Mouse", "LEFT",   MouseButton::LEFT);
	env.load_module_enum("Mouse", "MIDDLE", MouseButton::MIDDLE);
	env.load_module_enum("Mouse", "RIGHT",  MouseButton::RIGHT);

	env.load_module_function("Touch", "name",         touch_name);
	env.load_module_function("Touch", "num_buttons",  touch_num_buttons);
	env.load_module_function("Touch", "num_axes",     touch_num_axes);
	env.load_module_function("Touch", "pressed",      touch_pressed);
	env.load_module_function("Touch", "released",     touch_released);
	env.load_module_function("Touch", "any_pressed",  touch_any_pressed);
	env.load_module_function("Touch", "any_released", touch_any_released);
	env.load_module_function("Touch", "axis",         touch_axis);
}

} // namespace crown
