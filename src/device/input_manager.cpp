/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/error/error.inl"
#include "core/math/vector3.inl"
#include "device/input_device.h"
#include "device/input_manager.h"

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
	"ins",          // KeyboardButton::INS
	"del",          // KeyboardButton::DEL
	"end",          // KeyboardButton::END
	"ctrl_left",    // KeyboardButton::CTRL_LEFT
	"ctrl_right",   // KeyboardButton::CTRL_RIGHT
	"shift_left",   // KeyboardButton::SHIFT_LEFT
	"shift_right",  // KeyboardButton::SHIFT_RIGHT
	"caps_lock",    // KeyboardButton::CAPS_LOCK
	"alt_left",     // KeyboardButton::ALT_LEFT
	"alt_right",    // KeyboardButton::ALT_RIGHT
	"super_left",   // KeyboardButton::SUPER_LEFT
	"super_right",  // KeyboardButton::SUPER_RIGHT
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
CE_STATIC_ASSERT(countof(s_keyboard_button_names) == KeyboardButton::COUNT);

static const char* s_mouse_button_names[] =
{
	"left",    // MouseButton::LEFT
	"middle",  // MouseButton::MIDDLE
	"right",   // MouseButton::RIGHT
	"extra_1", // MouseButton::EXTRA_1
	"extra_2"  // MouseButton::EXTRA_2
};
CE_STATIC_ASSERT(countof(s_mouse_button_names) == MouseButton::COUNT);

static const char* s_mouse_axis_names[] =
{
	"cursor",       // MouseAxis::CURSOR
	"cursor_delta", // MouseAxis::CURSOR_DELTA
	"wheel"         // MouseAxis::WHEEL
};
CE_STATIC_ASSERT(countof(s_mouse_axis_names) == MouseAxis::COUNT);

static const char* s_touch_button_names[] =
{
	"pointer_0", // TouchButton::POINTER_0
	"pointer_1", // TouchButton::POINTER_1
	"pointer_2", // TouchButton::POINTER_2
	"pointer_3"  // TouchButton::POINTER_3
};
CE_STATIC_ASSERT(countof(s_touch_button_names) == TouchButton::COUNT);

static const char* s_touch_axis_names[] =
{
	"pointer_0", // TouchAxis::POINTER_0
	"pointer_1", // TouchAxis::POINTER_1
	"pointer_2", // TouchAxis::POINTER_2
	"pointer_3"  // TouchAxis::POINTER_3
};
CE_STATIC_ASSERT(countof(s_touch_axis_names) == TouchAxis::COUNT);

static const char* s_pad_button_names[] =
{
	"up",             // JoypadButton::UP
	"down",           // JoypadButton::DOWN
	"left",           // JoypadButton::LEFT
	"right",          // JoypadButton::RIGHT
	"start",          // JoypadButton::START
	"back",           // JoypadButton::BACK
	"guide",          // JoypadButton::GUIDE
	"thumb_left",     // JoypadButton::THUMB_LEFT
	"thumb_right",    // JoypadButton::THUMB_RIGHT
	"shoulder_left",  // JoypadButton::SHOULDER_LEFT
	"shoulder_right", // JoypadButton::SHOULDER_RIGHT
	"a",              // JoypadButton::A
	"b",              // JoypadButton::B
	"x",              // JoypadButton::X
	"y"               // JoypadButton::Y
};
CE_STATIC_ASSERT(countof(s_pad_button_names) == JoypadButton::COUNT);

static const char* s_pad_axis_names[] =
{
	"left",         // JoypadAxis::LEFT
	"right",        // JoypadAxis::RIGHT
	"trigger_left", // JoypadAxis::TRIGGER_LEFT
	"trigger_right" // JoypadAxis::TRIGGER_RIGHT
};
CE_STATIC_ASSERT(countof(s_pad_axis_names) == JoypadAxis::COUNT);

