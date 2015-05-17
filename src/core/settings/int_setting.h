/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"

namespace crown
{

/// Facility to store global integer settings.
class IntSetting
{
public:

						IntSetting(const char* name, const char* synopsis, int32_t value, int32_t min, int32_t max);

	const char*			name() const;
	const char*			synopsis() const;

	int32_t				value() const;
	int32_t				min() const;
	int32_t				max() const;

						operator int();

	IntSetting&			operator=(const int32_t value);

public:

	/// Returns the setting @name or NULL if not found.
	static IntSetting*	find_setting(const char* name);

private:

	const char*			m_name;
	const char*			m_synopsis;

	int32_t				m_value;
	int32_t				m_min;
	int32_t				m_max;

	IntSetting*			m_next;
};

} // namespace crown

