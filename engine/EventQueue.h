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
#include <stdio.h>
#include "Mutex.h"
#include "Queue.h"
#include "Log.h"

namespace crown
{

/// Represents an abstract queue of events
class EventQueue
{
public:

	//-----------------------------------------------------------------------------
	EventQueue(Allocator& a)
		: m_queue(a)
	{
	}

	//-----------------------------------------------------------------------------
	void push_event(void* data)
	{
		m_mutex.lock();
		m_queue.push_back(data);
		m_mutex.unlock();
	}

	//-----------------------------------------------------------------------------
	void* get_event()
	{
		m_mutex.lock();

		void* data;
		if (m_queue.size() > 0)
		{
			data = m_queue.front();
			m_queue.pop_front();
		}
		else
		{
			data = NULL;
		}

		m_mutex.unlock();

		return data;
	}

public:

	Mutex m_mutex;
	Queue<void*> m_queue;
};

} // namespace crown