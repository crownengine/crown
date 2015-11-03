/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "string_setting.h"
#include "string_utils.h"

namespace crown
{

static StringSetting* g_string_settings_head = NULL;

StringSetting::StringSetting(const char* name, const char* synopsis, const char* value)
	: _name(name)
	, _synopsis(synopsis)
	, _value(value)
	, _next(NULL)
{
	*this = value;

	if (g_string_settings_head != NULL)
	{
		_next = g_string_settings_head;
	}

	g_string_settings_head = this;
}

const char* StringSetting::name() const
{
	return _name;
}

const char* StringSetting::synopsis() const
{
	return _synopsis;
}

const char*	StringSetting::value() const
{
	return _value;
}

StringSetting& StringSetting::operator=(const char* value)
{
	_value = value;
	return *this;
}

StringSetting* StringSetting::find_setting(const char* name)
{
	StringSetting* head = g_string_settings_head;

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
