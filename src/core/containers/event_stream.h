/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "error.h"
#include "array.h"

namespace crown
{
struct EventHeader
{
	u32 type;
	u32 size;
};

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
	/// Appends the @a event of the given @a type and @a size to the stream @a s.
	inline void write(EventStream& s, u32 type, u32 size, const void* event)
	{
		EventHeader eh;
		eh.type = type;
		eh.size = size;

		array::push(s, (char*)&eh, sizeof(eh));
		array::push(s, (char*)event, size);
	}

	/// Appends the @a event of the given @a type to the stream @a s
	template <typename T>
	inline void write(EventStream& s, u32 type, const T& event)
	{
		event_stream::write(s, type, sizeof(T), &event);
	}
} // namespace event_stream

} // namespace crown
