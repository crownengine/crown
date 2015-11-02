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

static int input_device_num_buttons(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_uint32(id.num_buttons());
	return 1;
}

static int input_device_num_axes(lua_State* L, InputDevice& id)
{
	LuaStack stack(L);
	stack.push_uint32(id.num_axes());
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
KEYBOARD(num_buttons)
KEYBOARD(num_axes)
KEYBOARD(pressed)
KEYBOARD(released)
KEYBOARD(any_pressed)
KEYBOARD(any_released)
// KEYBOARD(axis) // Keyboard has no axis

MOUSE(name)
MOUSE(num_buttons)
MOUSE(num_axes)
MOUSE(pressed)
MOUSE(released)
MOUSE(any_pressed)
MOUSE(any_released)
MOUSE(axis)

TOUCH(name)
TOUCH(num_buttons)
TOUCH(num_axes)
TOUCH(pressed)
TOUCH(released)
TOUCH(any_pressed)
TOUCH(any_released)
TOUCH(axis)

JOYPAD(0, name)
JOYPAD(0, num_buttons)
JOYPAD(0, num_axes)
JOYPAD(0, pressed)
JOYPAD(0, released)
JOYPAD(0, any_pressed)
JOYPAD(0, any_released)
JOYPAD(0, axis)

JOYPAD(1, name)
JOYPAD(1, num_buttons)
JOYPAD(1, num_axes)
JOYPAD(1, pressed)
JOYPAD(1, released)
JOYPAD(1, any_pressed)
JOYPAD(1, any_released)
JOYPAD(1, axis)

JOYPAD(2, name)
JOYPAD(2, num_buttons)
JOYPAD(2, num_axes)
JOYPAD(2, pressed)
JOYPAD(2, released)
JOYPAD(2, any_pressed)
JOYPAD(2, any_released)
JOYPAD(2, axis)

JOYPAD(3, name)
JOYPAD(3, num_buttons)
JOYPAD(3, num_axes)
JOYPAD(3, pressed)
JOYPAD(3, released)
JOYPAD(3, any_pressed)
JOYPAD(3, any_released)
JOYPAD(3, axis)

void load_input(LuaEnvironment& env)
{
	env.load_module_function("Keyboard", "name",         KEYBOARD_FN(name));
	env.load_module_function("Keyboard", "num_buttons",  KEYBOARD_FN(num_buttons));
	env.load_module_function("Keyboard", "num_axes",     KEYBOARD_FN(num_axes));
	env.load_module_function("Keyboard", "pressed",      KEYBOARD_FN(pressed));
	env.load_module_function("Keyboard", "released",     KEYBOARD_FN(released));
	env.load_module_function("Keyboard", "any_pressed",  KEYBOARD_FN(any_pressed));
	env.load_module_function("Keyboard", "any_released", KEYBOARD_FN(any_released));

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

	env.load_module_function("Mouse", "name",         MOUSE_FN(name));
	env.load_module_function("Mouse", "num_buttons",  MOUSE_FN(num_buttons));
	env.load_module_function("Mouse", "num_axes",     MOUSE_FN(num_axes));
	env.load_module_function("Mouse", "pressed",      MOUSE_FN(pressed));
	env.load_module_function("Mouse", "released",     MOUSE_FN(released));
	env.load_module_function("Mouse", "any_pressed",  MOUSE_FN(any_pressed));
	env.load_module_function("Mouse", "any_released", MOUSE_FN(any_released));
	env.load_module_function("Mouse", "axis",         MOUSE_FN(axis));

	env.load_module_enum("Mouse", "LEFT",   MouseButton::LEFT);
	env.load_module_enum("Mouse", "MIDDLE", MouseButton::MIDDLE);
	env.load_module_enum("Mouse", "RIGHT",  MouseButton::RIGHT);

	env.load_module_function("Touch", "name",         TOUCH_FN(name));
	env.load_module_function("Touch", "num_buttons",  TOUCH_FN(num_buttons));
	env.load_module_function("Touch", "num_axes",     TOUCH_FN(num_axes));
	env.load_module_function("Touch", "pressed",      TOUCH_FN(pressed));
	env.load_module_function("Touch", "released",     TOUCH_FN(released));
	env.load_module_function("Touch", "any_pressed",  TOUCH_FN(any_pressed));
	env.load_module_function("Touch", "any_released", TOUCH_FN(any_released));
	env.load_module_function("Touch", "axis",         TOUCH_FN(axis));

	env.load_module_function("Pad1", "name",         JOYPAD_FN(0, name));
	env.load_module_function("Pad1", "num_buttons",  JOYPAD_FN(0, num_buttons));
	env.load_module_function("Pad1", "num_axes",     JOYPAD_FN(0, num_axes));
	env.load_module_function("Pad1", "pressed",      JOYPAD_FN(0, pressed));
	env.load_module_function("Pad1", "released",     JOYPAD_FN(0, released));
	env.load_module_function("Pad1", "any_pressed",  JOYPAD_FN(0, any_pressed));
	env.load_module_function("Pad1", "any_released", JOYPAD_FN(0, any_released));
	env.load_module_function("Pad1", "axis",         JOYPAD_FN(0, axis));

	env.load_module_function("Pad2", "name",         JOYPAD_FN(1, name));
	env.load_module_function("Pad2", "num_buttons",  JOYPAD_FN(1, num_buttons));
	env.load_module_function("Pad2", "num_axes",     JOYPAD_FN(1, num_axes));
	env.load_module_function("Pad2", "pressed",      JOYPAD_FN(1, pressed));
	env.load_module_function("Pad2", "released",     JOYPAD_FN(1, released));
	env.load_module_function("Pad2", "any_pressed",  JOYPAD_FN(1, any_pressed));
	env.load_module_function("Pad2", "any_released", JOYPAD_FN(1, any_released));
	env.load_module_function("Pad2", "axis",         JOYPAD_FN(1, axis));

	env.load_module_function("Pad3", "name",         JOYPAD_FN(2, name));
	env.load_module_function("Pad3", "num_buttons",  JOYPAD_FN(2, num_buttons));
	env.load_module_function("Pad3", "num_axes",     JOYPAD_FN(2, num_axes));
	env.load_module_function("Pad3", "pressed",      JOYPAD_FN(2, pressed));
	env.load_module_function("Pad3", "released",     JOYPAD_FN(2, released));
	env.load_module_function("Pad3", "any_pressed",  JOYPAD_FN(2, any_pressed));
	env.load_module_function("Pad3", "any_released", JOYPAD_FN(2, any_released));
	env.load_module_function("Pad3", "axis",         JOYPAD_FN(2, axis));

	env.load_module_function("Pad4", "name",         JOYPAD_FN(3, name));
	env.load_module_function("Pad4", "num_buttons",  JOYPAD_FN(3, num_buttons));
	env.load_module_function("Pad4", "num_axes",     JOYPAD_FN(3, num_axes));
	env.load_module_function("Pad4", "pressed",      JOYPAD_FN(3, pressed));
	env.load_module_function("Pad4", "released",     JOYPAD_FN(3, released));
	env.load_module_function("Pad4", "any_pressed",  JOYPAD_FN(3, any_pressed));
	env.load_module_function("Pad4", "any_released", JOYPAD_FN(3, any_released));
	env.load_module_function("Pad4", "axis",         JOYPAD_FN(3, axis));

	env.load_module_enum("JoypadButton", "UP",             JoypadButton::UP);
	env.load_module_enum("JoypadButton", "DOWN",           JoypadButton::DOWN);
	env.load_module_enum("JoypadButton", "LEFT",           JoypadButton::LEFT);
	env.load_module_enum("JoypadButton", "RIGHT",          JoypadButton::RIGHT);
	env.load_module_enum("JoypadButton", "START",          JoypadButton::START);
	env.load_module_enum("JoypadButton", "BACK",           JoypadButton::BACK);
	env.load_module_enum("JoypadButton", "LEFT_THUMB",     JoypadButton::LEFT_THUMB);
	env.load_module_enum("JoypadButton", "RIGHT_THUMB",    JoypadButton::RIGHT_THUMB);
	env.load_module_enum("JoypadButton", "LEFT_SHOULDER",  JoypadButton::LEFT_SHOULDER);
	env.load_module_enum("JoypadButton", "RIGHT_SHOULDER", JoypadButton::RIGHT_SHOULDER);
	env.load_module_enum("JoypadButton", "A",              JoypadButton::A);
	env.load_module_enum("JoypadButton", "B",              JoypadButton::B);
	env.load_module_enum("JoypadButton", "X",              JoypadButton::X);
	env.load_module_enum("JoypadButton", "Y",              JoypadButton::Y);

	env.load_module_enum("JoypadAxis", "LEFT",  JoypadAxis::LEFT);
	env.load_module_enum("JoypadAxis", "RIGHT", JoypadAxis::RIGHT);
}

} // namespace crown
