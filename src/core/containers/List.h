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
#include <cassert>
#include <cstring>

namespace crown
{

/**
	Dynamic array.
*/
template <typename T>
class List
{

public:

						List();
						List(uint capacity);
						List(const List<T>& list);
						~List();

	T&					operator[](uint index);
	const T&			operator[](uint index) const;

	bool				empty() const;
	uint				size() const;
	uint				capacity() const;
	void				set_capacity(uint capacity);
	void				grow();

	void				condense();

	uint				push_back(const T& item);
	void				pop_back();
	void				clear();

	const List<T>&		operator=(const List<T>& other);

	T*					begin();
	const T*			begin() const;
	T*					end();
	const T*			end() const;

private:

	uint				m_capacity;
	uint				m_size;
	T*					m_array;
};

/**
	Constructor.
@note
	Does not allocate memory.
*/
template <typename T>
inline List<T>::List() :
	m_capacity(0),
	m_size(0),
	m_array(0)
{
}

/**
	Constructor.
@note
	Allocates capacity * sizeof(T) bytes.
*/
template <typename T>
inline List<T>::List(uint capacity) :
	m_capacity(0),
	m_size(0),
	m_array(0)
{
	set_capacity(capacity);
}

/**
	Copy constructor.
*/
template <typename T>
inline List<T>::List(const List<T>& list) :
	m_capacity(0),
	m_size(0),
	m_array(0)
{
	*this = list;
}

/**
	Destructor.
*/
template <typename T>
inline List<T>::~List()
{
	if (m_array)
	{
		delete[] m_array;
	}
}

/**
	Random access.
@note
	The index has to be smaller than size()
*/
template <typename T>
inline T& List<T>::operator[](uint index)
{
	assert(index < m_size);

	return m_array[index];
}

/**
	Random access.
@note
	The index has to be smaller than size()
*/
template <typename T>
inline const T& List<T>::operator[](uint index) const
{
	assert(index < m_size);

	return m_array[index];
}

/**
	Returns whether the array is empty.
*/
template <typename T>
inline bool List<T>::empty() const
{
	return m_size == 0;
}

/**
	Returns the number of items in the array.
*/
template <typename T>
inline uint List<T>::size() const
{
	return m_size;
}

/**
	Returns the maximum number of items the array can hold.
*/
template <typename T>
inline uint List<T>::capacity() const
{
	return m_capacity;
}

/**
	Resizes the array to the given capacity.
@note
	Old items will be copied to the newly created array.
	If the new capacity is smaller than the previous one, the
	previous array will be truncated.
*/
template <typename T>
inline void List<T>::set_capacity(uint capacity)
{
	assert(capacity > 0);

	if (m_capacity == capacity)
	{
		return;
	}

	T* tmp = m_array;
	m_capacity = capacity;

	if (capacity < m_size)
	{
		m_size = capacity;
	}

	m_array = new T[capacity];

	memcpy(m_array, tmp, sizeof(T) * m_size);

	if (tmp)
	{
		delete[] tmp;
	}
}

template <typename T>
inline void List<T>::grow()
{
	set_capacity(m_capacity * 2 + 16);
}

/**
	Condenses the array so the capacity matches the actual number
	of items in the array.
*/
template <typename T>
inline void List<T>::condense()
{
	set_capacity(m_size);
}

/**
	Appends an item to the array and returns its index or -1 if full.
*/
template <typename T>
inline uint List<T>::push_back(const T& item)
{
	if (m_capacity == m_size)
	{
		grow();
	}

	m_array[m_size] = item;

	return 	m_size++;
}

/**
	Removes the item at the given index.
*/
template <typename T>
inline void List<T>::pop_back()
{
	assert(m_size > 0);

	m_size--;
}

/**
	Clears the content of the array.
@note
	Does not free memory, it only zeroes
	the number of items in the array.
*/
template <typename T>
inline void List<T>::clear()
{
	m_size = 0;
}

/**
	Copies the content of the other list into this.
*/
template <typename T>
inline const List<T>& List<T>::operator=(const List<T>& other)
{
	if (m_array)
	{
		delete[] m_array;
	}

	m_size = other.m_size;
	m_capacity = other.m_capacity;

	if (m_capacity)
	{
		m_array = new T[m_capacity];

		memcpy(m_array, other.m_array, m_size);
	}

	return *this;
}

//-----------------------------------------------------------------------------
template <typename T>
inline const T* List<T>::begin() const
{
	return m_array;
}

//-----------------------------------------------------------------------------
template <typename T>
inline T* List<T>::begin()
{
	return m_array;
}

//-----------------------------------------------------------------------------
template <typename T>
inline const T* List<T>::end() const
{
	return m_array + (m_size - 1);
}

//-----------------------------------------------------------------------------
template <typename T>
inline T* List<T>::end()
{
	return m_array + (m_size - 1);
}

} // namespace crown

