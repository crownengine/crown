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

namespace crown
{

class ReferenceCount
{

public:

					ReferenceCount();
					~ReferenceCount();

	bool			IsZero();

	void			Increase();
	void			Decrease();

	uint			GetReferenceCount();

private:

	void			AssertNotNull();

	uint*			mReferences;
};

//-----------------------------------------------------------------------------
inline ReferenceCount::ReferenceCount() : mReferences(NULL)
{
}

//-----------------------------------------------------------------------------
inline ReferenceCount::~ReferenceCount()
{
	if (mReferences != NULL && *mReferences == 0)
	{
		delete mReferences;
	}
}

//-----------------------------------------------------------------------------
inline bool ReferenceCount::IsZero()
{
	AssertNotNull();

	return *mReferences == 0;
}

//-----------------------------------------------------------------------------
inline void ReferenceCount::Increase()
{
	AssertNotNull();

	*mReferences += 1;
}

//-----------------------------------------------------------------------------
inline void ReferenceCount::Decrease()
{
	AssertNotNull();

	*mReferences -= 1;
}

//-----------------------------------------------------------------------------
inline uint ReferenceCount::GetReferenceCount()
{
	AssertNotNull();

	return *mReferences;
}

//-----------------------------------------------------------------------------
inline void ReferenceCount::AssertNotNull()
{
	if (mReferences == NULL)
	{
		mReferences = new uint;
		*mReferences = 0;
	}
}

/**
	Standard reference counted pointer.
*/
template <typename T>
class Shared
{

public:

					Shared();
					Shared(T* obj);
					Shared(Shared<T>& obj);
					~Shared();

	bool			IsNull() const;

	T&				operator*() const;
	T*				operator->() const;

	Shared<T>&		operator=(Shared<T>& obj);

	bool			operator==(const Shared<T>& obj) const;
	bool			operator!=(const Shared<T>& obj) const;

	T*				GetPointer();
	const T*		GetPointer() const;

	uint			GetReferenceCount();

private:

	T*				mPtr;
	ReferenceCount	mReferenceCount;
};

//-----------------------------------------------------------------------------
template <typename T>
inline Shared<T>::Shared() : mPtr(NULL)
{
}

//-----------------------------------------------------------------------------
template <typename T>
inline Shared<T>::Shared(T* obj) : mPtr(obj)
{
	if (mPtr != NULL)
	{
		mReferenceCount.Increase();
	}
}

//-----------------------------------------------------------------------------
template <typename T>
inline Shared<T>::Shared(Shared<T>& obj)
{
	mPtr = obj.mPtr;
	mReferenceCount = obj.mReferenceCount;

	if (mPtr != NULL)
	{
		obj.mReferenceCount.Increase();
	}
}

//-----------------------------------------------------------------------------
template <typename T>
inline Shared<T>::~Shared()
{
	if (mPtr != NULL)
	{
		mReferenceCount.Decrease();

		if (mReferenceCount.IsZero())
		{
			delete mPtr;
		}
	}
}

//-----------------------------------------------------------------------------
template <typename T>
inline bool Shared<T>::IsNull() const
{
	return mPtr == 0;
}

//-----------------------------------------------------------------------------
template <typename T>
inline T& Shared<T>::operator*() const
{
	assert(mPtr != NULL);

	return *mPtr;
}

//-----------------------------------------------------------------------------
template <typename T>
inline T* Shared<T>::operator->() const
{
	assert(mPtr != NULL);

	return mPtr;
}

//-----------------------------------------------------------------------------
template <typename T>
inline Shared<T>& Shared<T>::operator=(Shared<T>& obj)
{
	if (mPtr != NULL)
	{
		mReferenceCount.Decrease();

		if (mReferenceCount.IsZero())
		{
			delete mPtr;
		}
	}

	mPtr = obj.mPtr;
	mReferenceCount = obj.mReferenceCount;

	mReferenceCount.Increase();

	return *this;
}

//-----------------------------------------------------------------------------
template <typename T>
inline bool Shared<T>::operator==(const Shared<T>& obj) const
{
	return (mPtr == obj.mPtr);
}

//-----------------------------------------------------------------------------
template <typename T>
inline bool Shared<T>::operator!=(const Shared<T>& obj) const
{
	return (mPtr != obj.mPtr);
}

//-----------------------------------------------------------------------------
template <typename T>
inline T* Shared<T>::GetPointer()
{
	return mPtr;
}

//-----------------------------------------------------------------------------
template <typename T>
inline const T* Shared<T>::GetPointer() const
{
	return mPtr;
}

//-----------------------------------------------------------------------------
template <typename T>
inline uint Shared<T>::GetReferenceCount()
{
	return mReferenceCount.GetReferenceCount();
}

} // namespace crown

