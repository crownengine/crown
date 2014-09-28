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

#include "lua_stack.h"
#include "lua_environment.h"
#include "input.h"
#include "keyboard.h"


namespace crown
{
using namespace input_globals;

//-----------------------------------------------------------------------------
static int keyboard_modifier_pressed(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(keyboard().modifier_pressed((ModifierButton::Enum) stack.get_int(1)));
	return 1;
}

//-----------------------------------------------------------------------------
static int keyboard_button_pressed(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(keyboard().button_pressed((KeyboardButton::Enum) stack.get_int(1)));
	return 1;
}

//-----------------------------------------------------------------------------
static int keyboard_button_released(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(keyboard().button_released((KeyboardButton::Enum) stack.get_int(1)));
	return 1;
}

//-----------------------------------------------------------------------------
static int keyboard_any_pressed(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(keyboard().any_pressed());
	return 1;
}

//-----------------------------------------------------------------------------
static int keyboard_any_released(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(keyboard().any_released());
	return 1;
}

//-----------------------------------------------------------------------------
void load_keyboard(LuaEnvironment& env)
{
	env.load_module_function("Keyboard", "modifier_pressed", keyboard_modifier_pressed);
	env.load_module_function("Keyboard", "button_pressed",   keyboard_button_pressed);
	env.load_module_function("Keyboard", "button_released",  keyboard_button_released);
	env.load_module_function("Keyboard", "any_pressed",      keyboard_any_pressed);
	env.load_module_function("Keyboard", "any_released",     keyboard_any_released);
	
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
}

} // namespace crown
