/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "error.h"
#include "thread.h"

namespace crown
{
Thread::Thread()
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

Thread::~Thread()
{
	if (_is_running)
		stop();
}

void Thread::start(ThreadFunction func, void* user_data, u32 stack_size)
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

void Thread::stop()
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

bool Thread::is_running()
{
	return _is_running;
}

s32 Thread::run()
{
	_sem.post();
	return _function(_user_data);
}

#if CROWN_PLATFORM_POSIX
void* Thread::thread_proc(void* arg)
{
	static s32 result = -1;
	result = ((Thread*)arg)->run();
	return (void*)&result;
}
#elif CROWN_PLATFORM_WINDOWS
DWORD WINAPI Thread::thread_proc(void* arg)
{
	Thread* thread = (Thread*)arg;
	s32 result = thread->run();
	return result;
}
#endif

} // namespace crown
