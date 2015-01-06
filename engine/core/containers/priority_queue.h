/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "container_types.h"
#include "array.h"
#include <algorithm>

namespace crown
{

/// Functions to manipulate PriorityQueue.
///
/// @ingroup Containers
namespace priority_queue
{
	/// Returns the first item in the queue.
	template <typename T> const T& top(const PriorityQueue<T>& q);

	/// Inserts @a item into the queue.
	template <typename T> void push(PriorityQueue<T>& q, const T& item);

	/// Removes the first item from the queue.
	template <typename T> void pop(PriorityQueue<T>& q);
} // namespace priority_queue

namespace priority_queue
{
	template <typename T>
	const T& top(const PriorityQueue<T>& q)
	{
		return array::front(q._queue);
	}

	template <typename T>
	void push(PriorityQueue<T>& q, const T& item)
	{
		array::push_back(q._queue, item);
		std::push_heap(array::begin(q._queue), array::end(q._queue));
	}

	template <typename T>
	void pop(PriorityQueue<T>& q)
	{
		std::pop_heap(array::begin(q._queue), array::end(q._queue));
		array::pop_back(q._queue);
	}
} // namespace priority_queue

template <typename T>
PriorityQueue<T>::PriorityQueue(Allocator& a)
	: _queue(a)
{
}

} // namespace crown
