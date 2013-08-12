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
#include "List.h"

namespace crown
{

template <typename T>
class PriorityQueue
{
public:

	/// Does not allocate memory.
					PriorityQueue(Allocator& a);
					~PriorityQueue();

	/// Returns whether the queue is empty.
	bool			empty() const;

	/// Returns the number of items in the queue.
	uint32_t		size() const;

	/// Returns the first item in the queue.
	const T&		top() const;

	/// Inserts @a item into the queue.
	void			push(const T& item);

	/// Removes the first item from the queue.
	void			pop();

	T*				begin();
	const T*		begin() const;
	T*				end();
	const T*		end() const;

	T&				front();
	const T&		front() const;
	T&				back();
	const T&		back() const;

private:

	List<T>			m_queue;
};

//-----------------------------------------------------------------------------
template <typename T>
PriorityQueue<T>::PriorityQueue(Allocator& a) :
	m_queue(a)
{
}

//-----------------------------------------------------------------------------
template <typename T>
PriorityQueue<T>::~PriorityQueue()
{
}

//-----------------------------------------------------------------------------
template <typename T>
bool PriorityQueue<T>::empty() const
{
	return m_queue.empty();
}

//-----------------------------------------------------------------------------
template <typename T>
uint32_t PriorityQueue<T>::size() const
{
	return m_queue.size();
}

//-----------------------------------------------------------------------------
template <typename T>
const T& PriorityQueue<T>::top() const
{
	return m_queue.front();
}

//-----------------------------------------------------------------------------
template <typename T>
void PriorityQueue<T>::push(const T& item)
{
	m_queue.push_back(item);

	std::push_heap(begin(), end());
}

//-----------------------------------------------------------------------------
template <typename T>
void PriorityQueue<T>::pop()
{
	std::pop_heap(begin(), end());

	m_queue.pop_back();
}

//-----------------------------------------------------------------------------
template <typename T>
const T* PriorityQueue<T>::begin() const
{
	return m_queue.begin();
}

//-----------------------------------------------------------------------------
template <typename T>
T* PriorityQueue<T>::begin()
{
	return m_queue.begin();
}

//-----------------------------------------------------------------------------
template <typename T>
const T* PriorityQueue<T>::end() const
{
	return m_queue.end();
}

//-----------------------------------------------------------------------------
template <typename T>
T* PriorityQueue<T>::end()
{
	return m_queue.end();
}

//-----------------------------------------------------------------------------
template <typename T>
const T& PriorityQueue<T>::front() const
{
	return m_queue.front();
}

//-----------------------------------------------------------------------------
template <typename T>
T& PriorityQueue<T>::front()
{
	return m_queue.front();
}

//-----------------------------------------------------------------------------
template <typename T>
const T& PriorityQueue<T>::back() const
{
	return m_queue.back();
}

//-----------------------------------------------------------------------------
template <typename T>
T& PriorityQueue<T>::back()
{
	return m_queue.back();
}

} // namespace crown
