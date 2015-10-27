/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "error.h"
#include "array.h"

namespace crown
{

/// Array of generic event structs.
///
/// @ingroup Containers
typedef Array<char> EventStream;

/// Functions to manipulate EventStream.
///
/// The events are stored in the following form:
/// [event_header_0][event_data_0][event_header_1][event_data_1] ...
///
/// @ingroup Containers
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

		array::push(s, (char*)&header, sizeof(Header));
		array::push(s, (char*)event, size);
	}

	/// Appends the @a event of the given @a type to the stream @a s
	template <typename T>
	inline void write(EventStream& s, uint32_t type, const T& event)
	{
		event_stream::write(s, type, sizeof(T), &event);
	}
} // namespace event_stream
} // namespace crown
