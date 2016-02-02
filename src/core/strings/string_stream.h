/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "array.h"
#include "string_utils.h"

namespace crown
{
/// Stream of characters.
///
/// @ingroup Containers
typedef Array<char> StringStream;

/// Functions to operate on StringStream.
namespace string_stream
{
	/// Appends @a val to the stream @a s using appropriate formatting.
	StringStream& operator<<(StringStream& s, char ch);
	StringStream& operator<<(StringStream& s, s16 val);
	StringStream& operator<<(StringStream& s, u16 val);
	StringStream& operator<<(StringStream& s, s32 val);
	StringStream& operator<<(StringStream& s, u32 val);
	StringStream& operator<<(StringStream& s, s64 val);
	StringStream& operator<<(StringStream& s, u64 val);
	StringStream& operator<<(StringStream& s, f32 val);
	StringStream& operator<<(StringStream& s, f64 val);

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

	inline StringStream& operator<<(StringStream& s, s16 val)
	{
		return stream_printf(s, "%hd", val);
	}

	inline StringStream& operator<<(StringStream& s, u16 val)
	{
		return stream_printf(s, "%hu", val);
	}

	inline StringStream& operator<<(StringStream& s, s32 val)
	{
		return stream_printf(s, "%d", val);
	}

	inline StringStream& operator<<(StringStream& s, u32 val)
	{
		return stream_printf(s, "%u", val);
	}

	inline StringStream& operator<<(StringStream& s, s64 val)
	{
		return stream_printf(s, "%lld", val);
	}

	inline StringStream& operator<<(StringStream& s, u64 val)
	{
		return stream_printf(s, "%llu", val);
	}

	inline StringStream& operator<<(StringStream& s, f32 val)
	{
		return stream_printf(s, "%g", val);
	}

	inline StringStream& operator<<(StringStream& s, f64 val)
	{
		return stream_printf(s, "%g", val);
	}

	inline StringStream& operator<<(StringStream& s, const char* str)
	{
		array::push(s, str, strlen32(str));
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
		snprintf(buf, sizeof(buf), format, val);
		return s << buf;
	}
} // namespace string_stream
} // namespace crown
