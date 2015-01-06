/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "int_setting.h"
#include "string_utils.h"

namespace crown
{

static IntSetting* g_int_settings_head = NULL;

IntSetting::IntSetting(const char* name, const char* synopsis, int32_t value, int32_t min, int32_t max) :
	m_name(name),
	m_synopsis(synopsis),
	m_value(0),
	m_min(min),
	m_max(max),
	m_next(NULL)
{
	*this = value;

	if (g_int_settings_head != NULL)
	{
		m_next = g_int_settings_head;
	}

	g_int_settings_head = this;
}

const char* IntSetting::name() const
{
	return m_name;
}

const char* IntSetting::synopsis() const
{
	return m_synopsis;
}

int32_t IntSetting::value() const
{
	return m_value;
}

int32_t IntSetting::min() const
{
	return m_min;
}

int32_t IntSetting::max() const
{
	return m_max;
}

IntSetting::operator int()
{
	return m_value;
}

IntSetting& IntSetting::operator=(const int32_t value)
{
	if (value > m_max)
	{
		m_value = m_max;
	}
	else if (value < m_min)
	{
		m_value = m_min;
	}
	else
	{
		m_value = value;
	}

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

		head = head->m_next;
	}

	return NULL;
}

} // namespace crown

