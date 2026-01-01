/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/error/error.inl"
#include "core/math/constants.h"
#include "core/math/vector3.inl"
#include "device/input_device.h"
#include "device/input_manager.h"

namespace crown
{
static const char *s_keyboard_button_names[] =
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

static const char *s_mouse_button_names[] =
{
	"left",    // MouseButton::LEFT
	"middle",  // MouseButton::MIDDLE
	"right",   // MouseButton::RIGHT
	"extra_1", // MouseButton::EXTRA_1
	"extra_2"  // MouseButton::EXTRA_2
};
CE_STATIC_ASSERT(countof(s_mouse_button_names) == MouseButton::COUNT);

static const char *s_mouse_axis_names[] =
{
	"cursor",       // MouseAxis::CURSOR
	"cursor_delta", // MouseAxis::CURSOR_DELTA
	"wheel"         // MouseAxis::WHEEL
};
CE_STATIC_ASSERT(countof(s_mouse_axis_names) == MouseAxis::COUNT);

static const char *s_touch_button_names[] =
{
	"pointer_0", // TouchButton::POINTER_0
	"pointer_1", // TouchButton::POINTER_1
	"pointer_2", // TouchButton::POINTER_2
	"pointer_3"  // TouchButton::POINTER_3
};
CE_STATIC_ASSERT(countof(s_touch_button_names) == TouchButton::COUNT);

static const char *s_touch_axis_names[] =
{
	"pointer_0", // TouchAxis::POINTER_0
	"pointer_1", // TouchAxis::POINTER_1
	"pointer_2", // TouchAxis::POINTER_2
	"pointer_3"  // TouchAxis::POINTER_3
};
CE_STATIC_ASSERT(countof(s_touch_axis_names) == TouchAxis::COUNT);

static const char *s_pad_button_names[] =
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

static const char *s_pad_axis_names[] =
{
	"left",         // JoypadAxis::LEFT
	"right",        // JoypadAxis::RIGHT
	"trigger_left", // JoypadAxis::TRIGGER_LEFT
	"trigger_right" // JoypadAxis::TRIGGER_RIGHT
};
CE_STATIC_ASSERT(countof(s_pad_axis_names) == JoypadAxis::COUNT);

InputManager::InputManager(Allocator &a)
	: _allocator(&a)
	, _keyboard(NULL)
	, _mouse(NULL)
	, _touch(NULL)
	, _mouse_last_x(INT16_MAX)
	, _mouse_last_y(INT16_MAX)
	, _has_delta_axis_event(false)
	, _num_events(0)
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

