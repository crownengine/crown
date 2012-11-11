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

/**
	Fixed-size array.

	Does not grow in size.
	Does not initialize memory.
*/
template <typename T, uint SIZE>
class Array
{
public:

							Array();
							Array(const Array<T, SIZE>& array);
							~Array();

	T&						operator[](uint index);
	const T&				operator[](uint index) const;

	bool					empty() const;
	uint					size() const;
	uint					get_capacity() const;

	uint					push_back(const T& element);
	void					pop_back();

	void					clear();

	const Array<T, SIZE>&	operator=(const Array<T, SIZE>& b);

	T*						begin();
	const T*				begin() const;
	T*						end();
	const T*				end() const;

private:

	uint					mCount;			// The number of elements
	T						mArray[SIZE];
};

/**
	Constructor.
@note
	Does not initialize array memory.
*/
template <typename T, uint SIZE>
inline Array<T, SIZE>::Array()
{
	assert(SIZE > 0);

	mCount = 0;

	// Do not initialize for efficiency
}

/**
	Copy constructor.
*/
template <typename T, uint SIZE>
inline Array<T, SIZE>::Array(const Array<T, SIZE>& array)
{
	*this = array;
}

/**
	Destructor.
*/
template <typename T, uint SIZE>
inline Array<T, SIZE>::~Array()
{
}

/**
	Random access.
@note
	The index has to be smaller than size()
@param index
	The index
@return
	The element at the given index
*/
template <typename T, uint SIZE>
inline T& Array<T, SIZE>::operator[](uint index)
{
	assert(index >= 0);
	assert(index < mCount);

	return mArray[index];
}

/**
	Random access.
@note
	The index has to be smaller than size()
@param index
	The index
@return
	The element at the given index
*/
template <typename T, uint SIZE>
inline const T& Array<T, SIZE>::operator[](uint index) const
{
	assert(index >= 0);
	assert(index < mCount);

	return mArray[index];
}

/**
	Returns whether the array is empty.
@return
	True if empty, false otherwise
*/
template <typename T, uint SIZE>
inline bool Array<T, SIZE>::empty() const
{
	return mCount == 0;
}

/**
	Returns the number of elements in the array.
@return
	The number of elements
*/
template <typename T, uint SIZE>
inline uint Array<T, SIZE>::size() const
{
	return mCount;
}

/**
	Returns the maximum number of elements the array can hold.
@return
	The maximum number of elements
*/
template <typename T, uint SIZE>
inline uint Array<T, SIZE>::get_capacity() const
{
	return SIZE;
}

/**
	Appends an element to the array and returns its index or -1 if full.
@param element
	The element to append
@return
	The index of the element or -1 if full
*/
template <typename T, uint SIZE>
inline uint Array<T, SIZE>::push_back(const T& element)
{
	assert(mCount < SIZE);

	mArray[mCount] = element;

	return mCount++;
}

/**
	Removes the element at the given index.
@param index
	The index of the element to remove
*/
template <typename T, uint SIZE>
inline void Array<T, SIZE>::pop_back()
{
	assert(mCount > 0);

	mCount--;
}

/**
	Clears the content of the array.
@note
	Does not free memory, it only zeroes
	the number of elements in the array.
*/
template <typename T, uint SIZE>
inline void Array<T, SIZE>::clear()
{
	mCount = 0;
}

/**
	Copies the content of the other list into this.
@return
	The reference to list after copying
*/
template <typename T, uint SIZE>
inline const Array<T, SIZE>& Array<T, SIZE>::operator=(const Array<T, SIZE>& b)
{
	for (uint i = 0; i < b.mCount; i++)
	{
		mArray[i] = b.mArray[i];
	}

	mCount = b.mCount;

	return *this;
}

//-----------------------------------------------------------------------------
template <typename T, uint SIZE>
inline const T* Array<T, SIZE>::begin() const
{
	return mArray;
}

//-----------------------------------------------------------------------------
template <typename T, uint SIZE>
inline T* Array<T, SIZE>::begin()
{
	return mArray;
}

//-----------------------------------------------------------------------------
template <typename T, uint SIZE>
inline const T* Array<T, SIZE>::end() const
{
	return mArray + (mCount - 1);
}

//-----------------------------------------------------------------------------
template <typename T, uint SIZE>
inline T* Array<T, SIZE>::end()
{
	return mArray + (mCount - 1);
}

