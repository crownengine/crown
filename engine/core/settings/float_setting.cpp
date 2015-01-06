/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "float_setting.h"
#include "string_utils.h"

namespace crown
{

static FloatSetting* g_float_settings_head = NULL;

FloatSetting::FloatSetting(const char* name, const char* synopsis, float value, float min, float max) :
	m_name(name),
	m_synopsis(synopsis),
	m_value(0),
	m_min(min),
	m_max(max),
	m_next(NULL)
{
	*this = value;

	if (g_float_settings_head != NULL)
	{
		m_next = g_float_settings_head;
	}

	g_float_settings_head = this;
}

const char* FloatSetting::name() const
{
	return m_name;
}

const char* FloatSetting::synopsis() const
{
	return m_synopsis;
}

float FloatSetting::value() const
{
	return m_value;
}

float FloatSetting::min() const
{
	return m_min;
}

float FloatSetting::max() const
{
	return m_max;
}

FloatSetting::operator float()
{
	return m_value;
}

FloatSetting& FloatSetting::operator=(const float value)
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

FloatSetting* FloatSetting::find_setting(const char* name)
{
	FloatSetting* head = g_float_settings_head;

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

