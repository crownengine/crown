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

#include "Thread.h"
#include "Assert.h"
#include "StringUtils.h"

namespace crown
{
namespace os
{

//-----------------------------------------------------------------------------
Thread::Thread(os::ThreadFunction f, LPVOID params, const char* name)
{
	m_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) f, params, 0, NULL);

	CE_ASSERT(m_thread != NULL, "Unable to create thread");

	m_name = name;
}

//-----------------------------------------------------------------------------
Thread::~Thread()
{
}

//-----------------------------------------------------------------------------
void Thread::join()
{
	WaitForSingleObject(m_thread, INFINITE);
}

//-----------------------------------------------------------------------------
void Thread::detach()
{
	bool closed = CloseHandle(m_thread);

	CE_ASSERT(closed, "Unable to close thread");
}

} // namespace os
} // namespace crown
