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
#include "Semaphore.h"
#include "Assert.h"

namespace crown
{

typedef int32_t (*ThreadFunction)(void*);

class OsThread
{
public:
						OsThread(const char* name);
						~OsThread();

	void				start(ThreadFunction func, void* data = NULL, size_t stack_size = 0);
	void				stop();

	bool				is_running();

private:

	int32_t				run();

	static DWORD WINAPI	thread_proc(void* arg);

	OsThread(const OsThread& t); // no copy constructor
	OsThread& operator=(const OsThread& t); // no assignment operator	

private:

	const char*			m_name;
	HANDLE				m_handle;

	ThreadFunction 		m_function;
	void*				m_data;
	Semaphore			m_sem;
	size_t 				m_stack_size;

	bool				m_is_running :1;

	DWORD				m_exit_code;
};

//-----------------------------------------------------------------------------
CE_INLINE OsThread::OsThread(const char* name) :
	m_name(name),
	m_handle(INVALID_HANDLE_VALUE),
	m_function(NULL),
	m_data(NULL),
	m_stack_size(0),
	m_is_running(false),
	m_exit_code(0)
{
	memset(&m_handle, 0, sizeof(HANDLE));
}

//-----------------------------------------------------------------------------
CE_INLINE OsThread::~OsThread()
{
	if (m_is_running)
	{
		stop();
	}
}

//-----------------------------------------------------------------------------
CE_INLINE void OsThread::start(ThreadFunction func, void* data, size_t stack_size)
{
	CE_ASSERT(!m_is_running, "Thread is already running");
	CE_ASSERT(func != NULL, "Function must be != NULL");

	m_function = func;
	m_data = data;
	m_stack_size = stack_size;

	m_handle = CreateThread(NULL, stack_size, OsThread::thread_proc, this, 0, NULL);

	CE_ASSERT(m_handle != NULL, "Failed to create the thread '%s'", m_name);

	m_is_running = true;

	m_sem.wait();
}

//-----------------------------------------------------------------------------
CE_INLINE void OsThread::stop()
{
	CE_ASSERT(m_is_running, "Thread is not running");

	WaitForSingleObject(m_handle, INFINITE);
	GetExitCodeThread(m_handle, &m_exit_code);
	CloseHandle(m_handle);
	m_handle = INVALID_HANDLE_VALUE;

	m_is_running = false;
}

//-----------------------------------------------------------------------------
CE_INLINE bool OsThread::is_running()
{
	return m_is_running;
}

//-----------------------------------------------------------------------------
CE_INLINE int32_t OsThread::run()
{
	m_sem.post();

	return m_function(m_data);
}

//-----------------------------------------------------------------------------
CE_INLINE DWORD WINAPI OsThread::thread_proc(void* arg)
{
	OsThread* thread = (OsThread*)arg;

	int32_t result = thread->run();
	
	return result;
}

} // namespace crown
