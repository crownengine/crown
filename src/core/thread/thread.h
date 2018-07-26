/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/platform.h"
#include "core/thread/semaphore.h"
#include "core/types.h"

#if CROWN_PLATFORM_POSIX
	#include <pthread.h>
#elif CROWN_PLATFORM_WINDOWS
	#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#endif
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

	///
	Thread();

	///
	~Thread();

	///
	Thread(const Thread&) = delete;

	///
	Thread& operator=(const Thread&) = delete;

	///
	void start(ThreadFunction func, void* user_data = NULL, u32 stack_size = 0);

	///
	void stop();

	///
	bool is_running();
};

} // namespace crown