	for (u8 i = 0; i < CROWN_MAX_JOYPADS; ++i) {
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

InputDevice *InputManager::device_from_type(u16 type, u16 num)
{
	switch (type) {
	case InputDeviceType::KEYBOARD:
		return _keyboard;

	case InputDeviceType::MOUSE:
		return _mouse;

	case InputDeviceType::TOUCHSCREEN:
		return _touch;

	case InputDeviceType::JOYPAD:
		return _joypad[num];

	default:
		CE_FATAL("Unknown device type: %d", type);
		return NULL;
	}
}

InputDevice *InputManager::keyboard()
{
	return _keyboard;
}

InputDevice *InputManager::mouse()
{
	return _mouse;
}

InputDevice *InputManager::touch()
{
	return _touch;
}

u8 InputManager::num_joypads()
{
	return countof(_joypad);
}

InputDevice *InputManager::joypad(u8 i)
{
	CE_ASSERT(i < CROWN_MAX_JOYPADS, "Index out of bounds");
	return _joypad[i];
}

/// Applies the deadzone settings for the axis @a id to @a axis and returns its value.
static Vector3 deadzone(const InputDevice *dev, u8 id, const Vector3 &axis)
{
	Vector3 out;

	switch (dev->_deadzone_mode[id]) {
	case DeadzoneMode::RAW:
		out = axis;
		break;

	case DeadzoneMode::INDEPENDENT:
		out.x = fabs(axis.x) < dev->_deadzone_size[id] ? 0.0f : axis.x;
		out.y = fabs(axis.y) < dev->_deadzone_size[id] ? 0.0f : axis.y;
		out.z = fabs(axis.z) < dev->_deadzone_size[id] ? 0.0f : axis.z;
		break;

	case DeadzoneMode::CIRCULAR:
		if (length(axis) < dev->_deadzone_size[id]) {
			out = VECTOR3_ZERO;
		} else {
			const f32 size = 1.0f - dev->_deadzone_size[id];
			const f32 size_inv = 1.0f / size;
			const f32 axis_len = length(axis);
			out = axis;
			out = normalize(out) * (axis_len - dev->_deadzone_size[id]) * size_inv;
		}
		break;

	default:
		CE_FATAL("Unknown deadzone mode");
		out = axis;
		break;
	}

	return out;
}

void InputManager::read(const OsEvent &event)
{
	InputDevice *dev;
	InputEvent input_ev;

	switch (event.type) {
	case OsEventType::BUTTON: {
		const ButtonEvent ev = event.button;

		dev = device_from_type(ev.device_id, ev.device_num);
		if (CE_UNLIKELY(dev == NULL))
			return;

		// Publish the event.
		dev->set_button(ev.button_num, ev.pressed);

		input_ev.id     = ev.button_num;
		input_ev.type   = ev.pressed ? InputEventType::BUTTON_PRESSED : InputEventType::BUTTON_RELEASED;
		input_ev.value  = VECTOR3_ZERO;
		input_ev.device = dev;

		_events[_num_events++] = input_ev;
		break;
	}

	case OsEventType::AXIS: {
		const AxisEvent ev = event.axis;

		dev = device_from_type(ev.device_id, ev.device_num);
		if (CE_UNLIKELY(dev == NULL))
			return;

		Vector3 axis;

		if (ev.device_id == InputDeviceType::MOUSE) {
			if (ev.axis_num == MouseAxis::CURSOR_DELTA) {
				const Vector3 delta = _has_delta_axis_event ?
					dev->axis(MouseAxis::CURSOR_DELTA)
					: VECTOR3_ZERO
					;

				axis.x = delta.x + ev.axis_x;
				axis.y = delta.y + ev.axis_y;
				axis.z = 0.0f;

				_has_delta_axis_event = true;
			} else {
				axis.x = ev.axis_x;
				axis.y = ev.axis_y;
				axis.z = ev.axis_z;
			}
		} else if (ev.device_id == InputDeviceType::JOYPAD) {
			axis.x = (f32)ev.axis_x / (f32)INT16_MAX;
			axis.y = (f32)ev.axis_y / (f32)INT16_MAX;
			axis.z = (f32)ev.axis_z / (f32)INT16_MAX;

			axis = deadzone(dev, ev.axis_num, axis);
		} else {
			axis.x = ev.axis_x;
			axis.y = ev.axis_y;
			axis.z = ev.axis_z;
		}

		// Publish the event.
		dev->set_axis(ev.axis_num, axis.x, axis.y, axis.z);

		input_ev.id     = ev.axis_num;
		input_ev.type   = InputEventType::AXIS_CHANGED;
		input_ev.value  = axis;
		input_ev.device = dev;

		_events[_num_events++] = input_ev;
		break;
	}

	case OsEventType::STATUS: {
		const StatusEvent ev = event.status;

		dev = device_from_type(ev.device_id, ev.device_num);
		if (CE_UNLIKELY(dev == NULL))
			return;

		dev->_connected = ev.connected;
		break;
	}

	default:
		CE_FATAL("Unknown input event type");
		break;
	}
}

void InputManager::update()
{
	_keyboard->update();

	if (!_has_delta_axis_event) {
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

	_num_events = 0;
}

} // namespace crown
