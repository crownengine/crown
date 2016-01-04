/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "input_device.h"
#include "error.h"
#include "allocator.h"
#include "string_utils.h"
#include "string_id.h"
#include <string.h> // strcpy, memset

namespace crown
{

const char* InputDevice::name() const
{
	return _name;
}

bool InputDevice::connected() const
{
	return _connected;
}

uint8_t InputDevice::num_buttons() const
{
	return _num_buttons;
}

uint8_t InputDevice::num_axes() const
{
	return _num_axes;
}

bool InputDevice::pressed(uint8_t id) const
{
	CE_ASSERT(id < _num_buttons, "Index out of bounds");
	return (~_last_state[id] & _current_state[id]) != 0;
}

bool InputDevice::released(uint8_t id) const
{
	CE_ASSERT(id < _num_buttons, "Index out of bounds");
	return (_last_state[id] & ~_current_state[id]) != 0;
}

bool InputDevice::any_pressed() const
{
	return pressed(_last_button);
}

bool InputDevice::any_released() const
{
	return released(_last_button);
}

Vector3 InputDevice::axis(uint8_t id) const
{
	CE_ASSERT(id < _num_axes, "Index out of bounds");
	return _axis[id];
}

const char* InputDevice::button_name(uint8_t id)
{
	CE_ASSERT(id < _num_buttons, "Index out of bounds");
	return _button_name[id];
}

const char* InputDevice::axis_name(uint8_t id)
{
	CE_ASSERT(id < _num_axes, "Index out of bounds");
	return _axis_name[id];
}

uint8_t InputDevice::button_id(StringId32 name)
{
	for (uint32_t i = 0; i < _num_buttons; ++i)
	{
		if (_button_hash[i] == name)
			return i;
	}

	CE_ASSERT(false, "Unknown button name");
	return 0;
}

uint8_t InputDevice::axis_id(StringId32 name)
{
	for (uint32_t i = 0; i < _num_axes; ++i)
	{
		if (_axis_hash[i] == name)
			return i;
	}

	CE_ASSERT(false, "Unknown axis name");
	return 0;
}

void InputDevice::set_connected(bool connected)
{
	_connected = connected;
}

void InputDevice::set_button_state(uint8_t i, bool state)
{
	CE_ASSERT(i < _num_buttons, "Index out of bounds");
	_last_button = i;
	_current_state[i] = state;
}

void InputDevice::set_axis(uint8_t i, const Vector3& value)
{
	CE_ASSERT(i < _num_axes, "Index out of bounds");
	_axis[i] = value;
}

void InputDevice::update()
{
	memcpy(_last_state, _current_state, sizeof(uint8_t)*_num_buttons);
}

InputDevice* InputDevice::create(Allocator& a, const char* name, uint8_t num_buttons, uint8_t num_axes, const char** button_names, const char** axis_names)
{
	const uint32_t size = 0
		+ sizeof(InputDevice)
		+ sizeof(uint8_t)*num_buttons*2
		+ sizeof(Vector3)*num_axes
		+ sizeof(char*)*num_buttons
		+ sizeof(char*)*num_axes
		+ sizeof(StringId32)*num_buttons
		+ sizeof(StringId32)*num_axes
		+ strlen32(name) + 1
		;

	InputDevice* id = (InputDevice*)a.allocate(size);

	id->_connected = false;
	id->_num_buttons = num_buttons;
	id->_num_axes = num_axes;
	id->_last_button = 0;

	id->_last_state = (uint8_t*)&id[1];
	id->_current_state = (uint8_t*)(id->_last_state + num_buttons);
	id->_axis = (Vector3*)(id->_current_state + num_buttons);
	id->_button_name = (const char**)(id->_axis + num_axes);
	id->_axis_name = (const char**)(id->_button_name + num_buttons);
	id->_button_hash = (StringId32*)(id->_axis_name + num_axes);
	id->_axis_hash = (StringId32*)(id->_button_hash + num_buttons);
	id->_name = (char*)(id->_axis_hash + num_axes);

	memset(id->_last_state, 0, sizeof(uint8_t)*num_buttons);
	memset(id->_current_state, 0, sizeof(uint8_t)*num_buttons);
	memset(id->_axis, 0, sizeof(Vector3)*num_axes);
	memcpy(id->_button_name, button_names, sizeof(const char*)*num_buttons);
	memcpy(id->_axis_name, axis_names, sizeof(const char*)*num_axes);

	for (uint32_t i = 0; i < num_buttons; ++i)
		id->_button_hash[i] = StringId32(button_names[i]);

	for (uint32_t i = 0; i < num_axes; ++i)
		id->_axis_hash[i] = StringId32(axis_names[i]);

	strcpy(id->_name, name);

	return id;
}

void InputDevice::destroy(Allocator& a, InputDevice* id)
{
	a.deallocate(id);
}

} // namespace crown
