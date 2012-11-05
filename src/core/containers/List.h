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

	uint				push_back(const T& element);
	void				pop_back();
	void				clear();

	const List<T>&		operator=(const List<T>& other);

	T*					begin();
	const T*			begin() const;
	T*					end();
	const T*			end() const;

private:

	uint				mCapacity;
	uint				mSize;
	T*					mArray;
};

/**
	Constructor.
@note
	Does not allocate memory.
*/
template <typename T>
inline List<T>::List() :
	mCapacity(0),
	mSize(0),
	mArray(0)
{
}

/**
	Constructor.
@note
	Allocates capacity * sizeof(T) bytes.
*/
template <typename T>
inline List<T>::List(uint capacity) :
	mCapacity(0),
	mSize(0),
	mArray(0)
{
	set_capacity(capacity);
}

/**
	Copy constructor.
*/
template <typename T>
inline List<T>::List(const List<T>& list) :
	mCapacity(0),
	mSize(0),
	mArray(0)
{
	*this = list;
}

/**
	Destructor.
*/
template <typename T>
inline List<T>::~List()
{
	if (mArray)
	{
		delete[] mArray;
	}
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
template <typename T>
inline T& List<T>::operator[](uint index)
{
	assert(index < mSize);

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
template <typename T>
inline const T& List<T>::operator[](uint index) const
{
	assert(index < mSize);

	return mArray[index];
}

/**
	Returns whether the array is empty.
@return
	True if empty, false otherwise
*/
template <typename T>
inline bool List<T>::empty() const
{
	return mSize == 0;
}

/**
	Returns the number of elements in the array.
@return
	The number of elements
*/
template <typename T>
inline uint List<T>::size() const
{
	return mSize;
}

/**
	Returns the maximum number of elements the array can hold.
@return
	The maximum number of elements
*/
template <typename T>
inline uint List<T>::capacity() const
{
	return mCapacity;
}

/**
	Resizes the array to the given capacity.
@note
	Old elements will be copied to the newly created array.
	If the new capacity is smaller than the previous one, the
	previous array will be truncated.
@param capatity
	The new capacity
*/
template <typename T>
inline void List<T>::set_capacity(uint capacity)
{
	assert(capacity > 0);

	if (mCapacity == capacity)
	{
		return;
	}

	T* tmp = mArray;
	mCapacity = capacity;

	if (capacity < mSize)
	{
		mSize = capacity;
	}

	mArray = new T[capacity];

	memcpy(mArray, tmp, sizeof(T) * mSize);

	if (tmp)
	{
		delete[] tmp;
	}
}

template <typename T>
inline void List<T>::grow()
{
	set_capacity(mCapacity * 2 + 16);
}

/**
	Condenses the array so the capacity matches the actual number
	of elements in the array.
*/
template <typename T>
inline void List<T>::condense()
{
	set_capacity(mSize);
}

/**
	Appends an element to the array and returns its index or -1 if full.
@param element
	The element to append
@return
	The index of the element or -1 if full
*/
template <typename T>
inline uint List<T>::push_back(const T& element)
{
	if (mCapacity == mSize)
	{
		grow();
	}

	mArray[mSize] = element;

	return 	mSize++;
}

/**
	Removes the element at the given index.
@param index
	The index of the element to remove
*/
template <typename T>
inline void List<T>::pop_back()
{
	assert(mSize > 0);

	mSize--;
}

/**
	Clears the content of the array.
@note
	Does not free memory, it only zeroes
	the number of elements in the array.
*/
template <typename T>
inline void List<T>::clear()
{
	mSize = 0;
}

/**
	Copies the content of the other list into this.
@return
	The reference to list after copying
*/
template <typename T>
inline const List<T>& List<T>::operator=(const List<T>& other)
{
	if (mArray)
	{
		delete[] mArray;
	}

	mSize = other.mSize;
	mCapacity = other.mCapacity;

	if (mCapacity)
	{
		mArray = new T[mCapacity];

		memcpy(mArray, other.mArray, mSize);
	}

	return *this;
}

//-----------------------------------------------------------------------------
template <typename T>
inline const T* List<T>::begin() const
{
	return mArray;
}

//-----------------------------------------------------------------------------
template <typename T>
inline T* List<T>::begin()
{
	return mArray;
}

//-----------------------------------------------------------------------------
template <typename T>
inline const T* List<T>::end() const
{
	return mArray + (mSize - 1);
}

//-----------------------------------------------------------------------------
template <typename T>
inline T* List<T>::end()
{
	return mArray + (mSize - 1);
}

} // namespace crown

