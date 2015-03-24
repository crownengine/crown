/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "assert.h"
#include "memory.h"
#include "string_utils.h"
#include "array.h"
#include "string_utils.h"
#include <algorithm>
#include <cstring>

namespace crown
{

///
class DynamicString
{
public:

	DynamicString(Allocator& allocator = default_allocator());
	DynamicString(const char* s, Allocator& allocator = default_allocator());

	~DynamicString();

	DynamicString& operator+=(const DynamicString& s);
	DynamicString& operator+=(const char* s);
	DynamicString& operator+=(const char c);
	///
	DynamicString& operator=(const DynamicString& s);
	DynamicString& operator=(const char* s);
	DynamicString& operator=(const char c);

	bool operator<(const DynamicString& s) const;
	bool operator==(const DynamicString& s) const;
	bool operator==(const char* s) const;

	// Returns the length of the string.
	uint32_t length() const;

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

	///
	const char* c_str() const;

private:

	Array<char> _data;
};

inline DynamicString::DynamicString(Allocator& allocator)
	: _data(allocator)
{
	array::push_back(_data, '\0');
}

inline DynamicString::DynamicString(const char* s, Allocator& allocator)
	: _data(allocator)
{
	if (s != NULL)
	{
		array::push(_data, s, strlen(s));
	}
	array::push_back(_data, '\0');
}

inline DynamicString::~DynamicString()
{
}

inline DynamicString& DynamicString::operator+=(const DynamicString& s)
{
	return *this += s.c_str();
}

inline DynamicString& DynamicString::operator+=(const char* s)
{
	CE_ASSERT_NOT_NULL(s);
	array::pop_back(_data);
	array::push(_data, s, strlen(s));
	array::push_back(_data, '\0');
	return *this;
}

inline DynamicString& DynamicString::operator+=(const char c)
{
	array::pop_back(_data);
	array::push_back(_data, c);
	array::push_back(_data, '\0');
	return *this;
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
	array::push(_data, s, strlen(s));
	array::push_back(_data, '\0');
	return *this;
}

inline DynamicString& DynamicString::operator=(const char c)
{
	array::clear(_data);
	array::push_back(_data, c);

	return *this;
}

inline bool DynamicString::operator<(const DynamicString& s) const
{
	return strcmp(c_str(), s.c_str()) < 0;
}

inline bool DynamicString::operator==(const DynamicString& s) const
{
	return strcmp(c_str(), s.c_str()) == 0;
}

inline bool DynamicString::operator==(const char* s) const
{
	CE_ASSERT_NOT_NULL(s);

	return strcmp(c_str(), s) == 0;
}

inline uint32_t DynamicString::length() const
{
	return strlen(this->c_str());
}

inline void DynamicString::strip_leading(const char* s)
{
	CE_ASSERT_NOT_NULL(s);
	CE_ASSERT(starts_with(s), "String does not start with %s", s);

	const size_t my_len = strlen(c_str());
	const size_t s_len = strlen(s);

	memmove(array::begin(_data), array::begin(_data) + s_len, (my_len - s_len));
	array::resize(_data, my_len - s_len);
	array::push_back(_data, '\0');
}

inline void DynamicString::strip_trailing(const char* s)
{
	CE_ASSERT_NOT_NULL(s);
	CE_ASSERT(ends_with(s), "String does not end with %s", s);

	const size_t my_len = strlen(c_str());
	const size_t s_len = strlen(s);
	array::resize(_data, my_len - s_len);
	array::push_back(_data, '\0');
}

inline bool DynamicString::starts_with(const char* s) const
{
	CE_ASSERT_NOT_NULL(s);
	return strncmp(c_str(), s, strlen(s)) == 0;
}

inline bool DynamicString::ends_with(const char* s) const
{
	CE_ASSERT_NOT_NULL(s);

	const size_t my_len = strlen(c_str());
	const size_t s_len = strlen(s);

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
	return array::begin(_data);
}

} // namespace crown
