/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "core/platform.h"
#include "core/thread/semaphore.h"
#include "core/types.h"

#if CROWN_PLATFORM_POSIX
	#include <pthread.h>
#elif CROWN_PLATFORM_WINDOWS
	#include <windows.h>
	#include <process.h>
#endif

/// @defgroup Thread Thread
/// @ingroup Core
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

	Thread();

	~Thread();

	void start(ThreadFunction func, void* user_data = NULL, u32 stack_size = 0);

	void stop();

	bool is_running();

private:

	s32 run();

#if CROWN_PLATFORM_POSIX
	static void* thread_proc(void* arg);
#elif CROWN_PLATFORM_WINDOWS
	static DWORD WINAPI thread_proc(void* arg);
#endif

private:

	// Disable copying
	Thread(const Thread&);
	Thread& operator=(const Thread&);
};

} // namespace crown
