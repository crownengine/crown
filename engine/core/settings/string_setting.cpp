/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#include "string_setting.h"
#include "string_utils.h"

namespace crown
{

static StringSetting* g_string_settings_head = NULL;

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
const char* StringSetting::name() const
{
	return m_name;
}

//-----------------------------------------------------------------------------
const char* StringSetting::synopsis() const
{
	return m_synopsis;
}

//-----------------------------------------------------------------------------
const char*	StringSetting::value() const
{
	return m_value;
}

//-----------------------------------------------------------------------------
StringSetting& StringSetting::operator=(const char* value)
{
	m_value = value;

	return *this;
}

//-----------------------------------------------------------------------------
StringSetting* StringSetting::find_setting(const char* name)
{
	StringSetting* head = g_string_settings_head;

	while (head != NULL)
	{
		if (string::strcmp(name, head->name()) == 0)
		{
			return head;
		}

		head = head->m_next;
	}

	return NULL;
}

} // namespace crown
