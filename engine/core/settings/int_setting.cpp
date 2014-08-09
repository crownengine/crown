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

#include "int_setting.h"
#include "string_utils.h"

namespace crown
{

static IntSetting* g_int_settings_head = NULL;

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
const char* IntSetting::name() const
{
	return m_name;
}

//-----------------------------------------------------------------------------
const char* IntSetting::synopsis() const
{
	return m_synopsis;
}

//-----------------------------------------------------------------------------
int32_t IntSetting::value() const
{
	return m_value;
}

//-----------------------------------------------------------------------------
int32_t IntSetting::min() const
{
	return m_min;
}

//-----------------------------------------------------------------------------
int32_t IntSetting::max() const
{
	return m_max;
}

//-----------------------------------------------------------------------------
IntSetting::operator int()
{
	return m_value;
}

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
IntSetting*	IntSetting::find_setting(const char* name)
{
	IntSetting* head = g_int_settings_head;

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

