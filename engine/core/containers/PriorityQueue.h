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

#include <algorithm>
#include "ContainerTypes.h"

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
	//-----------------------------------------------------------------------------
	template <typename T>
	const T& top(const PriorityQueue<T>& q)
	{
		return array::front(q.m_queue);
	}

	//-----------------------------------------------------------------------------
	template <typename T>
	void push(PriorityQueue<T>& q, const T& item)
	{
		array::push_back(q.m_queue, item);
		std::push_heap(array::begin(q.m_queue), array::end(q.m_queue));
	}

	//-----------------------------------------------------------------------------
	template <typename T>
	void pop(PriorityQueue<T>& q)
	{
		std::pop_heap(array::begin(q.m_queue), array::end(q.m_queue));
		array::pop_back(q.m_queue);
	}
} // namespace priority_queue

template <typename T>
PriorityQueue<T>::PriorityQueue(Allocator& a)
	: m_queue(a)
{
}

} // namespace crown
