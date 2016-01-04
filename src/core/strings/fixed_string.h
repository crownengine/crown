/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "string_utils.h"
#include <algorithm>

namespace crown
{

class FixedString
{
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

	FixedString(const char* str, uint32_t len)
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
		const uint32_t len = strlen32(str);
		return _length == len && !strncmp(_data, str, len);
	}

	bool operator==(const FixedString& b) const
	{
		return (_length == b._length) && !strncmp(_data, b._data, _length);
	}

	bool operator<(const FixedString& b) const
	{
		const uint32_t len = std::max(_length, b._length);
		return strncmp(_data, b._data, len) < 0;
	}

	uint32_t length() const
	{
		return _length;
	}

	const char* data() const
	{
		return _data;
	}

private:

	uint32_t _length;
	const char* _data;
};

} // namespace crown
