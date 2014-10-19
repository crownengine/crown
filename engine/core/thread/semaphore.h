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
#include "mutex.h"

#if CROWN_PLATFORM_POSIX
	#include <pthread.h>
#elif CROWN_PLATFORM_WINDOWS
	#include "win_headers.h"
	#include <limits.h>
#endif

namespace crown
{

struct Semaphore
{
	Semaphore()
#if CROWN_PLATFORM_POSIX
		: _count(0)
#elif CROWN_PLATFORM_WINDOWS
		: _handle(INVALID_HANDLE_VALUE)
#endif
	{
#if CROWN_PLATFORM_POSIX
		int result = pthread_cond_init(&m_cond, NULL);
		CE_ASSERT(result == 0, "pthread_cond_init: errno = %d", result);
		CE_UNUSED(result);
#elif CROWN_PLATFORM_WINDOWS
		_handle = CreateSemaphore(NULL, 0, LONG_MAX, NULL);
		CE_ASSERT(_handle != NULL, "Unable to create semaphore!");
		CE_UNUSED(_handle);
#endif
	}

	~Semaphore()
	{
#if CROWN_PLATFORM_POSIX
		int result = pthread_cond_destroy(&m_cond);
		CE_ASSERT(result == 0, "pthread_cond_destroy: errno = %d", result);
		CE_UNUSED(result);
#elif CROWN_PLATFORM_WINDOWS
		CloseHandle(_handle);
#endif
	}

	void post(uint32_t count = 1)
	{
#if CROWN_PLATFORM_POSIX
		_mutex.lock();
		for (uint32_t i = 0; i < count; i++)
		{
			int result = pthread_cond_signal(&m_cond);
			CE_ASSERT(result == 0, "pthread_cond_signal: errno = %d", result);
			CE_UNUSED(result);
		}

		_count += count;
		_mutex.unlock();
#elif CROWN_PLATFORM_WINDOWS
		ReleaseSemaphore(_handle, count, NULL);
#endif
	}

	void wait()
	{
#if CROWN_PLATFORM_POSIX
		_mutex.lock();
		while (_count <= 0)
		{
			int result = pthread_cond_wait(&m_cond, &(_mutex._mutex));
			CE_ASSERT(result == 0, "pthread_cond_wait: errno = %d", result);
			CE_UNUSED(result);
		}

		_count--;
		_mutex.unlock();
#elif CROWN_PLATFORM_WINDOWS
		DWORD result = WaitForSingleObject(_handle, INFINITE);
		CE_ASSERT(result == WAIT_OBJECT_0, "WaitForSingleObject: GetLastError = %d", GetLastError());
		CE_UNUSED(result);
#endif
	}

private:

#if CROWN_PLATFORM_POSIX
	Mutex _mutex;
	pthread_cond_t m_cond;
	int32_t _count;
#elif CROWN_PLATFORM_WINDOWS
	HANDLE _handle;
#endif

private:

	// Disable copying
	Semaphore(const Semaphore& s);
	Semaphore& operator=(const Semaphore& s);
};

} // namespace crown
