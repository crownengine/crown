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

#include <cstring>
#include "ContainerTypes.h"
#include "Array.h"
#include "Assert.h"

namespace crown
{

/// Functions to manipulate Queue.
///
/// @ingroup Containers
namespace queue
{
	/// Returns whether the queue is empty.
	template<typename T> bool empty(const Queue<T>& q);

	/// Returns the number of items in the queue
	template<typename T> uint32_t size(const Queue<T>& q);

	/// Returns the number of items the queue can hold before
	/// a resize must occur.
	template<typename T> uint32_t space(const Queue<T>& q);

	/// Increase or decrease the capacity of the queue.
	/// @note
	/// Old items will be copied to the newly created queue.
	/// If the new @a capacity is smaller than the previous one, the
	/// queue will be truncated.
	template<typename T> void increase_capacity(Queue<T>& q, uint32_t capacity);

	/// Grows the queue to contain at least @a min_capacity items.
	/// If @a min_capacity is set to 0, the queue automatically
	/// determines the new capacity based on its size at the 
	/// time of call.
	template<typename T> void grow(Queue<T>& q, uint32_t min_capacity);

	/// Appends an @a item to the back of the queue
	template<typename T> void push_back(Queue<T>& q, const T& item);

	/// Removes the last item from the queue
	template<typename T> void pop_back(Queue<T>& q);

	/// Appends an @a item to the front of the queue
	template<typename T> void push_front(Queue<T>& q, const T& item);

	/// Removes the first item from the queue
	template<typename T> void pop_front(Queue<T>& q);

	/// Appends @a n @a items to the back of the queue
	template<typename T> void push(Queue<T>& q, const T *items, uint32_t n);

	/// Removes @a n items from the front of the queue
	template<typename T> void pop(Queue<T>& q, uint32_t n);

	/// Clears the content of the queue.
	/// @note
	/// Does not free memory nor call destructors, it only zeroes
	/// the number of items in the queue for efficiency.
	template<typename T> void clear(Queue<T>& q);

	template<typename T> T* begin(Queue<T>& q);
	template<typename T> const T* begin(const Queue<T>& q);
	template<typename T> T* end(Queue<T>& q);
	template<typename T> const T* end(const Queue<T>& q);

	template<typename T> T& front(Queue<T>& q);
	template<typename T> const T& front(const Queue<T>& q);
	template<typename T> T& back(Queue<T>& q);
	template<typename T> const T& back(const Queue<T>& q);
} // namespace queue

namespace queue
{
	//-----------------------------------------------------------------------------
	template <typename T>
	inline bool empty(const Queue<T>& q)
	{
		return q.m_size == 0;
	}

	//-----------------------------------------------------------------------------
	template <typename T>
	inline uint32_t size(const Queue<T>& q)
	{
		return q.m_size;
	}

	//-----------------------------------------------------------------------------
	template <typename T>
	inline uint32_t space(const Queue<T>& q)
	{
		return array::size(q.m_queue) - q.m_size;
	}

	//-----------------------------------------------------------------------------
	template <typename T>
	inline void increase_capacity(Queue<T>& q, uint32_t capacity)
	{
		uint32_t old_size = array::size(q.m_queue);

		array::resize(q.m_queue, capacity);

		if (q.m_read + q.m_size > old_size)
		{
			memmove(array::begin(q.m_queue) + capacity - (old_size - q.m_read), array::begin(q.m_queue) + q.m_read, (old_size - q.m_read) * sizeof(T));

			q.m_read += (capacity - old_size);
		}
	}

	//-----------------------------------------------------------------------------
	template <typename T>
	inline void grow(Queue<T>& q, uint32_t min_capacity)
	{
		uint32_t new_capacity = array::size(q.m_queue) * 2 + 1;

		if (new_capacity < min_capacity)
		{
			new_capacity = min_capacity;
		}

		increase_capacity(q, new_capacity);
	}

	//-----------------------------------------------------------------------------
	template <typename T>
	inline void push_back(Queue<T>& q, const T& item)
	{
		if (space(q) == 0)
		{
			grow(q, 0);
		}

		q[q.m_size] = item;

		q.m_size++;
	}

	//-----------------------------------------------------------------------------
	template <typename T>
	inline void pop_back(Queue<T>& q)
	{
		CE_ASSERT(q.m_size > 0, "The queue is empty");

		q.m_size--;
	}

