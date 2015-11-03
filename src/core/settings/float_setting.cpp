/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "float_setting.h"
#include "string_utils.h"
#include "math_utils.h"

namespace crown
{

static FloatSetting* g_float_settings_head = NULL;

FloatSetting::FloatSetting(const char* name, const char* synopsis, float value, float min, float max)
	: _name(name)
	, _synopsis(synopsis)
	, _value(0)
	, _min(min)
	, _max(max)
	, _next(NULL)
{
	*this = value;

	if (g_float_settings_head != NULL)
	{
		_next = g_float_settings_head;
	}

	g_float_settings_head = this;
}

const char* FloatSetting::name() const
{
	return _name;
}

const char* FloatSetting::synopsis() const
{
	return _synopsis;
}

float FloatSetting::value() const
{
	return _value;
}

float FloatSetting::min() const
{
	return _min;
}

float FloatSetting::max() const
{
	return _max;
}

FloatSetting::operator float()
{
	return _value;
}

FloatSetting& FloatSetting::operator=(const float value)
{
	_value = clamp(_min, _max, value);
	return *this;
}

FloatSetting* FloatSetting::find_setting(const char* name)
{
	FloatSetting* head = g_float_settings_head;

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

