/*
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

#include "Cond.h"

namespace crown
{
namespace os
{

//-----------------------------------------------------------------------------
Cond::Cond()
{
	memset(&m_cond, 0, sizeof(pthread_cond_t));

	pthread_cond_init(&m_cond, NULL);
}

//-----------------------------------------------------------------------------
Cond::~Cond()
{
	pthread_cond_destroy(&m_cond);
}

//-----------------------------------------------------------------------------
void Cond::signal()
{
	pthread_cond_signal(&m_cond);
}

//-----------------------------------------------------------------------------
void Cond::wait(Mutex& mutex)
{
	pthread_cond_wait(&m_cond, &(mutex.m_mutex));
}

} // namespace os
} // namespace crown
