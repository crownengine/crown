/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "error.h"
#include "memory.h"
#include "array.h"
#include "string_id.h"
#include "string_utils.h"
#include "fixed_string.h"
#include <string.h> // memmove

namespace crown
{
/// Dynamic array of characters.
///
/// @ingroup String
struct DynamicString
{
	ALLOCATOR_AWARE;

	Array<char> _data;

	DynamicString(Allocator& a);
	DynamicString(const char* s, Allocator& a = default_allocator());

	DynamicString& operator=(const DynamicString& s);
	DynamicString& operator=(const char* s);
	DynamicString& operator=(const char c);
	DynamicString& operator=(const FixedString& s);

	/// Sets the string to @a s.
	void set(const char* s, u32 len);

	/// Reserves space for at least @n characters.
	void reserve(u32 n);

	/// Returns the length of the string.
	u32 length() const;

	/// Returns whether the string is empty.
	bool empty() const;

	/// Removes leading white-space characters from the string.
	void ltrim();

	/// Removes trailing white-space characters from the string.
	void rtrim();

	/// Removes leading and trailing white-space characters from the string.
	void trim();

	/// Removes the leading string @a s.
	/// @note
	/// The string must start with @a s.
	void strip_leading(const char* s);

	/// Removes the trailing string @a s.
	/// @note
	/// The string must end with @a s.
	void strip_trailing(const char* s);

	/// Returns whether the string starts with the given @a s string.
	bool starts_with(const char* s) const;

	/// Returns wheterh the string ends with the given @a s string.
	bool ends_with(const char* s) const;

	/// Returns the StringId32 of the string.
	StringId32 to_string_id() const;

	/// Returns the string as a NULL-terminated string.
	const char* c_str() const;
};

inline DynamicString::DynamicString(Allocator& a)
	: _data(a)
{
}

inline DynamicString::DynamicString(const char* s, Allocator& a)
	: _data(a)
{
	CE_ASSERT_NOT_NULL(s);
	array::push(_data, s, strlen32(s));
}

inline void DynamicString::set(const char* s, u32 len)
{
	array::resize(_data, len);
	strncpy(array::begin(_data), s, len);
}

inline DynamicString& operator+=(DynamicString& a, const DynamicString& b)
{
	array::push(a._data, array::begin(b._data), array::size(b._data));
	return a;
}

inline DynamicString& operator+=(DynamicString& a, const char* s)
{
	CE_ASSERT_NOT_NULL(s);
	array::push(a._data, s, strlen32(s));
	return a;
}

inline DynamicString& operator+=(DynamicString& a, const char c)
{
	array::push_back(a._data, c);
	return a;
}

inline DynamicString& operator+=(DynamicString& a, const FixedString& s)
{
	array::push(a._data, s.data(), s.length());
	return a;
}

inline DynamicString& DynamicString::operator=(const DynamicString& s)
{
	_data = s._data;
	return *this;
}

inline DynamicString& DynamicString::operator=(const char* s)
{
	CE_ASSERT_NOT_NULL(s);
	array::clear(_data);
	array::push(_data, s, strlen32(s));
	return *this;
}

inline DynamicString& DynamicString::operator=(const char c)
{
	array::clear(_data);
	array::push_back(_data, c);
	return *this;
}

inline DynamicString& DynamicString::operator=(const FixedString& s)
{
	array::clear(_data);
	array::push(_data, s.data(), s.length());
	return *this;
}

inline bool operator<(const DynamicString& a, const DynamicString& b)
{
	return strcmp(a.c_str(), b.c_str()) < 0;
}

inline bool operator==(const DynamicString& a, const DynamicString& b)
{
	return strcmp(a.c_str(), b.c_str()) == 0;
}

inline bool operator==(const DynamicString& a, const char* b)
{
	CE_ASSERT_NOT_NULL(b);
	return strcmp(a.c_str(), b) == 0;
}

inline void DynamicString::reserve(u32 n)
{
	array::reserve(_data, n);
}

inline u32 DynamicString::length() const
{
	return strlen32(this->c_str());
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

inline void DynamicString::strip_leading(const char* s)
{
	CE_ASSERT_NOT_NULL(s);
	CE_ASSERT(starts_with(s), "String does not start with %s", s);

	const u32 my_len = strlen32(c_str());
	const u32 s_len = strlen32(s);

	memmove(array::begin(_data), array::begin(_data) + s_len, (my_len - s_len));
	array::resize(_data, my_len - s_len);
}

inline void DynamicString::strip_trailing(const char* s)
{
	CE_ASSERT_NOT_NULL(s);
	CE_ASSERT(ends_with(s), "String does not end with %s", s);

	const u32 my_len = strlen32(c_str());
	const u32 s_len = strlen32(s);
	array::resize(_data, my_len - s_len);
}

inline bool DynamicString::starts_with(const char* s) const
{
	CE_ASSERT_NOT_NULL(s);
	return strncmp(c_str(), s, strlen32(s)) == 0;
}

inline bool DynamicString::ends_with(const char* s) const
{
	CE_ASSERT_NOT_NULL(s);

	const u32 my_len = strlen32(c_str());
	const u32 s_len = strlen32(s);

	if (my_len >= s_len)
	{
		return strncmp(array::begin(_data) + (my_len - s_len), s, s_len) == 0;
	}

	return false;
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

} // namespace crown
