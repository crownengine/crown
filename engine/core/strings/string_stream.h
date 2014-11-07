/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "array.h"
#include "string_utils.h"
#include <stdio.h>

namespace crown
{

/// Stream of characters.
typedef Array<char> StringStream;

/// Functions to operate on StringStream.
namespace string_stream
{
	/// Appends @a val to the stream @a s using appropriate formatting.
	StringStream& operator<<(StringStream& s, char ch);
	StringStream& operator<<(StringStream& s, int16_t val);
	StringStream& operator<<(StringStream& s, uint16_t val);
	StringStream& operator<<(StringStream& s, int32_t val);
	StringStream& operator<<(StringStream& s, uint32_t val);
	StringStream& operator<<(StringStream& s, int64_t val);
	StringStream& operator<<(StringStream& s, uint64_t val);
	StringStream& operator<<(StringStream& s, float val);
	StringStream& operator<<(StringStream& s, double val);

	/// Appends the string @a string to the stream @a s.
	StringStream& operator<<(StringStream& s, const char* string);

	/// Returns the stream as a NULL-terminated string.
	const char* c_str(StringStream& s);

	template <typename T> StringStream& stream_printf(StringStream& s, const char* format, T& val);

} // namespace string_stream

namespace string_stream
{
	inline StringStream& operator<<(StringStream& s, char ch)
	{
		array::push_back(s, ch);
		return s;
	}

	inline StringStream& operator<<(StringStream& s, int16_t val)
	{
		return stream_printf(s, "%hd", val);
	}

	inline StringStream& operator<<(StringStream& s, uint16_t val)
	{
		return stream_printf(s, "%hu", val);
	}

	inline StringStream& operator<<(StringStream& s, int32_t val)
	{
		return stream_printf(s, "%d", val);
	}

	inline StringStream& operator<<(StringStream& s, uint32_t val)
	{
		return stream_printf(s, "%u", val);
	}

	inline StringStream& operator<<(StringStream& s, int64_t val)
	{
		return stream_printf(s, "%lld", val);
	}

	inline StringStream& operator<<(StringStream& s, uint64_t val)
	{
		return stream_printf(s, "%llu", val);
	}

	inline StringStream& operator<<(StringStream& s, float val)
	{
		return stream_printf(s, "%g", val);
	}

	inline StringStream& operator<<(StringStream& s, double val)
	{
		return stream_printf(s, "%g", val);
	}

	inline StringStream& operator<<(StringStream& s, const char* string)
	{
		array::push(s, string, strlen(string));
		return s;
	}

	inline const char* c_str(StringStream& s)
	{
		array::push_back(s, '\0');
		array::pop_back(s);
		return array::begin(s);
	}

	template <typename T>
	inline StringStream& stream_printf(StringStream& s, const char* format, T& val)
	{
		char buf[32];
		snprintf(buf, 32, format, val);
		return s << buf;
	}
} // namespace string_stream
} // namespace crown
