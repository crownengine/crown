/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/error/error.inl"
#include "core/platform.h"
#include "core/thread/semaphore.h"
#include "core/thread/thread.h"
#include <new>

#if CROWN_PLATFORM_POSIX
	#include <pthread.h>
#elif CROWN_PLATFORM_WINDOWS
	#include <windows.h>
	#include <process.h>
#endif

namespace crown
{
struct Private
{
	ThreadFunction _function;
	void* _user_data;
	Semaphore _sem;
	bool _is_running;
	s32 _exit_code;
#if CROWN_PLATFORM_POSIX
	pthread_t handle;
#elif CROWN_PLATFORM_WINDOWS
	HANDLE handle;
#endif
};

#if CROWN_PLATFORM_POSIX
static void* thread_proc(void* arg)
{
	Thread* thread = (Thread*)arg;
	thread->_priv->_sem.post();
	return (void*)(uintptr_t)thread->_priv->_function(thread->_priv->_user_data);
}
#elif CROWN_PLATFORM_WINDOWS
static DWORD WINAPI thread_proc(void* arg)
{
	Thread* thread = (Thread*)arg;
	thread->_priv->_sem.post();
	return thread->_priv->_function(thread->_priv->_user_data);
}
#endif

Thread::Thread()
{
	CE_STATIC_ASSERT(sizeof(_data) >= sizeof(*_priv));
	_priv = new (_data) Private();
	_priv->_function = NULL;
	_priv->_user_data = NULL;
	_priv->_is_running = false;
	_priv->_exit_code = 0;

#if CROWN_PLATFORM_POSIX
	_priv->handle = 0;
#elif CROWN_PLATFORM_WINDOWS
	_priv->handle = INVALID_HANDLE_VALUE;
#endif
}

Thread::~Thread()
{
	if (_priv->_is_running)
		stop();

	_priv->~Private();
}

void Thread::start(ThreadFunction func, void* user_data, u32 stack_size)
{
	CE_ASSERT(!_priv->_is_running, "Thread is already running");
	CE_ASSERT(func != NULL, "Function must be != NULL");
	_priv->_function = func;
	_priv->_user_data = user_data;

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

	err = pthread_create(&_priv->handle, &attr, thread_proc, this);
	CE_ASSERT(err == 0, "pthread_create: errno = %d", err);

	err = pthread_attr_destroy(&attr);
	CE_ASSERT(err == 0, "pthread_attr_destroy: errno = %d", err);
	CE_UNUSED(err);
#elif CROWN_PLATFORM_WINDOWS
	_priv->handle = CreateThread(NULL, stack_size, thread_proc, this, 0, NULL);
	CE_ASSERT(_priv->handle != NULL, "CreateThread: GetLastError = %d", GetLastError());
#endif

	_priv->_is_running = true;
	_priv->_sem.wait();
}

void Thread::stop()
{
	CE_ASSERT(_priv->_is_running, "Thread is not running");

#if CROWN_PLATFORM_POSIX
	void* retval;
	int err = pthread_join(_priv->handle, &retval);
	CE_ASSERT(err == 0, "pthread_join: errno = %d", err);
	CE_UNUSED(err);
	_priv->_exit_code = (s32)(uintptr_t)retval;
	_priv->handle = 0;
#elif CROWN_PLATFORM_WINDOWS
	WaitForSingleObject(_priv->handle, INFINITE);
	GetExitCodeThread(_priv->handle, (DWORD*)&_priv->_exit_code);
	CloseHandle(_priv->handle);
	_priv->handle = INVALID_HANDLE_VALUE;
#endif

	_priv->_is_running = false;
}

bool Thread::is_running()
{
	return _priv->_is_running;
}

s32 Thread::exit_code()
{
	return _priv->_exit_code;
}

} // namespace crown
