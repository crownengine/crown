/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/containers/array.inl"
#include "core/error/error.inl"
#include "core/strings/dynamic_string.h"
#include "core/strings/string.inl"
#include <string.h> // memmove

namespace crown
{
inline DynamicString::DynamicString(Allocator& a)
	: _data(a)
{
}

inline DynamicString::~DynamicString()
{
}

inline void DynamicString::set(const char* str, u32 len)
{
	array::resize(_data, len);
	memcpy(array::begin(_data), str, len);
}

inline DynamicString& DynamicString::operator=(const char* str)
{
	CE_ENSURE(NULL != str);
	array::clear(_data);
	array::push(_data, str, strlen32(str));
	return *this;
}

inline DynamicString& DynamicString::operator=(const char c)
{
	array::clear(_data);
	array::push_back(_data, c);
	return *this;
}

inline DynamicString& DynamicString::operator=(const StringView& fs)
{
	array::clear(_data);
	array::push(_data, fs.data(), fs.length());
	return *this;
}

inline DynamicString& operator+=(DynamicString& a, const DynamicString& b)
{
	array::push(a._data, array::begin(b._data), array::size(b._data));
	return a;
}

inline DynamicString& operator+=(DynamicString& a, const char* str)
{
	CE_ENSURE(NULL != str);
	array::push(a._data, str, strlen32(str));
	return a;
}

inline DynamicString& operator+=(DynamicString& a, const char c)
{
	array::push_back(a._data, c);
	return a;
}

inline DynamicString& operator+=(DynamicString& a, const StringView& fs)
{
	array::push(a._data, fs.data(), fs.length());
	return a;
}

inline bool operator<(const DynamicString& a, const DynamicString& b)
{
	return strcmp(a.c_str(), b.c_str()) < 0;
}

inline bool operator==(const DynamicString& a, const DynamicString& b)
{
	return array::size(a._data) == array::size(b._data)
		&& strcmp(a.c_str(), b.c_str()) == 0
		;
}

inline bool operator!=(const DynamicString& a, const DynamicString& b)
{
	return array::size(a._data) != array::size(b._data)
		|| strcmp(a.c_str(), b.c_str()) != 0
		;
}

inline bool operator==(const DynamicString& a, const char* str)
{
	CE_ENSURE(NULL != str);
	return strcmp(a.c_str(), str) == 0;
}

inline bool operator!=(const DynamicString& a, const char* str)
{
	CE_ENSURE(NULL != str);
	return strcmp(a.c_str(), str) != 0;
}

inline void DynamicString::reserve(u32 n)
{
	array::reserve(_data, n);
}

inline u32 DynamicString::length() const
{
	return strlen32(c_str());
}

inline bool DynamicString::empty() const
{
	return length() == 0;
}

inline void DynamicString::ltrim()
{
	const char* str = c_str();
	const char* end = skip_spaces(str);

	const u32 len = strlen32(end);

	memmove(array::begin(_data), end, len);
	array::resize(_data, len);
}

inline void DynamicString::rtrim()
{
	char* str = (char*)c_str();
	char* end = str + strlen32(str) - 1;

	while (end > str && isspace(*end)) --end;

	*(end + 1) = '\0';
}

inline void DynamicString::trim()
{
	ltrim();
	rtrim();
}

inline bool DynamicString::has_prefix(const char* prefix) const
{
	return str_has_prefix(c_str(), prefix);
}

inline bool DynamicString::has_suffix(const char* suffix) const
{
	return str_has_suffix(c_str(), suffix);
}

inline StringId32 DynamicString::to_string_id() const
{
	return StringId32(c_str());
}

inline const char* DynamicString::c_str() const
{
	Array<char>& data = const_cast<Array<char>& >(_data);
	array::push_back(data, '\0');
	array::pop_back(data);
	return array::begin(_data);
}

template <>
struct hash<DynamicString>
{
	u32 operator()(const DynamicString& val) const
	{
		return val.to_string_id()._id;
	}
};

} // namespace crown
