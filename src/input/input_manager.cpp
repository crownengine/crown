/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "input_manager.h"
#include "input_device.h"
#include "memory.h"

namespace crown
{
static const char* s_keyboard_button_names[] =
{
	"tab",          // KeyboardButton::TAB
	"enter",        // KeyboardButton::ENTER
	"escape",       // KeyboardButton::ESCAPE
	"space",        // KeyboardButton::SPACE
	"backspace",    // KeyboardButton::BACKSPACE
	"num_lock",     // KeyboardButton::NUM_LOCK
	"numpad_enter", // KeyboardButton::NUMPAD_ENTER
	"numpad_.",     // KeyboardButton::NUMPAD_DELETE
	"numpad_*",     // KeyboardButton::NUMPAD_MULTIPLY
	"numpad_+",     // KeyboardButton::NUMPAD_ADD
	"numpad_-",     // KeyboardButton::NUMPAD_SUBTRACT
	"numpad_/",     // KeyboardButton::NUMPAD_DIVIDE
	"numpad_0",     // KeyboardButton::NUMPAD_0
	"numpad_1",     // KeyboardButton::NUMPAD_1
	"numpad_2",     // KeyboardButton::NUMPAD_2
	"numpad_3",     // KeyboardButton::NUMPAD_3
	"numpad_4",     // KeyboardButton::NUMPAD_4
	"numpad_5",     // KeyboardButton::NUMPAD_5
	"numpad_6",     // KeyboardButton::NUMPAD_6
	"numpad_7",     // KeyboardButton::NUMPAD_7
	"numpad_8",     // KeyboardButton::NUMPAD_8
	"numpad_9",     // KeyboardButton::NUMPAD_9
	"f1",           // KeyboardButton::F1
	"f2",           // KeyboardButton::F2
	"f3",           // KeyboardButton::F3
	"f4",           // KeyboardButton::F4
	"f5",           // KeyboardButton::F5
	"f6",           // KeyboardButton::F6
	"f7",           // KeyboardButton::F7
	"f8",           // KeyboardButton::F8
	"f9",           // KeyboardButton::F9
	"f10",          // KeyboardButton::F10
	"f11",          // KeyboardButton::F11
	"f12",          // KeyboardButton::F12
	"home",         // KeyboardButton::HOME
	"left",         // KeyboardButton::LEFT
	"up",           // KeyboardButton::UP
	"right",        // KeyboardButton::RIGHT
	"down",         // KeyboardButton::DOWN
	"page_up",      // KeyboardButton::PAGE_UP
	"page_down",    // KeyboardButton::PAGE_DOWN
	"delete",       // KeyboardButton::DELETE
	"end",          // KeyboardButton::END
	"left_ctrl",    // KeyboardButton::LEFT_CTRL
	"right_ctrl",   // KeyboardButton::RIGHT_CTRL
	"left_shift",   // KeyboardButton::LEFT_SHIFT
	"right_shift",  // KeyboardButton::RIGHT_SHIFT
	"caps_lock",    // KeyboardButton::CAPS_LOCK
	"left_alt",     // KeyboardButton::LEFT_ALT
	"right_alt",    // KeyboardButton::RIGHT_ALT
	"left_super",   // KeyboardButton::LEFT_SUPER
	"right_super",  // KeyboardButton::RIGHT_SUPER
	"0",            // KeyboardButton::NUMBER_0
	"1",            // KeyboardButton::NUMBER_1
	"2",            // KeyboardButton::NUMBER_2
	"3",            // KeyboardButton::NUMBER_3
	"4",            // KeyboardButton::NUMBER_4
	"5",            // KeyboardButton::NUMBER_5
	"6",            // KeyboardButton::NUMBER_6
	"7",            // KeyboardButton::NUMBER_7
	"8",            // KeyboardButton::NUMBER_8
	"9",            // KeyboardButton::NUMBER_9
	"a",            // KeyboardButton::A
	"b",            // KeyboardButton::B
	"c",            // KeyboardButton::C
	"d",            // KeyboardButton::D
	"e",            // KeyboardButton::E
	"f",            // KeyboardButton::F
	"g",            // KeyboardButton::G
	"h",            // KeyboardButton::H
	"i",            // KeyboardButton::I
	"j",            // KeyboardButton::J
	"k",            // KeyboardButton::K
	"l",            // KeyboardButton::L
	"m",            // KeyboardButton::M
	"n",            // KeyboardButton::N
	"o",            // KeyboardButton::O
	"p",            // KeyboardButton::P
	"q",            // KeyboardButton::Q
	"r",            // KeyboardButton::R
	"s",            // KeyboardButton::S
	"t",            // KeyboardButton::T
	"u",            // KeyboardButton::U
	"v",            // KeyboardButton::V
	"w",            // KeyboardButton::W
	"x",            // KeyboardButton::X
	"y",            // KeyboardButton::Y
	"z"             // KeyboardButton::Z
};
CE_STATIC_ASSERT(CE_COUNTOF(s_keyboard_button_names) == KeyboardButton::COUNT);

static const char* s_mouse_button_names[] =
{
	"left",    // MouseButton::LEFT
	"middle",  // MouseButton::MIDDLE
	"right",   // MouseButton::RIGHT
	"extra_1", // MouseButton::EXTRA_1
	"extra_2"  // MouseButton::EXTRA_2
};
CE_STATIC_ASSERT(CE_COUNTOF(s_mouse_button_names) == MouseButton::COUNT);

static const char* s_mouse_axis_names[] =
{
	"cursor",       // MouseAxis::CURSOR
	"cursor_delta", // MouseAxis::CURSOR_DELTA
	"wheel"         // MouseAxis::WHEEL
};
CE_STATIC_ASSERT(CE_COUNTOF(s_mouse_axis_names) == MouseAxis::COUNT);

static const char* s_touch_button_names[] =
{
	"pointer_0", // TouchButton::POINTER_0
	"pointer_1", // TouchButton::POINTER_1
	"pointer_2", // TouchButton::POINTER_2
	"pointer_3"  // TouchButton::POINTER_3
};
CE_STATIC_ASSERT(CE_COUNTOF(s_touch_button_names) == TouchButton::COUNT);

static const char* s_pad_button_names[] =
{
	"up",             // JoypadButton::UP
	"down",           // JoypadButton::DOWN
	"left",           // JoypadButton::LEFT
	"right",          // JoypadButton::RIGHT
	"start",          // JoypadButton::START
	"back",           // JoypadButton::BACK
	"guide",          // JoypadButton::GUIDE
	"left_thumb",     // JoypadButton::LEFT_THUMB
	"right_thumb",    // JoypadButton::RIGHT_THUMB
	"left_shoulder",  // JoypadButton::LEFT_SHOULDER
	"right_shoulder", // JoypadButton::RIGHT_SHOULDER
	"a",              // JoypadButton::A
	"b",              // JoypadButton::B
	"x",              // JoypadButton::X
	"y"               // JoypadButton::Y
};
CE_STATIC_ASSERT(CE_COUNTOF(s_pad_button_names) == JoypadButton::COUNT);

static const char* s_pad_axis_names[] =
{
	"left",  // JoypadAxis::LEFT
	"right"  // JoypadAxis::RIGHT
};
CE_STATIC_ASSERT(CE_COUNTOF(s_pad_axis_names) == JoypadAxis::COUNT);

InputManager::InputManager(Allocator& a)
	: _allocator(&a)
	, _keyboard(NULL)
	, _mouse(NULL)
	, _touch(NULL)
{
	_keyboard = InputDevice::create(*_allocator
		, "Keyboard"
		, KeyboardButton::COUNT
		, 0
		, s_keyboard_button_names
		, NULL
		);
	_mouse = InputDevice::create(*_allocator
		, "Mouse"
		, MouseButton::COUNT
		, MouseAxis::COUNT
		, s_mouse_button_names
		, s_mouse_axis_names
		);
	_touch = InputDevice::create(*_allocator
		, "Touch"
		, TouchButton::COUNT
		, 0
		, s_touch_button_names
		, NULL
		);

	for (u8 i = 0; i < CROWN_MAX_JOYPADS; ++i)
	{
		_joypad[i] = InputDevice::create(*_allocator
			, "Joypad"
			, JoypadButton::COUNT
			, JoypadAxis::COUNT
			, s_pad_button_names
			, s_pad_axis_names
			);
	}

	_keyboard->set_connected(true);
	_mouse->set_connected(true);
	_touch->set_connected(true);
}

InputManager::~InputManager()
{
	for (u8 i = 0; i < CROWN_MAX_JOYPADS; ++i)
		InputDevice::destroy(*_allocator, _joypad[i]);

	InputDevice::destroy(*_allocator, _touch);
	InputDevice::destroy(*_allocator, _mouse);
	InputDevice::destroy(*_allocator, _keyboard);
}

InputDevice* InputManager::keyboard()
{
	return _keyboard;
}

InputDevice* InputManager::mouse()
{
	return _mouse;
}

InputDevice* InputManager::touch()
{
	return _touch;
}

u8 InputManager::num_joypads()
{
	return CE_COUNTOF(_joypad);
}

InputDevice* InputManager::joypad(u8 i)
{
	CE_ASSERT(i < CROWN_MAX_JOYPADS, "Index out of bounds");
	return _joypad[i];
}

void InputManager::update()
{
	_keyboard->update();
	_mouse->update();
	_touch->update();

	for (u8 i = 0; i < CROWN_MAX_JOYPADS; ++i)
		_joypad[i]->update();
}

} // namespace crown
