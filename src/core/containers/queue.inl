/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/containers/array.inl"
#include "core/containers/types.h"
#include "core/error/error.h"
#include <string.h> // memcpy

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
	template<typename T> u32 size(const Queue<T>& q);

	/// Returns the number of items the queue can hold before
	/// a resize must occur.
	template<typename T> u32 space(const Queue<T>& q);

	/// Increase or decrease the capacity of the queue.
	/// @note
	/// Old items will be copied to the newly created queue.
	/// If the new @a capacity is smaller than the previous one, the
	/// queue will be truncated.
	template<typename T> void increase_capacity(Queue<T>& q, u32 capacity);

	/// Grows the queue to contain at least @a min_capacity items.
	/// If @a min_capacity is set to 0, the queue automatically
	/// determines the new capacity based on its size at the
	/// time of call.
	template<typename T> void grow(Queue<T>& q, u32 min_capacity);

	/// Appends an @a item to the back of the queue
	template<typename T> void push_back(Queue<T>& q, const T& item);

	/// Removes the last item from the queue
	template<typename T> void pop_back(Queue<T>& q);

	/// Appends an @a item to the front of the queue
	template<typename T> void push_front(Queue<T>& q, const T& item);

	/// Removes the first item from the queue
	template<typename T> void pop_front(Queue<T>& q);

	/// Appends @a n @a items to the back of the queue
	template<typename T> void push(Queue<T>& q, const T *items, u32 n);

	/// Removes @a n items from the front of the queue
	template<typename T> void pop(Queue<T>& q, u32 n);

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
	template <typename T>
	inline bool empty(const Queue<T>& q)
	{
		return q._size == 0;
	}

	template <typename T>
	inline u32 size(const Queue<T>& q)
	{
		return q._size;
	}

	template <typename T>
	inline u32 space(const Queue<T>& q)
	{
		return array::size(q._queue) - q._size;
	}

	template <typename T>
	inline void increase_capacity(Queue<T>& q, u32 capacity)
	{
		u32 old_size = array::size(q._queue);

		array::resize(q._queue, capacity);

		if (q._read + q._size > old_size)
		{
			memmove(array::begin(q._queue) + capacity - (old_size - q._read), array::begin(q._queue) + q._read, (old_size - q._read) * sizeof(T));
			q._read += (capacity - old_size);
		}
	}

	template <typename T>
	inline void grow(Queue<T>& q, u32 min_capacity)
	{
		u32 new_capacity = array::size(q._queue) * 2 + 1;

		if (new_capacity < min_capacity)
			new_capacity = min_capacity;

		increase_capacity(q, new_capacity);
	}

	template <typename T>
	inline void push_back(Queue<T>& q, const T& item)
	{
		if (space(q) == 0)
			grow(q, 0);

		q[q._size] = item;

		++q._size;
	}

	template <typename T>
	inline void pop_back(Queue<T>& q)
	{
		CE_ASSERT(q._size > 0, "The queue is empty");
		--q._size;
	}

	template <typename T>
	inline void push_front(Queue<T>& q, const T& item)
	{
		if (space(q) == 0)
			grow(q, 0);

		q._read = (q._read - 1 + array::size(q._queue)) % array::size(q._queue);

		q[0] = item;

		++q._size;
	}

	template <typename T>
	inline void pop_front(Queue<T>& q)
	{
		CE_ASSERT(q._size > 0, "The queue is empty");

		q._read = (q._read + 1) % array::size(q._queue);
		--q._size;
	}

	template <typename T>
	inline void push(Queue<T>& q, const T *items, u32 n)
	{
		if (q.space() < n)
			q.grow(q.size() + n);

		const u32 size = array::size(q._queue);
		const u32 insert = (q._read + q._size) % size;

		u32 to_insert = n;
		if (insert + to_insert > size)
			to_insert = size - insert;

		memcpy(array::begin(q._queue) + insert, items, to_insert * sizeof(T));

		q._size += to_insert;
		items += to_insert;
		n -= to_insert;
		memcpy(array::begin(q._queue), items, n * sizeof(T));

		q._size += n;
	}

	template <typename T>
	inline void pop(Queue<T>& q, u32 n)
	{
		CE_ASSERT(q._size > 0, "The queue is empty");

		q._read = (q._read + n) % array::size(q._queue);
		q._size -= n;
	}

	template <typename T>
	inline void clear(Queue<T>& q)
	{
		q._read = 0;
		q._size = 0;
	}

	template <typename T>
	inline T* begin(Queue<T>& q)
	{
		return array::begin(q._queue) + q._read;
	}

	template <typename T>
	inline const T* begin(const Queue<T>& q)
	{
		return array::begin(q._queue) + q._read;
	}

	template <typename T>
	inline T* end(Queue<T>& q)
	{
		const u32 end = q._read + q._size;
		return end >= array::size(q._queue) ? array::end(q._queue) : array::begin(q._queue) + end;
	}

	template <typename T>
	inline const T* end(const Queue<T>& q)
	{
		const u32 end = q._read + q._size;
		return end >= array::size(q._queue) ? array::end(q._queue) : array::begin(q._queue) + end;
	}

	template <typename T>
	inline T& front(Queue<T>& q)
	{
		CE_ASSERT(q._size > 0, "The queue is empty");
		return q._queue[q._read];
	}

	template <typename T>
	inline const T& front(const Queue<T>& q)
	{
		CE_ASSERT(q._size > 0, "The queue is empty");
		return q._queue[q._read];
	}

	template <typename T>
	inline T& back(Queue<T>& q)
	{
		CE_ASSERT(q._size > 0, "The queue is empty");
		return q[q._size - 1];
	}

	template <typename T>
	inline const T& back(const Queue<T>& q)
	{
		CE_ASSERT(q._size > 0, "The queue is empty");
		return q[q._size - 1];
	}

} // namespace queue

template <typename T>
inline Queue<T>::Queue(Allocator& a)
	: _read(0)
	, _size(0)
	, _queue(a)
{
}

template <typename T>
inline T& Queue<T>::operator[](u32 index)
{
	return _queue[(_read + index) % array::size(_queue)];
}

template <typename T>
inline const T& Queue<T>::operator[](u32 index) const
{
	return _queue[(_read + index) % array::size(_queue)];
}

} // namespace crown
