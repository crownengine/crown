/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "string_utils.h"
#include <algorithm>

namespace crown
{
/// Fixed string.
///
/// @ingroup String
class FixedString
{
	u32 _length;
	const char* _data;

public:

	FixedString()
		: _length(0)
		, _data(NULL)
	{
	}

	FixedString(const char* str)
		: _length(strlen32(str))
		, _data(str)
	{
	}

	FixedString(const char* str, u32 len)
		: _length(len)
		, _data(str)
	{
	}

	FixedString(const FixedString& b)
		: _length(b._length)
		, _data(b._data)
	{
	}

	FixedString& operator=(const char* str)
	{
		_length = strlen32(str);
		_data = str;
		return *this;
	}

	bool operator==(const char* str) const
	{
		const u32 len = strlen32(str);
		return _length == len && !strncmp(_data, str, len);
	}

	bool operator==(const FixedString& b) const
	{
		return (_length == b._length) && !strncmp(_data, b._data, _length);
	}

	bool operator<(const FixedString& b) const
	{
		const u32 len = std::max(_length, b._length);
		return strncmp(_data, b._data, len) < 0;
	}

	u32 length() const
	{
		return _length;
	}

	const char* data() const
	{
		return _data;
	}
};

} // namespace crown
