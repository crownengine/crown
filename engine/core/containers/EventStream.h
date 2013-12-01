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

#include "Assert.h"
#include "Allocator.h"
#include "List.h"

namespace crown
{

typedef List<char> EventStream;

/// Functions for operating on a List<char> as a stream of events of the form:
/// [event_header_0][event_data_0][event_header_1][event_data_1] ...
namespace event_stream
{

struct Header
{
	uint32_t type;
	uint32_t size;
};

/// Appends the @a event of the given @a type and @a size to the stream @a s. 
inline void write(EventStream& s, uint32_t type, uint32_t size, const void* event)
{
	Header header;
	header.type = type;
	header.size = size;

	s.push((char*) &header, sizeof(Header));
	s.push((char*) event, (size_t) header.size);
}

/// Appends the @a event of the given @a type to the stream @a s
template <typename T>
inline void write(EventStream& s, uint32_t type, const T& event)
{
	event_stream::write(s, type, sizeof(T), &event);
}

} // namespace event_stream
} // namespace crown
