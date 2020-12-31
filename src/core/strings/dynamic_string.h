/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/containers/types.h"
#include "core/strings/string_id.h"
#include "core/strings/string_view.h"
#include "core/strings/types.h"

namespace crown
{
/// Dynamic array of characters.
///
/// @ingroup String
struct DynamicString
{
	ALLOCATOR_AWARE;

	Array<char> _data;

	///
	DynamicString(Allocator& a);

	///
	~DynamicString();

	///
	DynamicString& operator=(const char* str);

	///
	DynamicString& operator=(const char c);

	///
	DynamicString& operator=(const StringView& fs);

	/// Sets the string to @a str.
	void set(const char* str, u32 len);

	///
	void from_guid(const Guid& guid);

	///
	void from_string_id(const StringId32& id);

	///
	void from_string_id(const StringId64& id);

	/// Reserves space for at least @a n characters.
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

	/// Returns whether the string starts with @a prefix.
	bool has_prefix(const char* prefix) const;

	/// Returns wheterh the string ends with @a suffix.
	bool has_suffix(const char* suffix) const;

	/// Returns the StringId32 of the string.
	StringId32 to_string_id() const;

	/// Returns the string as a NULL-terminated string.
	const char* c_str() const;
};

} // namespace crown
