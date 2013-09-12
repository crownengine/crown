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
#include "OS.h"
#include "Assert.h"

namespace crown
{

class Mutex
{
public:

						Mutex();
						~Mutex();

	void				lock();
	void				unlock();

private:

	pthread_mutex_t		m_mutex;

private:

	// Disable copying.
						Mutex(const Mutex&);
	Mutex&				operator=(const Mutex&);

	friend class		Cond;
};

//-----------------------------------------------------------------------------
inline Mutex::Mutex()
{
	memset(&m_mutex, 0, sizeof(pthread_mutex_t));

	int32_t result = pthread_mutex_init(&m_mutex, NULL);

	CE_ASSERT(result == 0, "Failed to init mutex");
}

//-----------------------------------------------------------------------------
inline Mutex::~Mutex()
{
	int32_t result = pthread_mutex_destroy(&m_mutex);

	CE_ASSERT(result == 0, "Failed to destroy mutex");
}

//-----------------------------------------------------------------------------
inline void Mutex::lock()
{
	int32_t result = pthread_mutex_lock(&m_mutex);

	CE_ASSERT(result == 0, "Failed to acquire lock");
}

//-----------------------------------------------------------------------------
inline void Mutex::unlock()
{
	int32_t result = pthread_mutex_unlock(&m_mutex);

	CE_ASSERT(result == 0, "Failed to release lock");
}

} // namespace crown
