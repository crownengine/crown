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

#include "Config.h"
#include "Assert.h"
#include "Mutex.h"

#if CROWN_PLATFORM_POSIX
	#include <pthread.h>
#elif CROWN_PLATFORM_WINDOWS
	#include "WinHeaders.h"
	#include <limits.h>
#endif

namespace crown
{

struct Semaphore
{
	Semaphore()
		#if CROWN_PLATFORM_POSIX
		: m_count(0)
		#elif CROWN_PLATFORM_WINDOWS
		: m_handle(INVALID_HANDLE_VALUE)
		#endif
	{
		#if CROWN_PLATFORM_POSIX
		int result = pthread_cond_init(&m_cond, NULL);
		CE_ASSERT(result == 0, "pthread_cond_init: errno = %d", result);
		CE_UNUSED(result);
		#elif CROWN_PLATFORM_WINDOWS
		m_handle = CreateSemaphore(NULL, 0, LONG_MAX, NULL);
		CE_ASSERT(m_handle != NULL, "Unable to create semaphore!");
		CE_UNUSED(m_handle);
		#endif
	}

	~Semaphore()
	{
		#if CROWN_PLATFORM_POSIX
		int result = pthread_cond_destroy(&m_cond);
		CE_ASSERT(result == 0, "pthread_cond_destroy: errno = %d", result);
		CE_UNUSED(result);
		#elif CROWN_PLATFORM_WINDOWS
		CloseHandle(m_handle);
		#endif
	}

	void post(uint32_t count = 1)
	{
		#if CROWN_PLATFORM_POSIX
		m_mutex.lock();
		for (uint32_t i = 0; i < count; i++)
		{
			int result = pthread_cond_signal(&m_cond);
			CE_ASSERT(result == 0, "pthread_cond_signal: errno = %d", result);
			CE_UNUSED(result);
		}

		m_count += count;
		m_mutex.unlock();
		#elif CROWN_PLATFORM_WINDOWS
		ReleaseSemaphore(m_handle, count, NULL);
		#endif
	}

	void wait()
	{
		#if CROWN_PLATFORM_POSIX
		m_mutex.lock();
		while (m_count <= 0)
		{
			int result = pthread_cond_wait(&m_cond, &(m_mutex.m_mutex));
			CE_ASSERT(result == 0, "pthread_cond_wait: errno = %d", result);
			CE_UNUSED(result);	
		}

		m_count--;
		m_mutex.unlock();
		#elif CROWN_PLATFORM_WINDOWS
		DWORD milliseconds = (0 > msecs) ? INFINITE : msecs;
		DWORD result = WaitForSingleObject(m_handle, milliseconds);
		CE_ASSERT(result == WAIT_OBJECT_0, "Semaphore can not signal!");
		CE_UNUSED(result);
		#endif
	}

private:

	#if CROWN_PLATFORM_POSIX
	Mutex m_mutex;
	pthread_cond_t m_cond;
	int32_t m_count;
	#elif CROWN_PLATFORM_WINDOWS
	HANDLE m_handle;
	#endif

private:

	// Disable copying
	Semaphore(const Semaphore& s);
	Semaphore& operator=(const Semaphore& s);
};

} // namespace crown
