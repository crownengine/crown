/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "string_setting.h"
#include "string_utils.h"

namespace crown
{

static StringSetting* g_string_settings_head = NULL;

StringSetting::StringSetting(const char* name, const char* synopsis, const char* value) :
	m_name(name),
	m_synopsis(synopsis),
	m_value(value),
	m_next(NULL)
{
	*this = value;

	if (g_string_settings_head != NULL)
	{
		m_next = g_string_settings_head;
	}

	g_string_settings_head = this;
}

const char* StringSetting::name() const
{
	return m_name;
}

const char* StringSetting::synopsis() const
{
	return m_synopsis;
}

const char*	StringSetting::value() const
{
	return m_value;
}

StringSetting& StringSetting::operator=(const char* value)
{
	m_value = value;

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

		head = head->m_next;
	}

	return NULL;
}

} // namespace crown
