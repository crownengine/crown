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

#include <windows.h>
#include <process.h>
#include <WinBase.h>

#include "Types.h"
#include "OS.h"

namespace crown
{
namespace os
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

	const char*			m_name;
	HANDLE				m_handle;

	ThreadFunction 		m_function;
	void*				m_data;
	Semaphore			m_sem;
	size_t 				m_stack_size;

	bool				m_is_running :1;
};

//-----------------------------------------------------------------------------
inline Thread::Thread(const char* name) :
	m_name(name),
	m_handle(INVALID_HANDLE_VALUE),
	m_function(NULL),
	m_data(NULL),
	m_stack_size(0),
	m_is_running(false)
{
	memset(&m_handle, 0, sizeof(HANDLE));
}

//-----------------------------------------------------------------------------
inline Thread::~Thread()
{
}

//-----------------------------------------------------------------------------
inline void	Thread::start(ThreadFunction func, void* data = NULL, size_t stack_size = 0)
{
	m_handle = CreateThread(NULL, stack_size, (LPTHREAD_START_ROUTINE) Thread::background_proc, this, 0, NULL);

	CE_ASSERT(m_handle != NULL, "Failed to create the thread '%s'", m_name);

	m_is_running = true;

	m_sem.wait();
}

//-----------------------------------------------------------------------------
inline void	Thread::stop()
{
	WaitForSingleObject(m_handle, INFINITE);
	GetExitCodeThread(m_handle, (DWORD*)&m_exitCode);
	CloseHandle(m_handle);
	m_handle = INVALID_HANDLE_VALUE;

	m_is_running = false;
}

//-----------------------------------------------------------------------------
inline bool	Thread::is_running()
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
inline DWORD WINAPI Thread::thread_proc(void* arg)
{
	Thread* thread = (Thread*)arg;

	int32_t result = thread->run();
	
	return result;
}


} // namespace os
} // namespace crown
