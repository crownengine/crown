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
#include "Assert.h"
#include <cstring>

namespace crown
{

/// Circular buffer double-ended queue of POD items.
/// @note
/// Does not call constructors/destructors so it is not very suitable for non-POD items.
template <typename T>
class Queue
{
public:

					Queue(Allocator& allocator);
					~Queue();

	/// Random access by index
	T&				operator[](uint32_t index);

	/// Random access by index
	const T&		operator[](uint32_t index) const;

	/// Returns whether the queue is empty.
	bool			empty() const;

	/// Returns the number of items in the queue
	uint32_t		size() const;

	/// Returns the number of items the queue can hold before
	/// a resize must occur.
	uint32_t		space() const;

	/// Increase or decrease the capacity of the queue.
	/// @note
	/// Old items will be copied to the newly created queue.
	/// If the new @a capacity is smaller than the previous one, the
	/// queue will be truncated.
	void			increase_capacity(uint32_t capacity);

	/// Grows the queue to contain at least @a min_capacity items.
	/// If @a min_capacity is set to 0, the queue automatically
	/// determines the new capacity based on its size at the 
	/// time of call.
	void			grow(uint32_t min_capacity);

	/// Adds an @a item to the back of the queue
	void			push_back(const T& item);

	/// Removes the last item from the queue
	void			pop_back();

	/// Adds an @a item to the front of the queue
	void			push_front(const T& item);

	/// Removes the first item from the queue
	void			pop_front();
	
	/// Clears the content of the queue.
	/// @note
	/// Does not free memory nor call destructors, it only zeroes
	/// the number of items in the queue for efficiency.
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
	CE_ASSERT(m_size > 0, "The queue is empty");

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
	CE_ASSERT(m_size > 0, "The queue is empty");

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
	CE_ASSERT(m_size > 0, "The queue is empty");

	return m_queue[m_read];
}

//-----------------------------------------------------------------------------
template <typename T>
inline const T& Queue<T>::front() const
{
	CE_ASSERT(m_size > 0, "The queue is empty");

	return m_queue[m_read];
}

//-----------------------------------------------------------------------------
template <typename T>
inline T& Queue<T>::back()
{
	CE_ASSERT(m_size > 0, "The queue is empty");

	return (*this)[m_size - 1];
}

//-----------------------------------------------------------------------------
template <typename T>
inline const T& Queue<T>::back() const
{
	CE_ASSERT(m_size > 0, "The queue is empty");

	return (*this)[m_size - 1];
}

} // namespace crown

