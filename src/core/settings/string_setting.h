/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"

namespace crown
{

/// Facility to store global string settings.
class StringSetting
{
public:

	StringSetting(const char* name, const char* synopsis, const char* value);

	const char* name() const;
	const char* synopsis() const;

	const char* value() const;

	StringSetting& operator=(const char* value);

public:

	/// Returns the setting @name or NULL if not found.
	static StringSetting*	find_setting(const char* name);

private:

	const char* _name;
	const char* _synopsis;
	const char* _value;

	StringSetting* _next;
};

} // namespace crown
