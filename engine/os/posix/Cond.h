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

#include <pthread.h>
#include <cstring>

#include "Types.h"
#include "Mutex.h"
#include "OS.h"

namespace crown
{

class Cond
{
public:

					Cond();
					~Cond();

	void			signal();
	void			wait(Mutex& mutex);

private:

	pthread_cond_t	m_cond;

private:

	// Disable copying.
					Cond(const Cond&);
	Cond&			operator=(const Cond&);
};

//-----------------------------------------------------------------------------
inline Cond::Cond()
{
	memset(&m_cond, 0, sizeof(pthread_cond_t));

	int32_t result = pthread_cond_init(&m_cond, NULL);

	CE_ASSERT(result == 0, "Failed to init cond. errno: %d", result);
	CE_UNUSED(result);
}

//-----------------------------------------------------------------------------
inline Cond::~Cond()
{
	int32_t result = pthread_cond_destroy(&m_cond);

	CE_ASSERT(result == 0, "Failed to destroy cond. errno: %d", result);
	CE_UNUSED(result);
}

//-----------------------------------------------------------------------------
inline void Cond::signal()
{
	int32_t result = pthread_cond_signal(&m_cond);

	CE_ASSERT(result == 0, "Failed to signal cond. errno: %d", result);
	CE_UNUSED(result);
}

//-----------------------------------------------------------------------------
inline void Cond::wait(Mutex& mutex)
{
	int32_t result = pthread_cond_wait(&m_cond, &(mutex.m_mutex));

	CE_ASSERT(result == 0, "Failed to wait cond. errno: %d", result);
	CE_UNUSED(result);
}

} // namespace crown
