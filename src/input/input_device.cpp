/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "input_device.h"
#include "error.h"
#include <string.h> // memcpy

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

bool InputDevice::pressed(uint8_t i) const
{
	CE_ASSERT(i < _num_buttons, "Index out of bounds");
	return (~_last_state[i] & _current_state[i]) != 0;
}

bool InputDevice::released(uint8_t i) const
{
	CE_ASSERT(i < _num_buttons, "Index out of bounds");
	return (_last_state[i] & ~_current_state[i]) != 0;
}

bool InputDevice::any_pressed() const
{
	return pressed(_last_button);
}

bool InputDevice::any_released() const
{
	return released(_last_button);
}

Vector3 InputDevice::axis(uint8_t i) const
{
	CE_ASSERT(i < _num_axes, "Index out of bounds");
	return _axis[i];
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

} // namespace crown
