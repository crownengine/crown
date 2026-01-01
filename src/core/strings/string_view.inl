/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/murmur.h"
#include "core/strings/string.inl"
#include "core/strings/string_view.h"

namespace crown
{
inline StringView::StringView()
	: _length(0)
	, _data(NULL)
{
}

inline StringView::StringView(const char *str)
	: _length(strlen32(str))
	, _data(str)
{
}

inline StringView::StringView(const char *str, u32 len)
	: _length(len)
	, _data(str)
{
}

inline StringView &StringView::operator=(const char *str)
{
	_length = strlen32(str);
	_data = str;
	return *this;
}

inline u32 StringView::length() const
{
	return _length;
}

inline const char *StringView::data() const
{
	return _data;
}

inline bool operator==(const StringView &a, const char *str)
{
	const u32 len = strlen32(str);
	return a._length == len
		&& strncmp(a._data, str, len) == 0
		;
}

inline bool operator==(const StringView &a, const StringView &b)
{
	return a._length == b._length
		&& strncmp(a._data, b._data, a._length) == 0
		;
}

inline bool operator!=(const StringView &a, const StringView &b)
{
	return a._length != b._length
		|| strncmp(a._data, b._data, a._length) != 0
		;
}

inline bool operator<(const StringView &a, const StringView &b)
{
	const u32 len = min(a._length, b._length);
	const int cmp = strncmp(a._data, b._data, len);
	return cmp < 0 || (cmp == 0 && a._length < b._length);
}

template<>
struct hash<StringView>
{
	u32 operator()(const StringView &val) const
	{
		return (u32)murmur64(val._data, val._length, 0);
	}
};

} // namespace crown
