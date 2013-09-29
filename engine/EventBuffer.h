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

#include "OsTypes.h"

#define EVENT_BUFFER_MAX_SIZE 1024 * 4

namespace crown
{

struct EventBuffer
{
	char m_buffer[EVENT_BUFFER_MAX_SIZE];
	
	size_t m_size;
	size_t m_read;

	//-----------------------------------------------------------------------------
	EventBuffer() 
		: m_size(0), m_read(0)
	{
	}

	//-----------------------------------------------------------------------------
	void push_event(uint32_t event_type, void* event_data, size_t event_size)
	{
		if (m_size + sizeof(event_type) + sizeof(event_size) + event_size > EVENT_BUFFER_MAX_SIZE)
		{
			flush();
		}

		char* cur = m_buffer + m_size;

		*(uint32_t*) cur = event_type;
		*(size_t*) (cur + sizeof(event_type)) = event_size;
		memcpy(cur + sizeof(event_type) + sizeof(event_size), event_data, event_size);

		m_size += sizeof(event_type) + sizeof(event_size) + event_size;
	}

	//-----------------------------------------------------------------------------
	void* get_next_event(uint32_t& event_type, size_t& event_size)
	{
		if (m_read < m_size)
		{
			char* cur = m_buffer + m_read;

			// Saves type
			event_type = *(uint32_t*) cur;
			// Saves size
			event_size = *(size_t*)(cur + sizeof(uint32_t));

			// Set read to next event
			m_read += sizeof(size_t) + sizeof(uint32_t) + event_size;

			return cur + sizeof(size_t) + sizeof(uint32_t);
		}

		m_read = 0;

		return NULL;
	}

	//-----------------------------------------------------------------------------
	void clear()
	{
		m_size = 0;
		m_read = 0;
	}

	//-----------------------------------------------------------------------------
	void flush()
	{
		m_size = 0;
		m_read = 0;
	}
};

} // namespace crown