/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "input_manager.h"
#include "input_device.h"
#include "memory.h"
#include "vector3.h"
#include <string.h> // strlen, strcpy, memset

namespace crown
{

InputManager::InputManager()
	: _keyboard(NULL)
	, _mouse(NULL)
	, _touch(NULL)
{
	_keyboard = create_input_device("Keyboard", KeyboardButton::COUNT, 0);
	_mouse = create_input_device("Mouse", MouseButton::COUNT, MouseAxis::COUNT);
	_touch = create_input_device("Touch", TouchButton::COUNT, TouchButton::COUNT);

	for (uint8_t i = 0; i < CROWN_MAX_JOYPADS; ++i)
		_joypad[i] = create_input_device("Joypad", JoypadButton::COUNT, JoypadAxis::COUNT);

	_keyboard->set_connected(true);
	_mouse->set_connected(true);
	_touch->set_connected(true);
}

InputManager::~InputManager()
{
	for (uint8_t i = 0; i < CROWN_MAX_JOYPADS; ++i)
		default_allocator().deallocate(_joypad[i]);

	default_allocator().deallocate(_touch);
	default_allocator().deallocate(_mouse);
	default_allocator().deallocate(_keyboard);
}

InputDevice* InputManager::create_input_device(const char* name, uint8_t num_buttons, uint8_t num_axes)
{
	const uint32_t size = 0
		+ sizeof(InputDevice)
		+ sizeof(uint8_t)*num_buttons*2
		+ sizeof(Vector3)*num_axes
		+ strlen(name) + 1;

	InputDevice* id = (InputDevice*)default_allocator().allocate(size);

	id->_connected = false;
	id->_num_buttons = num_buttons;
	id->_num_axes = num_axes;
	id->_last_button = 0;

	id->_last_state = (uint8_t*)&id[1];
	id->_current_state = (uint8_t*)(id->_last_state + num_buttons);
	id->_axis = (Vector3*)(id->_current_state + num_buttons);
	id->_name = (char*)(id->_axis + num_axes);

	memset(id->_last_state, 0, sizeof(uint8_t)*num_buttons);
	memset(id->_current_state, 0, sizeof(uint8_t)*num_buttons);
	memset(id->_axis, 0, sizeof(Vector3)*num_axes);
	strcpy(id->_name, name);

	return id;
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
