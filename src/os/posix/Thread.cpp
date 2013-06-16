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

#include <stdlib.h>
#include <string.h>

#include "Thread.h"

namespace crown
{
namespace os
{

//-----------------------------------------------------------------------------
Thread::Thread(os::ThreadFunction f, void* params, const char* name) :
	m_name(name)
{
	memset(&m_thread, 0, sizeof(pthread_t));

	// Make thread joinable
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	// Create thread
	int rc = pthread_create(&m_thread, &attr, f, (void*)params);

	if (rc != 0)
	{
		os::printf("Unable to create the thread '%s' Error code: %d\n", name, rc);
		exit(-1);
	}

	// Free attr memory
	pthread_attr_destroy(&attr);
}

//-----------------------------------------------------------------------------
Thread::~Thread()
{
}

//-----------------------------------------------------------------------------
void Thread::join()
{
	pthread_join(m_thread, NULL);
}

//-----------------------------------------------------------------------------
void Thread::detach()
{
	pthread_detach(m_thread);
}

} // namespace os
} // namespace crown
