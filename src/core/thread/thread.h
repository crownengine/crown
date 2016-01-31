/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "error.h"
#include "platform.h"
#include "semaphore.h"
#include "types.h"

#if CROWN_PLATFORM_POSIX
	#include <pthread.h>
#elif CROWN_PLATFORM_WINDOWS
	#include "win_headers.h"
	#include <process.h>
#endif

/// @defgroup Thread Thread
namespace crown
{
/// Thread.
///
/// @ingroup Thread.
struct Thread
{
	typedef s32 (*ThreadFunction)(void* data);

	ThreadFunction _function;
	void* _user_data;
	Semaphore _sem;
	bool _is_running;
#if CROWN_PLATFORM_POSIX
	pthread_t _handle;
#elif CROWN_PLATFORM_WINDOWS
	HANDLE _handle;
#endif

	Thread()
		: _function(NULL)
		, _user_data(NULL)
		, _is_running(false)
#if CROWN_PLATFORM_POSIX
		, _handle(0)
#elif CROWN_PLATFORM_WINDOWS
		, _handle(INVALID_HANDLE_VALUE)
#endif
	{
	}

	~Thread()
	{
		if (_is_running)
			stop();
	}

	void start(ThreadFunction func, void* user_data = NULL, u32 stack_size = 0)
	{
		CE_ASSERT(!_is_running, "Thread is already running");
		CE_ASSERT(func != NULL, "Function must be != NULL");
		_function = func;
		_user_data = user_data;

#if CROWN_PLATFORM_POSIX
		pthread_attr_t attr;
		int err = pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
		CE_ASSERT(err == 0, "pthread_attr_init: errno = %d", err);

		if (stack_size != 0)
		{
			err = pthread_attr_setstacksize(&attr, stack_size);
			CE_ASSERT(err == 0, "pthread_attr_setstacksize: errno = %d", err);
		}

		err = pthread_create(&_handle, &attr, thread_proc, this);
		CE_ASSERT(err == 0, "pthread_create: errno = %d", err);

		err = pthread_attr_destroy(&attr);
		CE_ASSERT(err == 0, "pthread_attr_destroy: errno = %d", err);
		CE_UNUSED(err);
#elif CROWN_PLATFORM_WINDOWS
		_handle = CreateThread(NULL, stack_size, Thread::thread_proc, this, 0, NULL);
		CE_ASSERT(_handle != NULL, "CreateThread: GetLastError = %d", GetLastError());
#endif

		_is_running = true;
		_sem.wait();
	}

	void stop()
	{
		CE_ASSERT(_is_running, "Thread is not running");

#if CROWN_PLATFORM_POSIX
		int err = pthread_join(_handle, NULL);
		CE_ASSERT(err == 0, "pthread_join: errno = %d", err);
		CE_UNUSED(err);
		_handle = 0;
#elif CROWN_PLATFORM_WINDOWS
		WaitForSingleObject(_handle, INFINITE);
		// GetExitCodeThread(_handle, &m_exit_code);
		CloseHandle(_handle);
		_handle = INVALID_HANDLE_VALUE;
#endif

		_is_running = false;
	}

	bool is_running()
	{
		return _is_running;
	}

private:

	s32 run()
	{
		_sem.post();
		return _function(_user_data);
	}

#if CROWN_PLATFORM_POSIX
	static void* thread_proc(void* arg)
	{
		static s32 result = -1;
		result = ((Thread*)arg)->run();
		return (void*)&result;
	}
#elif CROWN_PLATFORM_WINDOWS
	static DWORD WINAPI thread_proc(void* arg)
	{
		Thread* thread = (Thread*)arg;
		s32 result = thread->run();
		return result;
	}
#endif

private:

	// Disable copying
	Thread(const Thread&);
	Thread& operator=(const Thread&);
};

} // namespace crown
