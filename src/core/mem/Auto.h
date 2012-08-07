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

namespace Crown
{

template <typename T>
class Auto
{

public:

					Auto();
					Auto(T* obj);
					Auto(const Auto<T>& obj);
					~Auto();

	bool			IsNull() const;

	T&				operator*() const;
	T*				operator->() const;

	Auto<T>&		operator=(const Auto<T>& obj);
	Auto<T>&		operator=(T* obj);

	bool			operator==(const Auto<T>& obj) const;
	bool			operator!=(const Auto<T>& obj) const;

	T*				GetPointer();

	T*				GetPointer(bool disown);
	const T*		GetPointer() const;

private:

	T*				mPtr;
};

//-----------------------------------------------------------------------------
template <typename T>
inline Auto<T>::Auto() : mPtr(NULL)
{
}

//-----------------------------------------------------------------------------
template <typename T>
inline Auto<T>::Auto(T* obj) : mPtr(obj)
{
}

//-----------------------------------------------------------------------------
template <typename T>
inline Auto<T>::Auto(const Auto<T>& obj)
{
	mPtr = obj.mPtr;
	const_cast<Auto<T>& >(obj).mPtr = NULL;
}

//-----------------------------------------------------------------------------
template <typename T>
inline Auto<T>::~Auto()
{
	if (mPtr)
	{
		delete mPtr;
	}
}

//-----------------------------------------------------------------------------
template <typename T>
inline bool Auto<T>::IsNull() const
{
	return mPtr == NULL;
}

//-----------------------------------------------------------------------------
template <typename T>
inline T& Auto<T>::operator*() const
{
	assert(mPtr != NULL);

	return *mPtr;
}

//-----------------------------------------------------------------------------
template <typename T>
inline T* Auto<T>::operator->() const
{
	assert(mPtr != NULL);

	return mPtr;
}

//-----------------------------------------------------------------------------
template <typename T>
inline Auto<T>& Auto<T>::operator=(const Auto<T>& obj)
{
	if (mPtr != NULL)
	{
		delete mPtr;
	}

	mPtr = obj.mPtr;
	obj.mPtr = NULL;
	return *this;
}

//-----------------------------------------------------------------------------
template <typename T>
inline Auto<T>& Auto<T>::operator=(T* obj)
{
	if (mPtr != NULL)
	{
		delete mPtr;
	}

	mPtr = obj;
	return *this;
}

//-----------------------------------------------------------------------------
template <typename T>
inline bool Auto<T>::operator==(const Auto<T>& obj) const
{
	return (mPtr == obj.mPtr);
}

//-----------------------------------------------------------------------------
template <typename T>
inline bool Auto<T>::operator!=(const Auto<T>& obj) const
{
	return (mPtr != obj.mPtr);
}

//-----------------------------------------------------------------------------
template <typename T>
inline T* Auto<T>::GetPointer()
{
	return mPtr;
}

//-----------------------------------------------------------------------------
template <typename T>
inline T* Auto<T>::GetPointer(bool disown)
{
	T* ptr = mPtr;

	if (disown)
	{
		mPtr = NULL;
	}

	return ptr;
}

//-----------------------------------------------------------------------------
template <typename T>
inline const T* Auto<T>::GetPointer() const
{
	return mPtr;
}

} // namespace Crown

