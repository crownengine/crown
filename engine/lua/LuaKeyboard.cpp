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
CE_EXPORT int keyboard_modifier_pressed(lua_State* L)
{
	LuaStack stack(L);

	int32_t modifier = stack.get_int(1);

	stack.push_bool(device()->keyboard()->modifier_pressed((ModifierButton::Enum) modifier));

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int keyboard_button_pressed(lua_State* L)
{
	LuaStack stack(L);

	int32_t button = stack.get_int(1);

	stack.push_bool(device()->keyboard()->button_pressed((KeyboardButton::Enum) button));

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int keyboard_button_released(lua_State* L)
{
	LuaStack stack(L);

	int32_t button = stack.get_int(1);

	stack.push_bool(device()->keyboard()->button_released((KeyboardButton::Enum) button));

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int keyboard_any_pressed(lua_State* L)
{
	LuaStack stack(L);

	stack.push_bool(device()->keyboard()->any_pressed());

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int keyboard_any_released(lua_State* L)
{
	LuaStack stack(L);

	stack.push_bool(device()->keyboard()->any_released());

	return 1;
}

//-----------------------------------------------------------------------------
void load_keyboard(LuaEnvironment& env)
{
	env.load_module_function("Keyboard", "modifier_pressed",	keyboard_modifier_pressed);
	env.load_module_function("Keyboard", "button_pressed",		keyboard_button_pressed);
	env.load_module_function("Keyboard", "button_released",		keyboard_button_released);
	env.load_module_function("Keyboard", "any_pressed",			keyboard_any_pressed);
	env.load_module_function("Keyboard", "any_released",		keyboard_any_released);

	env.load_module_enum("Keyboard", "TAB",			KeyboardButton::TAB);
	env.load_module_enum("Keyboard", "ENTER",		KeyboardButton::ENTER);
	env.load_module_enum("Keyboard", "ESCAPE",		KeyboardButton::ESCAPE);
	env.load_module_enum("Keyboard", "SPACE",		KeyboardButton::SPACE);
	env.load_module_enum("Keyboard", "BACKSPACE",	KeyboardButton::BACKSPACE);

	/* KeyPad */
	env.load_module_enum("Keyboard", "KP_0",		KeyboardButton::KP_0);
	env.load_module_enum("Keyboard", "KP_1",		KeyboardButton::KP_1);
	env.load_module_enum("Keyboard", "KP_2",		KeyboardButton::KP_2);
	env.load_module_enum("Keyboard", "KP_3",		KeyboardButton::KP_3);
	env.load_module_enum("Keyboard", "KP_4",		KeyboardButton::KP_4);
	env.load_module_enum("Keyboard", "KP_5",		KeyboardButton::KP_5);
	env.load_module_enum("Keyboard", "KP_6",		KeyboardButton::KP_6);
	env.load_module_enum("Keyboard", "KP_7",		KeyboardButton::KP_7);
	env.load_module_enum("Keyboard", "KP_8",		KeyboardButton::KP_8);
	env.load_module_enum("Keyboard", "KP_9",		KeyboardButton::KP_9);

	/* Function keys */
	env.load_module_enum("Keyboard", "F1",			KeyboardButton::F1);
	env.load_module_enum("Keyboard", "F2",			KeyboardButton::F2);
	env.load_module_enum("Keyboard", "F3",			KeyboardButton::F3);
	env.load_module_enum("Keyboard", "F4",			KeyboardButton::F4);
	env.load_module_enum("Keyboard", "F5",			KeyboardButton::F5);
	env.load_module_enum("Keyboard", "F6",			KeyboardButton::F6);
	env.load_module_enum("Keyboard", "F7",			KeyboardButton::F7);
	env.load_module_enum("Keyboard", "F8",			KeyboardButton::F8);
	env.load_module_enum("Keyboard", "F9",			KeyboardButton::F9);
	env.load_module_enum("Keyboard", "F10",			KeyboardButton::F10);
	env.load_module_enum("Keyboard", "F11",			KeyboardButton::F11);
	env.load_module_enum("Keyboard", "F12",			KeyboardButton::F12);

	/* Other keys */
	env.load_module_enum("Keyboard", "HOME",		KeyboardButton::HOME);
	env.load_module_enum("Keyboard", "LEFT",		KeyboardButton::LEFT);
	env.load_module_enum("Keyboard", "UP",			KeyboardButton::UP);
	env.load_module_enum("Keyboard", "RIGHT",		KeyboardButton::RIGHT);
	env.load_module_enum("Keyboard", "DOWN",		KeyboardButton::DOWN);
	env.load_module_enum("Keyboard", "PAGE_UP",		KeyboardButton::PAGE_UP);
	env.load_module_enum("Keyboard", "PAGE_DOWN",	KeyboardButton::PAGE_DOWN);

	/* Modifier keys */
	env.load_module_enum("Keyboard", "LCONTROL",	KeyboardButton::LCONTROL);
	env.load_module_enum("Keyboard", "RCONTROL",	KeyboardButton::RCONTROL);
	env.load_module_enum("Keyboard", "LSHIFT",		KeyboardButton::LSHIFT);
	env.load_module_enum("Keyboard", "RSHIFT",		KeyboardButton::RSHIFT);
	env.load_module_enum("Keyboard", "CAPS_LOCK",	KeyboardButton::CAPS_LOCK);
	env.load_module_enum("Keyboard", "LALT",		KeyboardButton::LALT);
	env.load_module_enum("Keyboard", "RALT",		KeyboardButton::RALT);
	env.load_module_enum("Keyboard", "LSUPER",		KeyboardButton::LSUPER);
	env.load_module_enum("Keyboard", "RSUPER",		KeyboardButton::RSUPER);

	/* [0x30, 0x39] reserved for ASCII digits */
	env.load_module_enum("Keyboard", "NUM_0",		KeyboardButton::NUM_0);
	env.load_module_enum("Keyboard", "NUM_1",		KeyboardButton::NUM_1);
	env.load_module_enum("Keyboard", "NUM_2",		KeyboardButton::NUM_2);
	env.load_module_enum("Keyboard", "NUM_3",		KeyboardButton::NUM_3);
	env.load_module_enum("Keyboard", "NUM_4",		KeyboardButton::NUM_4);
	env.load_module_enum("Keyboard", "NUM_5",		KeyboardButton::NUM_5);
	env.load_module_enum("Keyboard", "NUM_6",		KeyboardButton::NUM_6);
	env.load_module_enum("Keyboard", "NUM_7",		KeyboardButton::NUM_7);
	env.load_module_enum("Keyboard", "NUM_8",		KeyboardButton::NUM_8);
	env.load_module_enum("Keyboard", "NUM_9",		KeyboardButton::NUM_9);

	/* [0x41, 0x5A] reserved for ASCII alph,abet */
	env.load_module_enum("Keyboard", "A",			KeyboardButton::A);
	env.load_module_enum("Keyboard", "B",			KeyboardButton::B);
	env.load_module_enum("Keyboard", "C",			KeyboardButton::C);
	env.load_module_enum("Keyboard", "D",			KeyboardButton::D);
	env.load_module_enum("Keyboard", "E",			KeyboardButton::E);
	env.load_module_enum("Keyboard", "F",			KeyboardButton::F);
	env.load_module_enum("Keyboard", "G",			KeyboardButton::G);
	env.load_module_enum("Keyboard", "H",			KeyboardButton::H);
	env.load_module_enum("Keyboard", "I",			KeyboardButton::I);
	env.load_module_enum("Keyboard", "J",			KeyboardButton::J);
	env.load_module_enum("Keyboard", "K",			KeyboardButton::K);
	env.load_module_enum("Keyboard", "L",			KeyboardButton::L);
	env.load_module_enum("Keyboard", "M",			KeyboardButton::M);
	env.load_module_enum("Keyboard", "N",			KeyboardButton::N);
	env.load_module_enum("Keyboard", "O",			KeyboardButton::O);
	env.load_module_enum("Keyboard", "P",			KeyboardButton::P);
	env.load_module_enum("Keyboard", "Q",			KeyboardButton::Q);
	env.load_module_enum("Keyboard", "R",			KeyboardButton::R);
	env.load_module_enum("Keyboard", "S",			KeyboardButton::S);
	env.load_module_enum("Keyboard", "T",			KeyboardButton::T);
	env.load_module_enum("Keyboard", "U",			KeyboardButton::U);
	env.load_module_enum("Keyboard", "V",			KeyboardButton::V);
	env.load_module_enum("Keyboard", "W",			KeyboardButton::W);
	env.load_module_enum("Keyboard", "X",			KeyboardButton::X);
	env.load_module_enum("Keyboard", "Y",			KeyboardButton::Y);
	env.load_module_enum("Keyboard", "Z",			KeyboardButton::Z);

	/* [0x61, 0x7A] reserved for ASCII alphabet */
	env.load_module_enum("Keyboard", "a",			KeyboardButton::a);
	env.load_module_enum("Keyboard", "b",			KeyboardButton::b);
	env.load_module_enum("Keyboard", "c",			KeyboardButton::c);
	env.load_module_enum("Keyboard", "d",			KeyboardButton::d);
	env.load_module_enum("Keyboard", "e",			KeyboardButton::e);
	env.load_module_enum("Keyboard", "f",			KeyboardButton::f);
	env.load_module_enum("Keyboard", "g",			KeyboardButton::g);
	env.load_module_enum("Keyboard", "h",			KeyboardButton::h);
	env.load_module_enum("Keyboard", "i",			KeyboardButton::i);
	env.load_module_enum("Keyboard", "j",			KeyboardButton::j);
	env.load_module_enum("Keyboard", "k",			KeyboardButton::k);
	env.load_module_enum("Keyboard", "l",			KeyboardButton::l);
	env.load_module_enum("Keyboard", "m",			KeyboardButton::m);
	env.load_module_enum("Keyboard", "n",			KeyboardButton::n);
	env.load_module_enum("Keyboard", "o",			KeyboardButton::o);
	env.load_module_enum("Keyboard", "p",			KeyboardButton::p);
	env.load_module_enum("Keyboard", "q",			KeyboardButton::q);
	env.load_module_enum("Keyboard", "r",			KeyboardButton::r);
	env.load_module_enum("Keyboard", "s",			KeyboardButton::s);
	env.load_module_enum("Keyboard", "t",			KeyboardButton::t);
	env.load_module_enum("Keyboard", "u",			KeyboardButton::u);
	env.load_module_enum("Keyboard", "v",			KeyboardButton::v);
	env.load_module_enum("Keyboard", "w",			KeyboardButton::w);
	env.load_module_enum("Keyboard", "x",			KeyboardButton::x);
	env.load_module_enum("Keyboard", "y",			KeyboardButton::y);
	env.load_module_enum("Keyboard", "z",			KeyboardButton::z);
}

} // namespace crown
