#include "StringSetting.h"

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

	if (g_string_settings_head == NULL)
	{
		g_string_settings_head = this;
		m_next = NULL;
	}
	else
	{
		m_next = g_string_settings_head;
		g_string_settings_head = this;
	}
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

StringSetting&	StringSetting::operator=(const char* value)
{
	m_value = value;

	return *this;
}

} // namespace crown