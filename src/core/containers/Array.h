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

#include <cassert>

/**
	Fixed-size array.

	Does not grow in size.
	Does not initialize memory.
*/
template <typename T, int size>
class Array
{

public:

							Array();
							Array(const Array<T, size>& array);
							~Array();

	T&						operator[](int index);
	const T&				operator[](int index) const;

	bool					IsEmpty() const;
	int						GetSize() const;
	int						GetCapacity() const;

	int						Append(const T& element);
	int						Insert(int index, const T& element);
	void					Remove(int index);
	void					Clear();

	int						Find(const T& element) const;
	const T&				GetElement(int index) const;

	const Array<T, size>&	operator=(const Array<T, size>& b);

private:

	int						mCount;			// The number of elements
	T						mArray[size];
};

/**
	Constructor.
@note
	Does not initialize array memory.
*/
template <typename T, int size>
inline Array<T, size>::Array()
{
	assert(size > 0);

	mCount = 0;

	// Do not initialize for efficiency
}

/**
	Copy constructor.
*/
template <typename T, int size>
inline Array<T, size>::Array(const Array<T, size>& array)
{
	*this = array;
}

/**
	Destructor.
*/
template <typename T, int size>
inline Array<T, size>::~Array()
{
}

/**
	Random access.
@note
	The index has to be smaller than GetSize()
@param index
	The index
@return
	The element at the given index
*/
template <typename T, int size>
inline T& Array<T, size>::operator[](int index)
{
	assert(index >= 0);
	assert(index < mCount);

	return mArray[index];
}

/**
	Random access.
@note
	The index has to be smaller than GetSize()
@param index
	The index
@return
	The element at the given index
*/
template <typename T, int size>
inline const T& Array<T, size>::operator[](int index) const
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
template <typename T, int size>
inline bool Array<T, size>::IsEmpty() const
{
	return mCount == 0;
}

/**
	Returns the number of elements in the array.
@return
	The number of elements
*/
template <typename T, int size>
inline int Array<T, size>::GetSize() const
{
	return mCount;
}

/**
	Returns the maximum number of elements the array can hold.
@return
	The maximum number of elements
*/
template <typename T, int size>
inline int Array<T, size>::GetCapacity() const
{
	return size;
}

/**
	Appends an element to the array and returns its index or -1 if full.
@param element
	The element to append
@return
	The index of the element or -1 if full
*/
template <typename T, int size>
inline int Array<T, size>::Append(const T& element)
{
	assert(mCount < size);

	if (mCount < size)
	{
		mArray[mCount] = element;

		return mCount++;
	}

	return -1;
}

/**
	Inserts an element in the array at a given index.
	Returns a value equal to index if success or -1 if full.
@note
	Insert operation involves data moving.
@param index
	The index where to insert the element
@param element
	The element
@return
	A value equal to index if success or -1 if full
*/
template <typename T, int size>
inline int Array<T, size>::Insert(int index, const T& element)
{
	assert(index >= 0);
	assert(index <= mCount);
	assert(mCount < size);

	if (mCount < size)
	{
		for (int i = mCount; i > index; i--)
		{
			mArray[i] = mArray[i-1];
		}

		mArray[index] = element;

		return mCount++;
	}

	return -1;
}

/**
	Removes the element at the given index.
@param index
	The index of the element to remove
*/
template <typename T, int size>
inline void Array<T, size>::Remove(int index)
{
	assert(index >= 0);
	assert(index < mCount);

	mCount--;

	for (int i = index; i < mCount; i++)
	{
		mArray[i] = mArray[i+1];
	}
}

/**
	Clears the content of the array.
@note
	Does not free memory, it only zeroes
	the number of elements in the array.
*/
template <typename T, int size>
inline void Array<T, size>::Clear()
{
	mCount = 0;
}

/**
	Returns the index of a given element or -1 if not found.
@return
	The index of the element of -1 if not found.
*/
template <typename T, int size>
inline int Array<T, size>::Find(const T& element) const
{
	for (int i = 0; i < mCount; i++)
	{
		if (mArray[i] == element)
		{
			return i;
		}
	}

	return -1;
}

/**
	Random access.
@note
	The index has to be smaller than GetSize()
@param index
	The index
@return
	The element at the given index
*/
template <typename T, int size>
inline const T& Array<T, size>::GetElement(int index) const
{
	assert(index >= 0);
	assert(index < mCount);

	return mArray[index];
}

/**
	Copies the content of the other list into this.
@return
	The reference to list after copying
*/
template <typename T, int size>
inline const Array<T, size>& Array<T, size>::operator=(const Array<T, size>& b)
{
	for (int i = 0; i < b.mCount; i++)
	{
		mArray[i] = b.mArray[i];
	}

	mCount = b.mCount;

	return *this;
}

