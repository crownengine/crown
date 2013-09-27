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
#include "AtomicInt.h"

#define QUEUE_SIZE 1024 * 4

namespace crown
{

struct EventQueue
{
	//-----------------------------------------------------------------------------
	EventQueue() 
		: m_size(0), m_read(0)
	{
	}

	int32_t space()
	{
		return QUEUE_SIZE - m_size;
	}

	bool empty()
	{
		return m_read == m_size;
	}

	//-----------------------------------------------------------------------------
	void push_event(uint32_t event_type, void* event_data, size_t event_size)
	{
		int32_t next_size = 4 + 4 + event_size;

		if (next_size >= space())
		{
			Log::d("queue full");
			return;
		}

		int32_t cur_size = m_size;

		*(&m_buffer[cur_size]) = event_type;
		*(&m_buffer[cur_size] + 4) = event_size;
		memcpy(&m_buffer[cur_size] + 8, event_data, event_size);

		m_size += next_size;
	}

	uint32_t event_type()
	{
		if (empty())
		{
			return 0;
		}

		int32_t cur_read = m_read;
		uint32_t type = (uint32_t) *(&m_buffer[cur_read]);

		m_read += 4;

		return type;
	}

	//-----------------------------------------------------------------------------
	void get_next_event(void* data)
	{
		if (empty())
		{
			data = NULL;
		}

		int32_t cur_read = m_read;

		size_t event_size = (size_t) *(&m_buffer[cur_read]);
		memcpy(data, &m_buffer[cur_read] + 4, event_size);

		m_read += 4 + event_size;
	}

private:

	char m_buffer[QUEUE_SIZE];
	
	AtomicInt m_size;
	AtomicInt m_read;
};

} // namespace crown
