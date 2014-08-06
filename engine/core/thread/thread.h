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

#include "config.h"
#include "assert.h"
#include "types.h"
#include "semaphore.h"

#if CROWN_PLATFORM_POSIX
	#include <pthread.h>
#elif CROWN_PLATFORM_WINDOWS
	#include "win_headers.h"
	#include <process.h>
	#include <win_base.h>
#endif

namespace crown
{

typedef int32_t (*ThreadFunction)(void*);

struct Thread
{
	Thread()
#if CROWN_PLATFORM_POSIX
		: m_handle(0)
#elif CROWN_PLATFORM_WINDOWS
		: m_handle(INVALID_HANDLE_VALUE)
#endif
		, m_function(NULL)
		, m_data(NULL)
		, m_stack_size(0)
		, m_is_running(false)
	{
	}

	~Thread()
	{
		if (m_is_running)
			stop();
	}

	void start(ThreadFunction func, void* data = NULL, size_t stack_size = 0)
	{
		CE_ASSERT(!m_is_running, "Thread is already running");
		CE_ASSERT(func != NULL, "Function must be != NULL");
		m_function = func;
		m_data = data;
		m_stack_size = stack_size;

#if CROWN_PLATFORM_POSIX
		pthread_attr_t attr;
		int result = pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
		CE_ASSERT(result == 0, "pthread_attr_init: errno = %d", result);

		if (m_stack_size != 0)
		{
			result = pthread_attr_setstacksize(&attr, m_stack_size);
			CE_ASSERT(result == 0, "pthread_attr_setstacksize: errno = %d", result);
		}

		result = pthread_create(&m_handle, &attr, thread_proc, this);
		CE_ASSERT(result == 0, "pthread_create: errno = %d", result);

		// Free attr memory
		result = pthread_attr_destroy(&attr);
		CE_ASSERT(result == 0, "pthread_attr_destroy: errno = %d", result);
		CE_UNUSED(result);
#elif CROWN_PLATFORM_WINDOWS
		m_handle = CreateThread(NULL, stack_size, Thread::thread_proc, this, 0, NULL);
		CE_ASSERT(m_handle != NULL, "Failed to create the thread '%s'", m_name);
#endif

		m_is_running = true;
		m_sem.wait();
	}

	void stop()
	{
		CE_ASSERT(m_is_running, "Thread is not running");

#if CROWN_PLATFORM_POSIX
		int result = pthread_join(m_handle, NULL);
		CE_ASSERT(result == 0, "pthread_join: errno = %d", result);
		CE_UNUSED(result);
		m_handle = 0;
#elif CROWN_PLATFORM_WINDOWS
		WaitForSingleObject(m_handle, INFINITE);
		GetExitCodeThread(m_handle, &m_exit_code);
		CloseHandle(m_handle);
		m_handle = INVALID_HANDLE_VALUE;
#endif

		m_is_running = false;
	}

	bool is_running()
	{
		return m_is_running;
	}

private:

	int32_t run()
	{
		m_sem.post();
		return m_function(m_data);
	}

#if CROWN_PLATFORM_POSIX
	static void* thread_proc(void* arg)
	{
		static int32_t result = -1;
		result = ((Thread*)arg)->run();
		return (void*)&result;
	}
#elif CROWN_PLATFORM_WINDOWS
	static DWORD WINAPI OsThread::thread_proc(void* arg)
	{
		OsThread* thread = (OsThread*)arg;
		int32_t result = thread->run();
		return result;
	}
#endif

private:

#if CROWN_PLATFORM_POSIX
	pthread_t m_handle;
#elif CROWN_PLATFORM_WINDOWS
	HANDLE m_handle;
#endif

	ThreadFunction m_function;
	void* m_data;
	Semaphore m_sem;
	size_t m_stack_size;
	bool m_is_running;

private:

	// Disable copying
	Thread(const Thread&);
	Thread& operator=(const Thread&);
};

} // namespace crown
