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

#include "Assert.h"
#include "Types.h"
#include "Semaphore.h"
#include "Log.h"

namespace crown
{

typedef int32_t (*ThreadFunction)(void*);

class Thread
{
public:

						Thread(const char* name);
						~Thread();

	void				start(ThreadFunction func, void* data = NULL, size_t stack_size = 0);
	void				stop();

	bool				is_running();

private:

	int32_t				run();

	static void* 		thread_proc(void* arg);

private:

	const char* 		m_name;

	pthread_t			m_handle;
	ThreadFunction 	m_function;
	void*				m_data;
	Semaphore			m_sem;
	size_t 				m_stack_size;

	bool				m_is_running :1;
};

//-----------------------------------------------------------------------------
inline Thread::Thread(const char* name) :
	m_name(name),
	m_handle(0),
	m_function(NULL),
	m_data(NULL),
	m_stack_size(0),
	m_is_running(false)
{
	memset(&m_handle, 0, sizeof(pthread_t));
}

//-----------------------------------------------------------------------------
inline Thread::~Thread()
{
}

//-----------------------------------------------------------------------------
inline void Thread::start(ThreadFunction func, void* data, size_t stack_size)
{
	CE_ASSERT(!m_is_running, "Thread is already running");
	CE_ASSERT(func != NULL, "Function must be != NULL");

	m_function = func;
	m_data = data;
	m_stack_size = stack_size;

	pthread_attr_t attr;
	int32_t result = pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);


	CE_ASSERT(result == 0, "pthread_attr_init failed. errno: %d", result);

	if (m_stack_size != 0)
	{
		result = pthread_attr_setstacksize(&attr, m_stack_size);
		CE_ASSERT(result == 0, "pthread_attr_setstacksize failed. errno: %d", result);
	}

	result = pthread_create(&m_handle, &attr, thread_proc, this);
	CE_ASSERT(result == 0, "pthread_create failed. errno: %d", result);

	// Free attr memory
	result = pthread_attr_destroy(&attr);
	CE_ASSERT(result == 0, "pthread_attr_destroy failed. errno: %d", result);

	m_is_running = true;

	m_sem.wait();
}

//-----------------------------------------------------------------------------
inline void Thread::stop()
{
	CE_ASSERT(m_is_running, "Thread is not running");
	
	m_is_running = false;

	int32_t result = pthread_join(m_handle, NULL);

	CE_ASSERT(result == 0, "Thread join failed. errno: %d", result);

	m_handle = 0;
}

//-----------------------------------------------------------------------------
inline bool Thread::is_running()
{
	return m_is_running;
}

//-----------------------------------------------------------------------------
inline int32_t Thread::run()
{
	m_sem.post();
	
	return m_function(m_data);
}

//-----------------------------------------------------------------------------
inline void* Thread::thread_proc(void* arg)
{
	Thread* thread = (Thread*)arg;

	int32_t result = thread->run();

	CE_ASSERT(result == 0, "Function failed");

	thread->m_is_running = false;

	return NULL;
}


} // namespace crown