InputManager::InputManager(Allocator& a)
	: _allocator(&a)
	, _keyboard(NULL)
	, _mouse(NULL)
	, _touch(NULL)
	, _mouse_last_x(INT16_MAX)
	, _mouse_last_y(INT16_MAX)
	, _has_delta_axis_event(false)
{
	_keyboard = input_device::create(*_allocator
		, "Keyboard"
		, KeyboardButton::COUNT
		, 0
		, s_keyboard_button_names
		, NULL
		);
	_mouse = input_device::create(*_allocator
		, "Mouse"
		, MouseButton::COUNT
		, MouseAxis::COUNT
		, s_mouse_button_names
		, s_mouse_axis_names
		);
	_touch = input_device::create(*_allocator
		, "Touch"
		, TouchButton::COUNT
		, TouchAxis::COUNT
		, s_touch_button_names
		, s_touch_axis_names
		);

	for (u8 i = 0; i < CROWN_MAX_JOYPADS; ++i)
	{
		_joypad[i] = input_device::create(*_allocator
			, "Joypad"
			, JoypadButton::COUNT
			, JoypadAxis::COUNT
			, s_pad_button_names
			, s_pad_axis_names
			);
		_joypad[i]->set_deadzone(JoypadAxis::LEFT, DeadzoneMode::CIRCULAR, 0.24f);
		_joypad[i]->set_deadzone(JoypadAxis::RIGHT, DeadzoneMode::CIRCULAR, 0.27f);
		_joypad[i]->set_deadzone(JoypadAxis::TRIGGER_LEFT, DeadzoneMode::CIRCULAR, 0.12f);
		_joypad[i]->set_deadzone(JoypadAxis::TRIGGER_RIGHT, DeadzoneMode::CIRCULAR, 0.12f);
	}

	_keyboard->_connected = true;
	_mouse->_connected = true;
	_touch->_connected = true;
}

InputManager::~InputManager()
{
	for (u8 i = 0; i < CROWN_MAX_JOYPADS; ++i)
		input_device::destroy(*_allocator, *_joypad[i]);

	input_device::destroy(*_allocator, *_touch);
	input_device::destroy(*_allocator, *_mouse);
	input_device::destroy(*_allocator, *_keyboard);
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
	return countof(_joypad);
}

InputDevice* InputManager::joypad(u8 i)
{
	CE_ASSERT(i < CROWN_MAX_JOYPADS, "Index out of bounds");
	return _joypad[i];
}

void InputManager::read(const OsEvent& event)
{
	switch (event.type)
	{
	case OsEventType::BUTTON:
		{
			const ButtonEvent ev = event.button;
			switch (ev.device_id)
			{
			case InputDeviceType::KEYBOARD:
				_keyboard->set_button(ev.button_num, ev.pressed);
				break;

			case InputDeviceType::MOUSE:
				_mouse->set_button(ev.button_num, ev.pressed);
				break;

			case InputDeviceType::TOUCHSCREEN:
				_touch->set_button(ev.button_num, ev.pressed);
				break;

			case InputDeviceType::JOYPAD:
				_joypad[ev.device_num]->set_button(ev.button_num, ev.pressed);
				break;
			}
		}
		break;

	case OsEventType::AXIS:
		{
			const AxisEvent ev = event.axis;
			switch (ev.device_id)
			{
			case InputDeviceType::MOUSE:
				if (ev.axis_num == MouseAxis::CURSOR_DELTA)
				{
					const Vector3 delta = _has_delta_axis_event ? _mouse->axis(MouseAxis::CURSOR_DELTA) : vector3(0, 0, 0);
					_mouse->set_axis(MouseAxis::CURSOR_DELTA
						, delta.x + ev.axis_x
						, delta.y + ev.axis_y
						, 0
						);
					_has_delta_axis_event = true;
				}
				else
					_mouse->set_axis(ev.axis_num, ev.axis_x, ev.axis_y, ev.axis_z);

				break;

			case InputDeviceType::JOYPAD:
				_joypad[ev.device_num]->set_axis(ev.axis_num
					, (f32)ev.axis_x / (f32)INT16_MAX
					, (f32)ev.axis_y / (f32)INT16_MAX
					, (f32)ev.axis_z / (f32)INT16_MAX
					);
				break;
			}
		}
		break;

	case OsEventType::STATUS:
		{
			const StatusEvent ev = event.status;
			switch (ev.device_id)
			{
			case InputDeviceType::JOYPAD:
				_joypad[ev.device_num]->_connected = ev.connected;
				break;
			}
		}
		break;

	default:
		CE_FATAL("Unknown input event type");
		break;
	}
}

void InputManager::update()
{
	_keyboard->update();

	if (!_has_delta_axis_event)
	{
		_mouse->set_axis(MouseAxis::CURSOR_DELTA
			, 0
			, 0
			, 0
			);
	}
	_has_delta_axis_event = false;
	_mouse->update();

	_touch->update();

	for (u8 i = 0; i < CROWN_MAX_JOYPADS; ++i)
		_joypad[i]->update();
}

} // namespace crown