	//-----------------------------------------------------------------------------
	template <typename T>
	inline void push_front(Queue<T>& q, const T& item)
	{
		if (space(q) == 0)
		{
			grow(q, 0);
		}

		q.m_read = (q.m_read - 1 + array::size(q.m_queue)) % array::size(q.m_queue);

		q[0] = item;

		q.m_size++;
	}

	//-----------------------------------------------------------------------------
	template <typename T>
	inline void pop_front(Queue<T>& q)
	{
		CE_ASSERT(q.m_size > 0, "The queue is empty");

		q.m_read = (q.m_read + 1) % array::size(q.m_queue);
		q.m_size--;
	}

	//-----------------------------------------------------------------------------
	template <typename T>
	inline void push(Queue<T>& q, const T *items, uint32_t n)
	{
		if (q.space() < n)
		{
			q.grow(q.size() + n);		
		}

		const uint32_t size = array::size(q.m_queue);
		const uint32_t insert = (q.m_read + q.m_size) % size;

		uint32_t to_insert = n;
		if (insert + to_insert > size)
		{
			to_insert = size - insert;
		}

		memcpy(array::begin(q.m_queue) + insert, items, to_insert * sizeof(T));

		q.m_size += to_insert;
		items += to_insert;
		n -= to_insert;
		memcpy(array::begin(q.m_queue), items, n * sizeof(T));

		q.m_size += n;
	}

	//-----------------------------------------------------------------------------
	template <typename T>
	inline void pop(Queue<T>& q, uint32_t n)
	{
		CE_ASSERT(q.m_size > 0, "The queue is empty");

		q.m_read = (q.m_read + n) % array::size(q.m_queue);
		q.m_size -= n;
	}

	//-----------------------------------------------------------------------------
	template <typename T>
	inline void clear(Queue<T>& q)
	{
		q.m_read = 0;
		q.m_size = 0;
	}

	//-----------------------------------------------------------------------------
	template <typename T>
	inline T* begin(Queue<T>& q)
	{
		return array::begin(q.m_queue) + q.m_read;
	}

	//-----------------------------------------------------------------------------
	template <typename T>
	inline const T* begin(const Queue<T>& q)
	{
		return array::begin(q.m_queue) + q.m_read;
	}

	//-----------------------------------------------------------------------------
	template <typename T>
	inline T* end(Queue<T>& q)
	{
		uint32_t end = q.m_read + q.m_size;

		return end >= array::size(q.m_queue) ? array::end(q.m_queue) : array::begin(q.m_queue) + end;
	}

	//-----------------------------------------------------------------------------
	template <typename T>
	inline const T* end(const Queue<T>& q)
	{
		uint32_t end = q.m_read + q.m_size;

		return end >= array::size(q.m_queue) ? array::end(q.m_queue) : array::begin(q.m_queue) + end;
	}

	//-----------------------------------------------------------------------------
	template <typename T>
	inline T& front(Queue<T>& q)
	{
		CE_ASSERT(q.m_size > 0, "The queue is empty");

		return q.m_queue[q.m_read];
	}

	//-----------------------------------------------------------------------------
	template <typename T>
	inline const T& front(const Queue<T>& q)
	{
		CE_ASSERT(q.m_size > 0, "The queue is empty");

		return q.m_queue[q.m_read];
	}

	//-----------------------------------------------------------------------------
	template <typename T>
	inline T& back(Queue<T>& q)
	{
		CE_ASSERT(q.m_size > 0, "The queue is empty");

		return q[q.m_size - 1];
	}

	//-----------------------------------------------------------------------------
	template <typename T>
	inline const T& back(const Queue<T>& q)
	{
		CE_ASSERT(q.m_size > 0, "The queue is empty");

		return q[q.m_size - 1];
	}

} // namespace queue

//-----------------------------------------------------------------------------
template <typename T>
inline Queue<T>::Queue(Allocator& allocator)
	: m_read(0)
	, m_size(0)
	, m_queue(allocator)
{
}

//-----------------------------------------------------------------------------
template <typename T>
inline T& Queue<T>::operator[](uint32_t index)
{
	return m_queue[(m_read + index) % array::size(m_queue)];
}

//-----------------------------------------------------------------------------
template <typename T>
inline const T& Queue<T>::operator[](uint32_t index) const
{
	return m_queue[(m_read + index) % array::size(m_queue)];
}

} // namespace crown

