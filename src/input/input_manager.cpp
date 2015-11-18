/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "input_manager.h"
#include "input_device.h"
#include "memory.h"
#include "vector3.h"

namespace crown
{

InputManager::InputManager()
	: _keyboard(NULL)
	, _mouse(NULL)
	, _touch(NULL)
{
	_keyboard = InputDevice::create_input_device(default_allocator()
		, "Keyboard"
		, KeyboardButton::COUNT
		, 0
		);
	_mouse = InputDevice::create_input_device(default_allocator()
		, "Mouse"
		, MouseButton::COUNT
		, MouseAxis::COUNT
		);
	_touch = InputDevice::create_input_device(default_allocator()
		, "Touch"
		, TouchButton::COUNT
		, TouchButton::COUNT
		);

	for (uint8_t i = 0; i < CROWN_MAX_JOYPADS; ++i)
	{
		_joypad[i] = InputDevice::create_input_device(default_allocator()
			, "Joypad"
			, JoypadButton::COUNT
			, JoypadAxis::COUNT
			);
	}

	_keyboard->set_connected(true);
	_mouse->set_connected(true);
	_touch->set_connected(true);
}

InputManager::~InputManager()
{
	for (uint8_t i = 0; i < CROWN_MAX_JOYPADS; ++i)
		InputDevice::destroy_input_device(default_allocator(), _joypad[i]);

	InputDevice::destroy_input_device(default_allocator(), _touch);
	InputDevice::destroy_input_device(default_allocator(), _mouse);
	InputDevice::destroy_input_device(default_allocator(), _keyboard);
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

InputDevice* InputManager::joypad(uint8_t i)
{
	CE_ASSERT(i < CROWN_MAX_JOYPADS, "Index out of bounds");
	return _joypad[i];
}

void InputManager::update()
{
	_keyboard->update();
	_mouse->update();
	_touch->update();

	for (uint8_t i = 0; i < CROWN_MAX_JOYPADS; ++i)
		_joypad[i]->update();
}

} // namespace crown
