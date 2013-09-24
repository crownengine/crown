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
#include <limits.h>

#include "Assert.h"

class Semaphore
{
public:

	Semaphore();
	~Semaphore();

	void post(uint32_t count = 1) const;
	bool wait(int32_t msecs = -1) const;

private:

	Semaphore(const Semaphore& s); // no copy constructor
	Semaphore& operator=(const Semaphore& s); // no assignment operator

	HANDLE m_handle;
};

//-----------------------------------------------------------------------------
inline Semaphore::Semaphore()
{
	m_handle = CreateSemaphore(NULL, 0, LONG_MAX, NULL);
	CE_ASSERT(m_handle != NULL, "Unable to create semaphore!");
}

//-----------------------------------------------------------------------------
inline Semaphore::~Semaphore()
{
	CloseHandle(m_handle);
}

//-----------------------------------------------------------------------------
inline void Semaphore::post(uint32_t count) const
{
	ReleaseSemaphore(m_handle, count, NULL);
}

//-----------------------------------------------------------------------------
inline bool Semaphore::wait(int32_t msecs) const
{
	DWORD milliseconds = (0 > msecs) ? INFINITE : msecs;
	return WAIT_OBJECT_0 == WaitForSingleObject(m_handle, milliseconds);
}