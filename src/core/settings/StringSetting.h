#pragma once

#include "Types.h"

namespace crown
{

/// Facility to store global string settings.
class StringSetting
{
public:

						StringSetting(const char* name, const char* synopsis, const char* value);

	const char*			name() const;
	const char*			synopsis() const;

	const char*			value() const;

	StringSetting&		operator=(const char* value);

private:

	const char*			m_name;
	const char*			m_synopsis;
	const char*			m_value;

	StringSetting*		m_next;
};

} // namespace crown