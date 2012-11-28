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

#pragma once

#include "Types.h"
#include "List.h"
#include "Allocator.h"
#include <cassert>
#include <cstring>

namespace crown
{

/**
	Circular buffer double-ended queue.
*/
template <typename T>
class Queue
{
public:

					Queue(Allocator& allocator);
					~Queue();

	T&				operator[](uint32_t index);
	const T&		operator[](uint32_t index) const;

	bool			empty() const;
	uint32_t		size() const;
	uint32_t		space() const;
	void			increase_capacity(uint32_t capacity);
	void			grow(uint32_t min_capacity);

	void			push_back(const T& item);
	void			pop_back();
	void			push_front(const T& item);
	void			pop_front();
	
	void 			clear();

	T*				begin();
	const T*		begin() const;
	T*				end();
	const T*		end() const;

	T&				front();
	const T&		front() const;
	T&				back();
	const T&		back() const;

private:

	uint32_t		m_read;
	uint32_t		m_size;
	List<T>			m_queue;
};

//-----------------------------------------------------------------------------
template <typename T>
inline Queue<T>::Queue(Allocator& allocator) :
	m_read(0),
	m_size(0),
	m_queue(allocator)
{
}

//-----------------------------------------------------------------------------
template <typename T>
inline Queue<T>::~Queue()
{
}

//-----------------------------------------------------------------------------
template <typename T>
inline T& Queue<T>::operator[](uint32_t index)
{
	return m_queue[(m_read + index) % m_queue.size()];
}

//-----------------------------------------------------------------------------
template <typename T>
inline const T& Queue<T>::operator[](uint32_t index) const
{
	return m_queue[(m_read + index) % m_queue.size()];
}

//-----------------------------------------------------------------------------
template <typename T>
inline bool Queue<T>::empty() const
{
	return m_size == 0;
}

//-----------------------------------------------------------------------------
template <typename T>
inline uint32_t Queue<T>::size() const
{
	return m_size;
}

//-----------------------------------------------------------------------------
template <typename T>
inline uint32_t Queue<T>::space() const
{
	return m_queue.size() - m_size;
}

//-----------------------------------------------------------------------------
template <typename T>
inline void Queue<T>::increase_capacity(uint32_t capacity)
{
	uint32_t old_size = m_queue.size();

	m_queue.resize(capacity);

	if (m_read + m_size > old_size)
	{
		memmove(m_queue.begin() + capacity - (old_size - m_read), m_queue.begin() + m_read, (old_size - m_read) * sizeof(T));

		m_read += (capacity - old_size);
	}
}

//-----------------------------------------------------------------------------
template <typename T>
inline void Queue<T>::grow(uint32_t min_capacity)
{
	uint32_t new_capacity = m_queue.size() * 2 + 1;

	if (new_capacity < min_capacity)
	{
		new_capacity = min_capacity;
	}

	increase_capacity(new_capacity);
}

//-----------------------------------------------------------------------------
template <typename T>
inline void Queue<T>::push_back(const T& item)
{
	if (space() == 0)
	{
		grow(0);
	}

	(*this)[m_size] = item;

	m_size++;
}

//-----------------------------------------------------------------------------
template <typename T>
inline void Queue<T>::pop_back()
{
	assert(m_size > 0);

	m_size--;
}

//-----------------------------------------------------------------------------
template <typename T>
inline void Queue<T>::push_front(const T& item)
{
	if (space() == 0)
	{
		grow(0);
	}

	m_read = (m_read - 1 + m_queue.size()) % m_queue.size();

	(*this)[0] = item;

	m_size++;
}

//-----------------------------------------------------------------------------
template <typename T>
inline void Queue<T>::pop_front()
{
	assert(m_size > 0);

	m_read = (m_read + 1) % m_queue.size();

	m_size--;
}

//-----------------------------------------------------------------------------
template <typename T>
inline void Queue<T>::clear()
{
	m_read = 0;
	m_size = 0;
}

//-----------------------------------------------------------------------------
template <typename T>
inline T* Queue<T>::begin()
{
	return m_queue.begin() + m_read;
}

//-----------------------------------------------------------------------------
template <typename T>
inline const T* Queue<T>::begin() const
{
	return m_queue.begin() + m_read;
}

//-----------------------------------------------------------------------------
template <typename T>
inline T* Queue<T>::end()
{
	uint32_t end = m_read + m_size;

	return end >= m_queue.size() ? m_queue.end() : m_queue.begin() + end;
}

//-----------------------------------------------------------------------------
template <typename T>
inline const T* Queue<T>::end() const
{
	uint32_t end = m_read + m_size;

	return end >= m_queue.size() ? m_queue.end() : m_queue.begin() + end;
}

//-----------------------------------------------------------------------------
template <typename T>
inline T& Queue<T>::front()
{
	assert(m_size > 0);

	return m_queue[m_read];
}

//-----------------------------------------------------------------------------
template <typename T>
inline const T& Queue<T>::front() const
{
	assert(m_size > 0);

	return m_queue[m_read];
}

//-----------------------------------------------------------------------------
template <typename T>
inline T& Queue<T>::back()
{
	assert(m_size > 0);

	return (*this)[m_size - 1];
}

//-----------------------------------------------------------------------------
template <typename T>
inline const T& Queue<T>::back() const
{
	assert(m_size > 0);

	return (*this)[m_size - 1];
}

} // namespace crown

