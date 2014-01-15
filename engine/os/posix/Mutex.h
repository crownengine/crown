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
	pthread_mutexattr_t m_attr;

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
	int result;

	result = pthread_mutexattr_init(&m_attr);
	CE_ASSERT(result == 0, "Failed to init mutex attr. errno: %d", result);
	result = pthread_mutexattr_settype(&m_attr, PTHREAD_MUTEX_ERRORCHECK);
	CE_ASSERT(result == 0, "Failed to set mutex type. errno: %d", result);
	result = pthread_mutex_init(&m_mutex, &m_attr);
	CE_ASSERT(result == 0, "Failed to init mutex. errno: %d", result);
}

//-----------------------------------------------------------------------------
inline Mutex::~Mutex()
{
	int result;

	result = pthread_mutex_destroy(&m_mutex);
	CE_ASSERT(result == 0, "Failed to destroy mutex. errno: %d", result);
	result = pthread_mutexattr_destroy(&m_attr);
	CE_ASSERT(result == 0, "Failed to destroy mutex attr. errno: %d", result);
}

//-----------------------------------------------------------------------------
inline void Mutex::lock()
{
	int result = pthread_mutex_lock(&m_mutex);
	CE_ASSERT(result == 0, "Failed to acquire lock. errno: %d", result);
}

//-----------------------------------------------------------------------------
inline void Mutex::unlock()
{
	int result = pthread_mutex_unlock(&m_mutex);
	CE_ASSERT(result == 0, "Failed to release lock. errno: %d", result);
}

} // namespace crown
