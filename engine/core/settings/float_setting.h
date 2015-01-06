/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"

namespace crown
{

/// Facility to store global float settings.
class FloatSetting
{
public:

							FloatSetting(const char* name, const char* synopsis, float value, float min, float max);

	const char*				name() const;
	const char*				synopsis() const;

	float					value() const;
	float					min() const;
	float					max() const;

							operator float();

	FloatSetting&			operator=(const float value);

public:

	/// Returns the setting @name or NULL if not found.
	static FloatSetting*	find_setting(const char* name);

private:

	const char*				m_name;
	const char*				m_synopsis;

	float					m_value;
	float					m_min;
	float					m_max;

	FloatSetting*			m_next;
};

} // namespace crown

