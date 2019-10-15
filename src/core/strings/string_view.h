/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/strings/string.h"

namespace crown
{
/// String view.
///
/// @ingroup String
struct StringView
{
	u32 _length;
	const char* _data;

	StringView()
		: _length(0)
		, _data(NULL)
	{
	}

	StringView(const char* str)
		: _length(strlen32(str))
		, _data(str)
	{
	}

	StringView(const char* str, u32 len)
		: _length(len)
		, _data(str)
	{
	}

	StringView& operator=(const char* str)
	{
		_length = strlen32(str);
		_data = str;
		return *this;
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

inline bool operator==(const StringView& a, const char* str)
{
	const u32 len = strlen32(str);
	return a._length == len
		&& strncmp(a._data, str, len) == 0
		;
}

inline bool operator==(const StringView& a, const StringView& b)
{
	return a._length == b._length
		&& strncmp(a._data, b._data, a._length) == 0
		;
}

inline bool operator<(const StringView& a, const StringView& b)
{
	const u32 len = max(a._length, b._length);
	return strncmp(a._data, b._data, len) < 0;
}

} // namespace crown
