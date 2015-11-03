/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "int_setting.h"
#include "string_utils.h"
#include "math_utils.h"

namespace crown
{

static IntSetting* g_int_settings_head = NULL;

IntSetting::IntSetting(const char* name, const char* synopsis, int32_t value, int32_t min, int32_t max)
	: _name(name)
	, _synopsis(synopsis)
	, _value(0)
	, _min(min)
	, _max(max)
	, _next(NULL)
{
	*this = value;

	if (g_int_settings_head != NULL)
	{
		_next = g_int_settings_head;
	}

	g_int_settings_head = this;
}

const char* IntSetting::name() const
{
	return _name;
}

const char* IntSetting::synopsis() const
{
	return _synopsis;
}

int32_t IntSetting::value() const
{
	return _value;
}

int32_t IntSetting::min() const
{
	return _min;
}

int32_t IntSetting::max() const
{
	return _max;
}

IntSetting::operator int()
{
	return _value;
}

IntSetting& IntSetting::operator=(const int32_t value)
{
	_value = clamp(_min, _max, value);
	return *this;
}

IntSetting*	IntSetting::find_setting(const char* name)
{
	IntSetting* head = g_int_settings_head;

	while (head != NULL)
	{
		if (strcmp(name, head->name()) == 0)
		{
			return head;
		}

		head = head->_next;
	}

	return NULL;
}

} // namespace crown

