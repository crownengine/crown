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

	stack.push_bool(device()->keyboard()->modifier_pressed((ModifierKey) modifier));

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int keyboard_key_pressed(lua_State* L)
{
	LuaStack stack(L);

	int32_t key = stack.get_int(1);

	stack.push_bool(device()->keyboard()->key_pressed((KeyCode) key));

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int keyboard_key_released(lua_State* L)
{
	LuaStack stack(L);

	int32_t key = stack.get_int(1);

	stack.push_bool(device()->keyboard()->key_released((KeyCode) key));

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
	env.load_module_function("Keyboard", "key_pressed",			keyboard_key_pressed);
	env.load_module_function("Keyboard", "key_released",		keyboard_key_released);
	env.load_module_function("Keyboard", "any_pressed",			keyboard_any_pressed);
	env.load_module_function("Keyboard", "any_released",		keyboard_any_released);

	env.load_module_enum("Keyboard", "KC_NOKEY",		KC_NOKEY);

	env.load_module_enum("Keyboard", "KC_TAB",			KC_TAB);
	env.load_module_enum("Keyboard", "KC_ENTER",		KC_ENTER);
	env.load_module_enum("Keyboard", "KC_ESCAPE",		KC_ESCAPE);
	env.load_module_enum("Keyboard", "KC_SPACE",		KC_SPACE);
	env.load_module_enum("Keyboard", "KC_BACKSPACE",	KC_BACKSPACE);

	/* KeyPad */
	env.load_module_enum("Keyboard", "KC_KP_0",			KC_KP_0);
	env.load_module_enum("Keyboard", "KC_KP_1",			KC_KP_1);
	env.load_module_enum("Keyboard", "KC_KP_2",			KC_KP_2);
	env.load_module_enum("Keyboard", "KC_KP_3",			KC_KP_3);
	env.load_module_enum("Keyboard", "KC_KP_4",			KC_KP_4);
	env.load_module_enum("Keyboard", "KC_KP_5",			KC_KP_5);
	env.load_module_enum("Keyboard", "KC_KP_6",			KC_KP_6);
	env.load_module_enum("Keyboard", "KC_KP_7",			KC_KP_7);
	env.load_module_enum("Keyboard", "KC_KP_8",			KC_KP_8);
	env.load_module_enum("Keyboard", "KC_KP_9",			KC_KP_9);

	/* Function keys */
	env.load_module_enum("Keyboard", "KC_F1",			KC_F1);
	env.load_module_enum("Keyboard", "KC_F2",			KC_F2);
	env.load_module_enum("Keyboard", "KC_F3",			KC_F3);
	env.load_module_enum("Keyboard", "KC_F4",			KC_F4);
	env.load_module_enum("Keyboard", "KC_F5",			KC_F5);
	env.load_module_enum("Keyboard", "KC_F6",			KC_F6);
	env.load_module_enum("Keyboard", "KC_F7",			KC_F7);
	env.load_module_enum("Keyboard", "KC_F8",			KC_F8);
	env.load_module_enum("Keyboard", "KC_F9",			KC_F9);
	env.load_module_enum("Keyboard", "KC_F10",			KC_F10);
	env.load_module_enum("Keyboard", "KC_F11",			KC_F11);
	env.load_module_enum("Keyboard", "KC_F12",			KC_F12);

	/* Other keys */
	env.load_module_enum("Keyboard", "KC_HOME",			KC_HOME);
	env.load_module_enum("Keyboard", "KC_LEFT",			KC_LEFT);
	env.load_module_enum("Keyboard", "KC_UP",			KC_UP);
	env.load_module_enum("Keyboard", "KC_RIGHT",		KC_RIGHT);
	env.load_module_enum("Keyboard", "KC_DOWN",			KC_DOWN);
	env.load_module_enum("Keyboard", "KC_PAGE_UP",		KC_PAGE_UP);
	env.load_module_enum("Keyboard", "KC_PAGE_DOWN",	KC_PAGE_DOWN);

	/* Modifier keys */
	env.load_module_enum("Keyboard", "KC_LCONTROL",		KC_LCONTROL);
	env.load_module_enum("Keyboard", "KC_RCONTROL",		KC_RCONTROL);
	env.load_module_enum("Keyboard", "KC_LSHIFT",		KC_LSHIFT);
	env.load_module_enum("Keyboard", "KC_RSHIFT",		KC_RSHIFT);
	env.load_module_enum("Keyboard", "KC_CAPS_LOCK",	KC_CAPS_LOCK);
	env.load_module_enum("Keyboard", "KC_LALT",			KC_LALT);
	env.load_module_enum("Keyboard", "KC_RALT",			KC_RALT);
	env.load_module_enum("Keyboard", "KC_LSUPER",		KC_LSUPER);
	env.load_module_enum("Keyboard", "KC_RSUPER",		KC_RSUPER);

	/* [0x30, 0x39] reserved for ASCII digits */
	env.load_module_enum("Keyboard", "KC_0",			KC_0);
	env.load_module_enum("Keyboard", "KC_1",			KC_1);
	env.load_module_enum("Keyboard", "KC_2",			KC_2);
	env.load_module_enum("Keyboard", "KC_3",			KC_3);
	env.load_module_enum("Keyboard", "KC_4",			KC_4);
	env.load_module_enum("Keyboard", "KC_5",			KC_5);
	env.load_module_enum("Keyboard", "KC_6",			KC_6);
	env.load_module_enum("Keyboard", "KC_7",			KC_7);
	env.load_module_enum("Keyboard", "KC_8",			KC_8);
	env.load_module_enum("Keyboard", "KC_9",			KC_9);

	/* [0x41, 0x5A] reserved for ASCII alph,abet */
	env.load_module_enum("Keyboard", "KC_A",			KC_A);
	env.load_module_enum("Keyboard", "KC_B",			KC_B);
	env.load_module_enum("Keyboard", "KC_C",			KC_C);
	env.load_module_enum("Keyboard", "KC_D",			KC_D);
	env.load_module_enum("Keyboard", "KC_E",			KC_E);
	env.load_module_enum("Keyboard", "KC_F",			KC_F);
	env.load_module_enum("Keyboard", "KC_G",			KC_G);
	env.load_module_enum("Keyboard", "KC_H",			KC_H);
	env.load_module_enum("Keyboard", "KC_I",			KC_I);
	env.load_module_enum("Keyboard", "KC_J",			KC_J);
	env.load_module_enum("Keyboard", "KC_K",			KC_K);
	env.load_module_enum("Keyboard", "KC_L",			KC_L);
	env.load_module_enum("Keyboard", "KC_M",			KC_M);
	env.load_module_enum("Keyboard", "KC_N",			KC_N);
	env.load_module_enum("Keyboard", "KC_O",			KC_O);
	env.load_module_enum("Keyboard", "KC_P",			KC_P);
	env.load_module_enum("Keyboard", "KC_Q",			KC_Q);
	env.load_module_enum("Keyboard", "KC_R",			KC_R);
	env.load_module_enum("Keyboard", "KC_S",			KC_S);
	env.load_module_enum("Keyboard", "KC_T",			KC_T);
	env.load_module_enum("Keyboard", "KC_U",			KC_U);
	env.load_module_enum("Keyboard", "KC_V",			KC_V);
	env.load_module_enum("Keyboard", "KC_W",			KC_W);
	env.load_module_enum("Keyboard", "KC_X",			KC_X);
	env.load_module_enum("Keyboard", "KC_Y",			KC_Y);
	env.load_module_enum("Keyboard", "KC_Z",			KC_Z);

	/* [0x61, 0x7A] reserved for ASCII alphabet */
	env.load_module_enum("Keyboard", "KC_a",			KC_a);
	env.load_module_enum("Keyboard", "KC_b",			KC_b);
	env.load_module_enum("Keyboard", "KC_c",			KC_c);
	env.load_module_enum("Keyboard", "KC_d",			KC_d);
	env.load_module_enum("Keyboard", "KC_e",			KC_e);
	env.load_module_enum("Keyboard", "KC_f",			KC_f);
	env.load_module_enum("Keyboard", "KC_g",			KC_g);
	env.load_module_enum("Keyboard", "KC_h",			KC_h);
	env.load_module_enum("Keyboard", "KC_i",			KC_i);
	env.load_module_enum("Keyboard", "KC_j",			KC_j);
	env.load_module_enum("Keyboard", "KC_k",			KC_k);
	env.load_module_enum("Keyboard", "KC_l",			KC_l);
	env.load_module_enum("Keyboard", "KC_m",			KC_m);
	env.load_module_enum("Keyboard", "KC_n",			KC_n);
	env.load_module_enum("Keyboard", "KC_o",			KC_o);
	env.load_module_enum("Keyboard", "KC_p",			KC_p);
	env.load_module_enum("Keyboard", "KC_q",			KC_q);
	env.load_module_enum("Keyboard", "KC_r",			KC_r);
	env.load_module_enum("Keyboard", "KC_s",			KC_s);
	env.load_module_enum("Keyboard", "KC_t",			KC_t);
	env.load_module_enum("Keyboard", "KC_u",			KC_u);
	env.load_module_enum("Keyboard", "KC_v",			KC_v);
	env.load_module_enum("Keyboard", "KC_w",			KC_w);
	env.load_module_enum("Keyboard", "KC_x",			KC_x);
	env.load_module_enum("Keyboard", "KC_y",			KC_y);
	env.load_module_enum("Keyboard", "KC_z",			KC_z);

	// The last key _must_ be <= 0xFF
	env.load_module_enum("Keyboard", "KC_COUNT",		KC_COUNT);
}

} // namespace crown
