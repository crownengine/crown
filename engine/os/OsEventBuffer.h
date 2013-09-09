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

#include <cstring>

#include "Types.h"
#include "OS.h"
#include "Log.h"
#include "OsEvents.h"

#define MAX_OS_EVENT_BUFFER_SIZE 1024

namespace crown
{


///	__OsEventBuffer__ is a global buffer used for storing events.
///	Each subsystem can read its relative events and modifies its behaviour consequently.
///
/// [type #0][size #0][data #0] ... [type #n][size #n][data #n]
class OsEventBuffer
{

public:
	/// Constructor
				OsEventBuffer();

	/// Pushes an @a event_data of size @a event_size with type @a event_type 
	void		push_event(OsEventType event_type, void* event_data, size_t event_size);
	/// Pushes an entire @a event_buffer of size @a buffer_size
	void		push_event_buffer(char* event_buffer, size_t buffer_size);
	/// Retrieves the @a event_type and @a event_size of next os event
	void*		get_next_event(uint32_t& event_type, size_t& event_size);

	/// Clears entire os buffer
	void		clear();
	/// Flushes entire os buffer
	void		flush();

	/// Returns buffer's size
	size_t		size() const;
	/// Return buffer
	char*		buffer();

public:

	size_t		m_size;
	char		m_buffer[MAX_OS_EVENT_BUFFER_SIZE];
};

OsEventBuffer* os_event_buffer();

} // namespace crown