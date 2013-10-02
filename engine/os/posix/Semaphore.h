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

#include <errno.h>
#include <semaphore.h>

#include "Assert.h"
#include "Mutex.h"
#include "Cond.h"
#include "Log.h"

namespace crown
{

class Semaphore
{
public:

			Semaphore();
			~Semaphore();

	void	post(uint32_t count = 1);
	void	wait();

private:

	Mutex 	m_mutex;
	Cond 	m_cond;

	int32_t m_count;

private:

	Semaphore(const Semaphore& s); // no copy constructor
	Semaphore& operator=(const Semaphore& s); // no assignment operator
};

//-----------------------------------------------------------------------------
inline Semaphore::Semaphore() : m_count(0)
{
}

//-----------------------------------------------------------------------------
inline Semaphore::~Semaphore()
{
}

//-----------------------------------------------------------------------------
inline void Semaphore::post(uint32_t count)
{
	m_mutex.lock();

	for (uint32_t i = 0; i < count; i++)
	{
		m_cond.signal();
	}

	m_count += count;

	m_mutex.unlock();	
}

//-----------------------------------------------------------------------------
inline void Semaphore::wait()
{
	m_mutex.lock();

	while (m_count <= 0)
	{
		m_cond.wait(m_mutex);
	}

	m_count--;

	m_mutex.unlock();
}


} // namespace crown