/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <stdio.h>
#include "container_types.h"
#include "string_utils.h"
#include "os.h"

namespace crown
{

/// Stream of characters.
typedef Array<char> StringStream;

/// Functions to operate on StringStream.
namespace string_stream
{
	/// Appends @a val to the stream @a s using appropriate formatting.
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
		array::push(s, string, string::strlen(string));
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
