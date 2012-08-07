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
#include "Exceptions.h"

namespace Crown
{

class WeakReferenced
{
public:
	WeakReferenced();
	virtual ~WeakReferenced();
};

class WeakAssociationsStorage
{
public:
	static void UnRegisterAllByObject(WeakReferenced* object);

private:
	WeakAssociationsStorage() {};
	~WeakAssociationsStorage() {};

	static void Register(WeakReferenced** ptr);
	static void UnRegister(WeakReferenced** ptr);

	template<typename T>
	friend class Weak;
};

//! A Weak pointer that is set to NULL when the poined object is deleted
//  T: Must inherit Object
template<typename T>
class Weak
{
public:
	Weak(): mPtr(NULL) { }
	Weak(WeakReferenced* obj): mPtr(obj)
	{
		Register();
	}

	Weak(const Weak<T>& obj)
	{
		mPtr = obj.mPtr;
		Register();
	}

	~Weak()
	{
		UnRegister();
	}

	inline bool IsNull() const
	{
		return mPtr == 0;
	}

	inline bool IsValid() const
	{
		return mPtr != 0;
	}

	inline T& operator*() const
	{
		if (mPtr == 0)
		{
			throw NullPointerException("Dereferencing null Weak<T> pointer");
		}

		return *((T*)mPtr);
	}

	inline T* operator->() const
	{
		if (mPtr == 0)
		{
			throw NullPointerException("Dereferencing null Weak<T> pointer");
		}

		return (T*)mPtr;
	}

	Weak<T>& operator=(const Weak<T>& obj)
	{
		if (mPtr != 0)
		{
			UnRegister();
		}

		mPtr = obj.mPtr;
		Register();
		return *this;
	}

	Weak<T>& operator=(WeakReferenced* obj)
	{
		UnRegister();
		mPtr = obj;
		Register();
		return *this;
	}

	inline bool operator==(const Weak<T>& obj) const
	{
		return (mPtr == obj.mPtr);
	}

	inline bool operator!=(const Weak<T>& obj) const
	{
		return (mPtr != obj.mPtr);
	}

	T* GetPointer()
	{
		return (T*)mPtr;
	}

	const T* GetPointer() const
	{
		return (const T*)mPtr;
	}

private:
	WeakReferenced* mPtr;

	inline void Register()
	{
		WeakAssociationsStorage::Register((WeakReferenced**)&mPtr);
	}

	inline void UnRegister()
	{
		WeakAssociationsStorage::UnRegister((WeakReferenced**)&mPtr);
	}
};

} // namespace Crown

