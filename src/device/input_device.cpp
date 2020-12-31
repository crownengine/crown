/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/error/error.h"
#include "core/math/constants.h"
#include "core/math/math.h"
#include "core/math/vector3.inl"
#include "core/memory/allocator.h"
#include "core/memory/memory.inl"
#include "core/strings/string.inl"
#include "core/strings/string_id.inl"
#include "device/input_device.h"
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

u8 InputDevice::num_buttons() const
{
	return _num_buttons;
}

u8 InputDevice::num_axes() const
{
	return _num_axes;
}

bool InputDevice::pressed(u8 id) const
{
	return id < _num_buttons
		? (~_last_state[id] & _state[id]) != 0
		: false
		;
}

bool InputDevice::released(u8 id) const
{
	return id < _num_buttons
		? (_last_state[id] & ~_state[id]) != 0
		: false
		;
}

u8 InputDevice::any_pressed() const
{
	return pressed(_first_button[1]) ? _first_button[1] : UINT8_MAX;
}

u8 InputDevice::any_released() const
{
	return released(_first_button[0]) ? _first_button[0] : UINT8_MAX;
}

f32 InputDevice::button(u8 id) const
{
	return id < _num_buttons
		? f32(_state[id])
		: 0.0f
		;
}

Vector3 InputDevice::axis(u8 id) const
{
	if (id >= _num_axes)
		return VECTOR3_ZERO;

	Vector3 axis = _axis[id];
	switch (_deadzone_mode[id])
	{
	case DeadzoneMode::RAW:
		// No deadzone
		break;

	case DeadzoneMode::INDEPENDENT:
		axis.x = fabs(axis.x) < _deadzone_size[id] ? 0.0f : axis.x;
		axis.y = fabs(axis.y) < _deadzone_size[id] ? 0.0f : axis.y;
		axis.z = fabs(axis.z) < _deadzone_size[id] ? 0.0f : axis.z;
		break;

	case DeadzoneMode::CIRCULAR:
		if (length(axis) < _deadzone_size[id])
		{
			axis = VECTOR3_ZERO;
		}
		else
		{
			const f32 size = 1.0f - _deadzone_size[id];
			const f32 size_inv = 1.0f / size;
			const f32 axis_len = length(axis);
			axis = normalize(axis) * (axis_len - _deadzone_size[id]) * size_inv;
		}
		break;

	default:
		CE_FATAL("Unknown deadzone mode");
		break;
	}

	return axis;
}

const char* InputDevice::button_name(u8 id)
{
	return id < _num_buttons
		? _button_name[id]
		: NULL
		;
}

const char* InputDevice::axis_name(u8 id)
{
	return id < _num_axes
		? _axis_name[id]
		: NULL
		;
}

u8 InputDevice::button_id(StringId32 name)
{
	for (u32 i = 0; i < _num_buttons; ++i)
	{
		if (_button_hash[i] == name)
			return i;
	}

	return UINT8_MAX;
}

u8 InputDevice::axis_id(StringId32 name)
{
	for (u32 i = 0; i < _num_axes; ++i)
	{
		if (_axis_hash[i] == name)
			return i;
	}

	return UINT8_MAX;
}

f32 InputDevice::deadzone(u8 id, DeadzoneMode::Enum* deadzone_mode)
{
	if (id < _num_axes)
	{
		*deadzone_mode = (DeadzoneMode::Enum)_deadzone_mode[id];
		return _deadzone_size[id];
	}
	else
	{
		*deadzone_mode = DeadzoneMode::COUNT;
		return 0.0f;
	}
}

void InputDevice::set_deadzone(u8 id, DeadzoneMode::Enum deadzone_mode, f32 deadzone_size)
{
	if (id < _num_axes)
	{
		_deadzone_mode[id] = deadzone_mode;
		_deadzone_size[id] = deadzone_size;
	}
}

void InputDevice::set_button(u8 id, u8 state)
{
	CE_ASSERT(id < _num_buttons, "Index out of bounds");
	_state[id] = state;

	if (_first_button[state % countof(_first_button)] == UINT8_MAX)
		_first_button[state % countof(_first_button)] = id;
}

void InputDevice::set_axis(u8 id, f32 x, f32 y, f32 z)
{
	CE_ASSERT(id < _num_axes, "Index out of bounds");
	_axis[id].x = x;
	_axis[id].y = y;
	_axis[id].z = z;
}

void InputDevice::update()
{
	memcpy(_last_state, _state, sizeof(u8)*_num_buttons);
	_first_button[0] = UINT8_MAX;
	_first_button[1] = UINT8_MAX;
}

namespace input_device
{
	InputDevice* create(Allocator& a, const char* name, u8 num_buttons, u8 num_axes, const char** button_names, const char** axis_names)
	{
		const u32 size = 0
			+ sizeof(InputDevice) + alignof(InputDevice)
			+ sizeof(u8)*num_buttons*2 + alignof(u8)
			+ sizeof(Vector3)*num_axes + alignof(Vector3)
			+ sizeof(u32)*num_axes + alignof(u32)
			+ sizeof(f32)*num_axes + alignof(f32)
			+ sizeof(StringId32)*num_buttons + alignof(StringId32)
			+ sizeof(StringId32)*num_axes + alignof(StringId32)
			+ strlen32(name) + 1 + alignof(char)
			;

		InputDevice* id = (InputDevice*)a.allocate(size);

		id->_connected       = false;
		id->_num_buttons     = num_buttons;
		id->_num_axes        = num_axes;
		id->_first_button[0] = UINT8_MAX;
		id->_first_button[1] = UINT8_MAX;
		id->_button_name     = button_names;
		id->_axis_name       = axis_names;

		id->_last_state    = (u8*         )&id[1];
		id->_state         = (u8*         )memory::align_top(id->_last_state + num_buttons,  alignof(u8         ));
		id->_axis          = (Vector3*    )memory::align_top(id->_state + num_buttons,       alignof(Vector3    ));
		id->_deadzone_mode = (u32*        )memory::align_top(id->_axis + num_axes,           alignof(u32        ));
		id->_deadzone_size = (f32*        )memory::align_top(id->_deadzone_mode + num_axes,  alignof(f32        ));
		id->_button_hash   = (StringId32* )memory::align_top(id->_deadzone_size + num_axes,  alignof(StringId32 ));
		id->_axis_hash     = (StringId32* )memory::align_top(id->_button_hash + num_buttons, alignof(StringId32 ));
		id->_name          = (char*       )memory::align_top(id->_axis_hash + num_axes,      alignof(char       ));

		memset(id->_last_state, 0, sizeof(u8)*num_buttons);
		memset(id->_state, 0, sizeof(u8)*num_buttons);
		memset(id->_axis, 0, sizeof(Vector3)*num_axes);
		memset(id->_deadzone_mode, 0, sizeof(*id->_deadzone_mode)*num_axes);
		memset(id->_deadzone_size, 0, sizeof(*id->_deadzone_size)*num_axes);

		for (u32 i = 0; i < num_buttons; ++i)
			id->_button_hash[i] = StringId32(button_names[i]);

		for (u32 i = 0; i < num_axes; ++i)
			id->_axis_hash[i] = StringId32(axis_names[i]);

		strcpy(id->_name, name);

		return id;
	}

	void destroy(Allocator& a, InputDevice& id)
	{
		a.deallocate(&id);
	}

} // namespace input_device

} // namespace crown
