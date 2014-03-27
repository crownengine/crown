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
#include "ContainerTypes.h"
#include "StringUtils.h"
#include "OS.h"

namespace crown
{

class StringStream
{
public:

						StringStream(Allocator& allocator);

	void				clear();

	StringStream&		operator<<(int16_t val);
	StringStream&		operator<<(uint16_t val);
	StringStream&		operator<<(int32_t val);
	StringStream&		operator<<(uint32_t val);
	StringStream&		operator<<(int64_t val);
	StringStream&		operator<<(uint64_t val);
	StringStream&		operator<<(float val);
	StringStream&		operator<<(double val);

	StringStream&		operator<<(const char* s);

	const char*			c_str();

private:

	template <typename T>
	StringStream&		stream_printf(const char* format, T& val);

private:

	Array<char>			m_string;
};

//-----------------------------------------------------------------------------
inline StringStream::StringStream(Allocator& allocator)
	: m_string(allocator)
{
}

//-----------------------------------------------------------------------------
inline void StringStream::clear()
{
	array::clear(m_string);
}

//-----------------------------------------------------------------------------
inline StringStream& StringStream::operator<<(int16_t val)
{
	return stream_printf("%hd", val);
}

//-----------------------------------------------------------------------------
inline StringStream& StringStream::operator<<(uint16_t val)
{
	return stream_printf("%hu", val);
}

//-----------------------------------------------------------------------------
inline StringStream& StringStream::operator<<(int32_t val)
{
	return stream_printf("%d", val);
}

//-----------------------------------------------------------------------------
inline StringStream& StringStream::operator<<(uint32_t val)
{
	return stream_printf("%u", val);
}

//-----------------------------------------------------------------------------
inline StringStream& StringStream::operator<<(int64_t val)
{
	return stream_printf("%lld", val);
}

//-----------------------------------------------------------------------------
inline StringStream& StringStream::operator<<(uint64_t val)
{
	return stream_printf("%llu", val);
}

//-----------------------------------------------------------------------------
inline StringStream& StringStream::operator<<(float val)
{
	return stream_printf("%g", val);
}

//-----------------------------------------------------------------------------
inline StringStream& StringStream::operator<<(double val)
{
	return stream_printf("%g", val);
}

//-----------------------------------------------------------------------------
inline StringStream& StringStream::operator<<(const char* s)
{
	array::push(m_string, s, string::strlen(s));

	return *this;
}

//-----------------------------------------------------------------------------
inline const char* StringStream::c_str()
{
	array::push_back(m_string, '\0');
	array::pop_back(m_string);

	return array::begin(m_string);
}

//-----------------------------------------------------------------------------
template <typename T>
inline StringStream& StringStream::stream_printf(const char* format, T& val)
{
	char buf[32];
	snprintf(buf, 32, format, val);

	return *this << buf;
}

} // namespace crown
