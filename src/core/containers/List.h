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
#include "IEnumerable.h"
#include "Exceptions.h"
#include <cassert>

namespace Crown
{

template <typename T>
class ListEnumerator;

/**
	Dynamic array.
*/
template <typename T>
class List
{

public:

	typedef EnumeratorHelper<T> Enumerator;


						List();
						List(int capacity);
						List(const List<T>& list);
	virtual				~List();

	T&					operator[](int index);
	const T&			operator[](int index) const;

	bool				IsEmpty() const;
	int					GetSize() const;
	int					GetCapacity() const;
	void				SetCapacity(int capacity);

	void				Condense();

	int					Append(const T& element);
	int					Insert(int index, const T& element);
	void				Remove(int index);
	void				Clear();

	int					Find(const T& element) const;
	const T&			GetElement(int index) const;

	const List<T>&		operator=(const List<T>& other);

	IEnumerator<T>*		getBegin() const;							//!< See IEnumerable<T>
	IEnumerator<T>*		getEnd() const;								//!< See IEnumerable<T>

	inline const T*		GetData() const { return mArray; }

private:

	int					mCapacity;
	int					mSize;
	T*					mArray;
};

template <typename T>
class ListEnumerator: public IEnumerator<T>
{
public:
	ListEnumerator(const List<T>& l, eEnumeratorState state):
		IEnumerator<T>(state), mList(l), mCurrent(0)
	{ }

	//! Returns the current item.
	const T& current() const
	{
		if (mState != ES_Iterating) { } // Removes a plethora of warnings

		//  ; //Should throw, for now ignore it (could cause errors)
		return mList[mCurrent];
	}

	//! Move the iterator to the next item.
	bool next()
	{
		switch (mState)
		{
			case ES_Begin:

				if (mList.GetSize() == 0)
				{
					mState = ES_End;
					return false;
				}

				mCurrent = 0;
				mState = ES_Iterating;
				break;
			case ES_Iterating:
				mCurrent++;

				if (mCurrent == mList.GetSize())
				{
					mCurrent = mList.GetSize();
					mState = ES_End;
					return false;
				}

				break;
			case ES_End:
				return false;
				break;
		}

		return true;
	}

	//! Move the iterator to the previous item.
	bool prev()
	{
		switch (mState)
		{
			case ES_End:

				if (mList.GetSize() == 0)
				{
					mState = ES_Begin;
					return false;
				}

				mCurrent = mList.GetSize()-1;
				mState = ES_Iterating;
				break;
			case ES_Iterating:
				mCurrent--;

				if (mCurrent == -1)
				{
					mCurrent = 0;
					mState = ES_Begin;
					return false;
				}

				break;
			case ES_Begin:
				return false;
				break;
		}

		return true;
	}

private:

	const List<T>& mList;
	int mCurrent;

	using IEnumerator<T>::mState;
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
inline List<T>::List(int capacity) :
	mCapacity(0),
	mSize(0),
	mArray(0)
{
	SetCapacity(capacity);
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
	The index has to be smaller than GetSize()
@param index
	The index
@return
	The element at the given index
*/
template <typename T>
inline T& List<T>::operator[](int index)
{
	assert(index >= 0);
	assert(index < mSize);

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
template <typename T>
inline const T& List<T>::operator[](int index) const
{
	assert(index >= 0);
	assert(index < mSize);

	return mArray[index];
}

/**
	Returns whether the array is empty.
@return
	True if empty, false otherwise
*/
template <typename T>
bool List<T>::IsEmpty() const
{
	return mSize == 0;
}

/**
	Returns the number of elements in the array.
@return
	The number of elements
*/
template <typename T>
int List<T>::GetSize() const
{
	return mSize;
}

/**
	Returns the maximum number of elements the array can hold.
@return
	The maximum number of elements
*/
template <typename T>
inline int List<T>::GetCapacity() const
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
inline void List<T>::SetCapacity(int capacity)
{
	assert(capacity > 0);

	if (mCapacity == capacity)
	{
		return;
	}

	T* tmp;
	tmp = mArray;
	mCapacity = capacity;

	if (capacity < mSize)
	{
		mSize = capacity;
	}

	mArray = new T[capacity];
	for (int i = 0; i < mSize; i++)
	{
		mArray[i] = tmp[i];
	}

	if (tmp)
	{
		delete[] tmp;
	}
}

/**
	Condenses the array so the capacity matches the actual number
	of elements in the array.
*/
template <typename T>
inline void List<T>::Condense()
{
	SetCapacity(mSize);
}

/**
	Appends an element to the array and returns its index or -1 if full.
@param element
	The element to append
@return
	The index of the element or -1 if full
*/
template <typename T>
int List<T>::Append(const T& element)
{
	if (mCapacity == mSize)
	{
		SetCapacity(1 + (mCapacity * 2));
	}

	mArray[mSize] = element;

	return 	mSize++;
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
template <typename T>
int List<T>::Insert(int index, const T& element)
{
	assert(index >= 0);
	assert(index <= mSize);

	if (mCapacity == mSize)
	{
		SetCapacity(1 + (mCapacity * 2));
	}

	for (int i = mSize; i > index; i--)
	{
		mArray[i] = mArray[i-1];
	}

	mArray[index] = element;
	mSize++;

	return index;
}

/**
	Removes the element at the given index.
@param index
	The index of the element to remove
*/
template <typename T>
void List<T>::Remove(int index)
{
	assert(index >= 0);
	assert(index < mSize);

	mSize--;

	for (int i = index; i < mSize; i++)
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
template <typename T>
void List<T>::Clear()
{
	mSize = 0;
}

/**
	Returns the index of a given element or -1 if not found.
@return
	The index of the element of -1 if not found.
*/
template <typename T>
int List<T>::Find(const T& element) const
{
	for (int i = 0; i < mSize; i++)
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
template <typename T>
const T& List<T>::GetElement(int index) const
{
	assert(index >= 0);
	assert(index < mSize);

	return mArray[index];
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

		for (int i = 0; i < mSize; i++)
		{
			mArray[i] = other[i];
		}
	}

	return *this;
}

//-----------------------------------------------------------------------------
template <typename T>
IEnumerator<T>* List<T>::getBegin() const
{
	return new ListEnumerator<T>(*this, ES_Begin);
}

//-----------------------------------------------------------------------------
template <typename T>
IEnumerator<T>* List<T>::getEnd() const
{
	return new ListEnumerator<T>(*this, ES_End);
}

} // namespace Crown

