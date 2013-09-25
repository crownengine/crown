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
#include "Mutex.h"
#include "List.h"

namespace crown
{

class EventQueue
{
public:

	//-----------------------------------------------------------------------------
	EventQueue(Allocator& a)
		: m_read(0), m_array(a)
	{
	}

	//-----------------------------------------------------------------------------
	void push_event(uint32_t type, char* data, uint32_t size)
	{
		m_mutex.lock();
		m_array.push((char*)&type, sizeof(uint32_t));
		m_array.push((char*)&size, sizeof(uint32_t));
		m_array.push(data, size);
		m_mutex.unlock();
	}

	//-----------------------------------------------------------------------------
	uint32_t event_type()
	{
		m_mutex.lock();
		uint32_t type;
		memcpy(&type, m_array.begin() + m_read, sizeof(uint32_t));
		m_mutex.unlock();

		return type;
	}

	//-----------------------------------------------------------------------------
	void event_data(char* event_data)
	{
		m_mutex.lock();
		uint32_t size;
		memcpy(&size, m_array.begin() + m_read, sizeof(uint32_t));
		memcpy(event_data, m_array.begin() + m_read + size, size);
		m_read += sizeof(uint32_t) + size;
		m_mutex.unlock();
	}

	//-----------------------------------------------------------------------------
	void skip()
	{
		m_mutex.lock();
		uint32_t size;
		memcpy(&size, m_array.begin() + m_read, sizeof(uint32_t));
		m_read += sizeof(uint32_t) + size;
		m_mutex.unlock();
	}

	//-----------------------------------------------------------------------------
	void clear()
	{
		m_mutex.lock();
		m_read = 0;
		m_array.clear();
		m_mutex.unlock();
	}

public:

	Mutex m_mutex;
	size_t m_read;
	List<char> m_array;
};

} // namespace crown