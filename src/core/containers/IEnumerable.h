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

#include <stdlib.h>
#include "Types.h"
#include "Exceptions.h"
#include "Generic.h"

namespace Crown
{

enum eEnumeratorState
{
	ES_Begin = 0,
	ES_Iterating,
	ES_End
};

template<typename T>
class IEnumerator
{
public:
	IEnumerator(eEnumeratorState state):
		mState(state)
	{ }

	virtual ~IEnumerator() {}

	//! Returns the current item.
	virtual const T& current() const = 0;

	//! Move the iterator to the next item.
	virtual bool next() = 0;

	//! Move the iterator to the previous item.
	virtual bool prev() = 0;

	//! Returns true if the enumerator is at the beginning
	inline bool isBegin()
	{
		return mState == ES_Begin;
	}

	//! Returns true if the enumerator is at the end
	inline bool isEnd()
	{
		return mState == ES_End;
	}

protected:
	eEnumeratorState mState;
};

template<class T>
class IEnumerable
{
public:

	//! Returns a IEnumerator<T> pointing to the begin of the structure.
	virtual IEnumerator<T>* getBegin() const = 0;

	//! Returns a IEnumerator<T> pointing to the end of the structure.
	virtual IEnumerator<T>* getEnd() const = 0;
};

class IGenericEnumerable
{
public:
	virtual IEnumerator<Generic>* getBeginGeneric() const = 0;
	virtual IEnumerator<Generic>* getEndGeneric() const = 0;
};

template<typename T>
class EnumeratorHelper
{
public:
	EnumeratorHelper(IEnumerator<T>* e):
		mEnumerator(e)
	{ }

	~EnumeratorHelper()
	{
		if (mEnumerator)
		{
			delete mEnumerator;
		}
	}

	//! See IEnumerator<T>
	inline const T& current()
	{
		if (mEnumerator == NULL)
		{
			throw NullPointerException("EnumeratorHelper<T>: No enumerator instance set");
		}

		return mEnumerator->current();
	}

	//! See IEnumerator<T>
	inline bool next()
	{
		if (mEnumerator == NULL)
		{
			throw NullPointerException("EnumeratorHelper<T>: No enumerator instance set");
		}

		return mEnumerator->next();
	}

	//! See IEnumerator<T>
	inline bool prev()
	{
		if (mEnumerator == NULL)
		{
			throw NullPointerException("EnumeratorHelper<T>: No enumerator instance set");
		}

		return mEnumerator->prev();
	}

	//! See IEnumerator<T>
	inline bool isBegin()
	{
		if (mEnumerator == NULL)
		{
			throw NullPointerException("EnumeratorHelper<T>: No enumerator instance set");
		}

		return mEnumerator->isBegin();
	}

	//! See IEnumerator<T>
	inline bool isEnd()
	{
		if (mEnumerator == NULL)
		{
			throw NullPointerException("EnumeratorHelper<T>: No enumerator instance set");
		}

		return mEnumerator->isEnd();
	}

	const EnumeratorHelper& operator=(EnumeratorHelper& other)
	{
		mEnumerator = other.mEnumerator;
		other.mEnumerator = NULL;
	}

	const EnumeratorHelper& operator=(const IEnumerator<T>* e)
	{
		if (mEnumerator)
		{
			delete mEnumerator;
		}

		mEnumerator = e;
	}

private:
	IEnumerator<T>* mEnumerator;
};

} // namespace Crown

