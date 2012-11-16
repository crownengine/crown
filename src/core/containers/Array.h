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
#include "Allocator.h"
#include <cassert>

namespace crown
{

/**
	Fixed-size array.
@note
	Does not grow in size.
	Does not call constructors/destructors so it is not very suitable for non-POD items.
*/
template <typename T, uint32_t SIZE>
class Array
{
public:

							Array(Allocator& allocator);
							Array(const Array<T, SIZE>& array);
							~Array();

	T&						operator[](uint32_t index);
	const T&				operator[](uint32_t index) const;

	bool					empty() const;
	uint32_t					size() const;
	uint32_t					get_capacity() const;

	uint32_t					push_back(const T& item);
	void					pop_back();

	void					clear();

	const Array<T, SIZE>&	operator=(const Array<T, SIZE>& b);

	T*						begin();
	const T*				begin() const;
	T*						end();
	const T*				end() const;

private:

	Allocator*				m_allocator;
	uint32_t					m_count;			// The number of items
	T						m_array[SIZE];
};

/**
	Constructor.
@note
	Does not initialize array memory.
*/
template <typename T, uint32_t SIZE>
inline Array<T, SIZE>::Array(Allocator& allocator) : m_allocator(&allocator), m_count(0)
{
	assert(SIZE > 0);

	// Do not initialize for efficiency
}

/**
	Copy constructor.
*/
template <typename T, uint32_t SIZE>
inline Array<T, SIZE>::Array(const Array<T, SIZE>& array)
{
	*this = array;
}

/**
	Destructor.
*/
template <typename T, uint32_t SIZE>
inline Array<T, SIZE>::~Array()
{
}

/**
	Random access.
@note
	The index has to be smaller than size()
*/
template <typename T, uint32_t SIZE>
inline T& Array<T, SIZE>::operator[](uint32_t index)
{
	assert(index >= 0);
	assert(index < m_count);

	return m_array[index];
}

/**
	Random access.
@note
	The index has to be smaller than size()
*/
template <typename T, uint32_t SIZE>
inline const T& Array<T, SIZE>::operator[](uint32_t index) const
{
	assert(index >= 0);
	assert(index < m_count);

	return m_array[index];
}

/**
	Returns whether the array is empty.
*/
template <typename T, uint32_t SIZE>
inline bool Array<T, SIZE>::empty() const
{
	return m_count == 0;
}

/**
	Returns the number of items in the array.
*/
template <typename T, uint32_t SIZE>
inline uint32_t Array<T, SIZE>::size() const
{
	return m_count;
}

/**
	Returns the maximum number of items the array can hold.
*/
template <typename T, uint32_t SIZE>
inline uint32_t Array<T, SIZE>::get_capacity() const
{
	return SIZE;
}

/**
	Appends an item to the array and returns its index.
*/
template <typename T, uint32_t SIZE>
inline uint32_t Array<T, SIZE>::push_back(const T& item)
{
	assert(m_count < SIZE);

	m_array[m_count] = item;

	return m_count++;
}

/**
	Removes the item at the given index.
*/
template <typename T, uint32_t SIZE>
inline void Array<T, SIZE>::pop_back()
{
	assert(m_count > 0);

	m_count--;
}

/**
	Clears the content of the array.
@note
	Does not free memory, it only zeroes
	the number of items in the array.
*/
template <typename T, uint32_t SIZE>
inline void Array<T, SIZE>::clear()
{
	m_count = 0;
}

/**
	Copies the content of other list into this.
*/
template <typename T, uint32_t SIZE>
inline const Array<T, SIZE>& Array<T, SIZE>::operator=(const Array<T, SIZE>& b)
{
	for (uint32_t i = 0; i < b.m_count; i++)
	{
		m_array[i] = b.m_array[i];
	}

	m_count = b.m_count;

	return *this;
}

//-----------------------------------------------------------------------------
template <typename T, uint32_t SIZE>
inline const T* Array<T, SIZE>::begin() const
{
	return m_array;
}

//-----------------------------------------------------------------------------
template <typename T, uint32_t SIZE>
inline T* Array<T, SIZE>::begin()
{
	return m_array;
}

//-----------------------------------------------------------------------------
template <typename T, uint32_t SIZE>
inline const T* Array<T, SIZE>::end() const
{
	return m_array + (m_count - 1);
}

//-----------------------------------------------------------------------------
template <typename T, uint32_t SIZE>
inline T* Array<T, SIZE>::end()
{
	return m_array + (m_count - 1);
}

} // namespace crown

