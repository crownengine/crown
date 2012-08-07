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

//#include "GarbageBin.h"
#include "Weak.h"
#include "GarbageBin.h"

namespace Crown
{

/* Delegate with 0 arguments */

template<typename TResult>
class IDelegate: public virtual IGarbageable
{
public:
	virtual TResult Invoke() = 0;
	virtual IDelegate<TResult>* Duplicate() = 0;
	virtual void SetCalledObject(void* object) = 0;
};

template<typename TClass, typename TResult>
class Delegate: public IDelegate<TResult>, public virtual IGarbageable
{
public:
	Delegate(TClass* object):
		mObject(object)
	{ }

	~Delegate()
	{ }

	void SetCalledObject(void* object)
	 { mObject = static_cast<TClass*>(object); }

	IDelegate<TResult>* Duplicate()
	 { return new Delegate(mObject.GetPointer()); }

protected:
	Weak<TClass> mObject;
};

/* Delegate with 1 argument */

template<typename TResult, typename TArg0>
class IDelegate1: public IDelegate<TResult>
{
public:
	virtual TResult Invoke(TArg0 arg0) = 0;
};

template<typename TClass, typename TResult, typename TArg0>
class Delegate1: public IDelegate1<TResult, TArg0>
{
public:
	typedef TResult (TClass::*MethodType)(TArg0 arg0);

	Delegate1(TClass* object, MethodType method):
	 mObject(object), mMethod(method), mDefaultArg0(TArg0())
	{ }

	Delegate1(TClass* object, MethodType method, TArg0 defaultArg0):
	 mObject(object), mMethod(method), mDefaultArg0(defaultArg0)
	{ }
	
	~Delegate1()
	{ }

	void SetCalledObject(void* object)
	 { mObject = static_cast<TClass*>(object); }

	TResult Invoke();
	TResult Invoke(TArg0 arg0);

	virtual IDelegate<TResult>* Duplicate()
	{
		return new Delegate1(mObject.GetPointer(), mMethod, mDefaultArg0);
	}

	TArg0 GetDefaultArg0() const;

protected:
	Weak<TClass> mObject;
	MethodType mMethod;
	TArg0 mDefaultArg0;
};

template<typename TClass, typename TResult, typename TArg0>
TResult Delegate1<TClass, TResult, TArg0>::Invoke()
{
	if (mObject.IsNull() || mMethod == NULL)
		return TResult();
	return (mObject.GetPointer()->*mMethod)(mDefaultArg0);
}

template<typename TClass, typename TResult, typename TArg0>
TResult Delegate1<TClass, TResult, TArg0>::Invoke(TArg0 arg0)
{
	if (mObject.IsNull() || mMethod == NULL)
		return TResult();
	return (mObject.GetPointer()->*mMethod)(arg0);
}

template<typename TClass, typename TResult, typename TArg0>
TArg0 Delegate1<TClass, TResult, TArg0>::GetDefaultArg0() const
{
	return mDefaultArg0;
}



/* Delegate with 2 arguments */

template<typename TResult, typename TArg0, typename TArg1>
class IDelegate2: public IDelegate1<TResult, TArg0>
{
public:
	virtual TResult Invoke(TArg0 arg0, TArg1 arg1) = 0;
};

template<typename TClass, typename TResult, typename TArg0, typename TArg1>
class Delegate2: public IDelegate2<TResult, TArg0, TArg1>
{
public:
	typedef TResult (TClass::*MethodType)(TArg0 arg0, TArg1 arg1);

	Delegate2(TClass* object, MethodType method):
	 mObject(object), mMethod(method), mDefaultArg0(TArg0()), mDefaultArg1(TArg1())
	{ }

	Delegate2(TClass* object, MethodType method, TArg0 defaultArg0, TArg1 defaultArg1):
	 mObject(object), mMethod(method), mDefaultArg0(defaultArg0), mDefaultArg1(defaultArg1)
	{ }

	~Delegate2()
	{ }

	void SetCalledObject(void* object)
	 { mObject = static_cast<TClass*>(object); }

	TResult Invoke();
	TResult Invoke(TArg0 arg0);
	TResult Invoke(TArg0 arg0, TArg1 arg1);

	virtual IDelegate<TResult>* Duplicate()
	{
		return new Delegate2(mObject.GetPointer(), mMethod, mDefaultArg0, mDefaultArg1);
	}

	TArg0 GetDefaultArg0() const;
	TArg1 GetDefaultArg1() const;

protected:
	Weak<TClass> mObject;
	MethodType mMethod;
	TArg0 mDefaultArg0;
	TArg1 mDefaultArg1;
};

template<typename TClass, typename TResult, typename TArg0, typename TArg1>
TResult Delegate2<TClass, TResult, TArg0, TArg1>::Invoke()
{
	if (mObject.IsNull() || mMethod == NULL)
		return TResult();
	return (mObject.GetPointer()->*mMethod)(mDefaultArg0, mDefaultArg1);
}

template<typename TClass, typename TResult, typename TArg0, typename TArg1>
TResult Delegate2<TClass, TResult, TArg0, TArg1>::Invoke(TArg0 arg0)
{
	if (mObject.IsNull() || mMethod == NULL)
		return TResult();
	return (mObject.GetPointer()->*mMethod)(arg0, mDefaultArg1);
}

template<typename TClass, typename TResult, typename TArg0, typename TArg1>
TResult Delegate2<TClass, TResult, TArg0, TArg1>::Invoke(TArg0 arg0, TArg1 arg1)
{
	if (mObject.IsNull() || mMethod == NULL)
		return TResult();
	return (mObject.GetPointer()->*mMethod)(arg0, arg1);
}

template<typename TClass, typename TResult, typename TArg0, typename TArg1>
TArg0 Delegate2<TClass, TResult, TArg0, TArg1>::GetDefaultArg0() const
{
	return mDefaultArg0;
}

template<typename TClass, typename TResult, typename TArg0, typename TArg1>
TArg1 Delegate2<TClass, TResult, TArg0, TArg1>::GetDefaultArg1() const
{
	return mDefaultArg1;
}

/* Delegate with 3 arguments */

template<typename TResult, typename TArg0, typename TArg1, typename TArg2>
class IDelegate3: public IDelegate2<TResult, TArg0, TArg1>
{
public:
	virtual TResult Invoke(TArg0 arg0, TArg1 arg1, TArg2 arg2) = 0;
};

template<typename TClass, typename TResult, typename TArg0, typename TArg1, typename TArg2>
class Delegate3: public IDelegate3<TResult, TArg0, TArg1, TArg2>
{
public:
	typedef TResult (TClass::*MethodType)(TArg0 arg0, TArg1 arg1, TArg2 arg2);

	Delegate3(TClass* object, MethodType method):
	 mObject(object), mMethod(method), mDefaultArg0(TArg0()), mDefaultArg1(TArg1()), mDefaultArg2(TArg2())
	{ }

	Delegate3(TClass* object, MethodType method, TArg0 defaultArg0, TArg1 defaultArg1, TArg2 defaultArg2):
	 mObject(object), mMethod(method), mDefaultArg0(defaultArg0), mDefaultArg1(defaultArg1), mDefaultArg2(defaultArg2)
	{ }

	~Delegate3()
	{ }

	void SetCalledObject(void* object)
	 { mObject = static_cast<TClass*>(object); }

	TResult Invoke();
	TResult Invoke(TArg0 arg0);
	TResult Invoke(TArg0 arg0, TArg1 arg1);
	TResult Invoke(TArg0 arg0, TArg1 arg1, TArg2 arg2);

	virtual IDelegate<TResult>* Duplicate()
	{
		return new Delegate3(mObject.GetPointer(), mMethod, mDefaultArg0, mDefaultArg1, mDefaultArg2);
	}

	TArg0 GetDefaultArg0() const;
	TArg1 GetDefaultArg1() const;
	TArg2 GetDefaultArg2() const;

protected:
	Weak<TClass> mObject;
	MethodType mMethod;
	TArg0 mDefaultArg0;
	TArg1 mDefaultArg1;
	TArg2 mDefaultArg2;
};

template<typename TClass, typename TResult, typename TArg0, typename TArg1, typename TArg2>
TResult Delegate3<TClass, TResult, TArg0, TArg1, TArg2>::Invoke()
{
	if (mObject.IsNull() || mMethod == NULL)
		return TResult();
	return (mObject.GetPointer()->*mMethod)(mDefaultArg0, mDefaultArg1, mDefaultArg2);
}

template<typename TClass, typename TResult, typename TArg0, typename TArg1, typename TArg2>
TResult Delegate3<TClass, TResult, TArg0, TArg1, TArg2>::Invoke(TArg0 arg0)
{
	if (mObject.IsNull() || mMethod == NULL)
		return TResult();
	return (mObject.GetPointer()->*mMethod)(arg0, mDefaultArg1, mDefaultArg2);
}

template<typename TClass, typename TResult, typename TArg0, typename TArg1, typename TArg2>
TResult Delegate3<TClass, TResult, TArg0, TArg1, TArg2>::Invoke(TArg0 arg0, TArg1 arg1)
{
	if (mObject.IsNull() || mMethod == NULL)
		return TResult();
	return (mObject.GetPointer()->*mMethod)(arg0, arg1, mDefaultArg2);
}

template<typename TClass, typename TResult, typename TArg0, typename TArg1, typename TArg2>
TResult Delegate3<TClass, TResult, TArg0, TArg1, TArg2>::Invoke(TArg0 arg0, TArg1 arg1, TArg2 arg2)
{
	if (mObject.IsNull() || mMethod == NULL)
		return TResult();
	return (mObject.GetPointer()->*mMethod)(arg0, arg1, arg2);
}

template<typename TClass, typename TResult, typename TArg0, typename TArg1, typename TArg2>
TArg0 Delegate3<TClass, TResult, TArg0, TArg1, TArg2>::GetDefaultArg0() const
{
	return mDefaultArg0;
}

template<typename TClass, typename TResult, typename TArg0, typename TArg1, typename TArg2>
TArg1 Delegate3<TClass, TResult, TArg0, TArg1, TArg2>::GetDefaultArg1() const
{
	return mDefaultArg1;
}

template<typename TClass, typename TResult, typename TArg0, typename TArg1, typename TArg2>
TArg2 Delegate3<TClass, TResult, TArg0, TArg1, TArg2>::GetDefaultArg2() const
{
	return mDefaultArg2;
}

/* Delegate with 4 arguments */

template<typename TResult, typename TArg0, typename TArg1, typename TArg2, typename TArg3>
class IDelegate4: public IDelegate3<TResult, TArg0, TArg1, TArg2>
{
public:
	virtual TResult Invoke(TArg0 arg0, TArg1 arg1, TArg2 arg2, TArg3 arg3) = 0;
};

template<typename TClass, typename TResult, typename TArg0, typename TArg1, typename TArg2, typename TArg3>
class Delegate4: public IDelegate4<TResult, TArg0, TArg1, TArg2, TArg3>
{
public:
	typedef TResult (TClass::*MethodType)(TArg0 arg0, TArg1 arg1, TArg2 arg2, TArg3 arg3);

	Delegate4(TClass* object, MethodType method):
	 mObject(object), mMethod(method), mDefaultArg0(TArg0()), mDefaultArg1(TArg1()), mDefaultArg2(TArg2()), mDefaultArg3(TArg3())
	{ }

	Delegate4(TClass* object, MethodType method, TArg0 defaultArg0, TArg1 defaultArg1, TArg2 defaultArg2, TArg3 defaultArg3):
	 mObject(object), mMethod(method), mDefaultArg0(defaultArg0), mDefaultArg1(defaultArg1), mDefaultArg2(defaultArg2), mDefaultArg3(defaultArg3)
	{ }

	~Delegate4()
	{ }

	void SetCalledObject(void* object)
	 { mObject = static_cast<TClass*>(object); }

	TResult Invoke();
	TResult Invoke(TArg0 arg0);
	TResult Invoke(TArg0 arg0, TArg1 arg1);
	TResult Invoke(TArg0 arg0, TArg1 arg1, TArg2 arg2);
	TResult Invoke(TArg0 arg0, TArg1 arg1, TArg2 arg2, TArg3 arg3);

	virtual IDelegate<TResult>* Duplicate()
	{
		return new Delegate4(mObject.GetPointer(), mMethod, mDefaultArg0, mDefaultArg1, mDefaultArg2, mDefaultArg3);
	}

	TArg0 GetDefaultArg0() const;
	TArg1 GetDefaultArg1() const;
	TArg2 GetDefaultArg2() const;
	TArg3 GetDefaultArg3() const;

protected:
	Weak<TClass> mObject;
	MethodType mMethod;
	TArg0 mDefaultArg0;
	TArg1 mDefaultArg1;
	TArg2 mDefaultArg2;
	TArg3 mDefaultArg3;
};

template<typename TClass, typename TResult, typename TArg0, typename TArg1, typename TArg2, typename TArg3>
TResult Delegate4<TClass, TResult, TArg0, TArg1, TArg2, TArg3>::Invoke()
{
	if (mObject.IsNull() || mMethod == NULL)
		return TResult();
	return (mObject.GetPointer()->*mMethod)(mDefaultArg0, mDefaultArg1, mDefaultArg2, mDefaultArg3);
}

template<typename TClass, typename TResult, typename TArg0, typename TArg1, typename TArg2, typename TArg3>
TResult Delegate4<TClass, TResult, TArg0, TArg1, TArg2, TArg3>::Invoke(TArg0 arg0)
{
	if (mObject.IsNull() || mMethod == NULL)
		return TResult();
	return (mObject.GetPointer()->*mMethod)(arg0, mDefaultArg1, mDefaultArg2, mDefaultArg3);
}

template<typename TClass, typename TResult, typename TArg0, typename TArg1, typename TArg2, typename TArg3>
TResult Delegate4<TClass, TResult, TArg0, TArg1, TArg2, TArg3>::Invoke(TArg0 arg0, TArg1 arg1)
{
	if (mObject.IsNull() || mMethod == NULL)
		return TResult();
	return (mObject.GetPointer()->*mMethod)(arg0, arg1, mDefaultArg2, mDefaultArg3);
}

template<typename TClass, typename TResult, typename TArg0, typename TArg1, typename TArg2, typename TArg3>
TResult Delegate4<TClass, TResult, TArg0, TArg1, TArg2, TArg3>::Invoke(TArg0 arg0, TArg1 arg1, TArg2 arg2)
{
	if (mObject.IsNull() || mMethod == NULL)
		return TResult();
	return (mObject.GetPointer()->*mMethod)(arg0, arg1, arg2, mDefaultArg3);
}

template<typename TClass, typename TResult, typename TArg0, typename TArg1, typename TArg2, typename TArg3>
TResult Delegate4<TClass, TResult, TArg0, TArg1, TArg2, TArg3>::Invoke(TArg0 arg0, TArg1 arg1, TArg2 arg2, TArg3 arg3)
{
	if (mObject.IsNull() || mMethod == NULL)
		return TResult();
	return (mObject.GetPointer()->*mMethod)(arg0, arg1, arg2, arg3);
}

template<typename TClass, typename TResult, typename TArg0, typename TArg1, typename TArg2, typename TArg3>
TArg0 Delegate4<TClass, TResult, TArg0, TArg1, TArg2, TArg3>::GetDefaultArg0() const
{
	return mDefaultArg0;
}

template<typename TClass, typename TResult, typename TArg0, typename TArg1, typename TArg2, typename TArg3>
TArg1 Delegate4<TClass, TResult, TArg0, TArg1, TArg2, TArg3>::GetDefaultArg1() const
{
	return mDefaultArg1;
}

template<typename TClass, typename TResult, typename TArg0, typename TArg1, typename TArg2, typename TArg3>
TArg2 Delegate4<TClass, TResult, TArg0, TArg1, TArg2, TArg3>::GetDefaultArg2() const
{
	return mDefaultArg2;
}

template<typename TClass, typename TResult, typename TArg0, typename TArg1, typename TArg2, typename TArg3>
TArg3 Delegate4<TClass, TResult, TArg0, TArg1, TArg2, TArg3>::GetDefaultArg3() const
{
	return mDefaultArg3;
}

/* 
 * Delegate Helper functions
 */

template <typename TClass, typename TResult, typename TArg0>
static IDelegate1<TResult, TArg0>* CreateDelegate(TClass* obj, TResult (TClass::*method)(TArg0))
{
	return new Delegate1<TClass, TResult, TArg0>(obj, method);
}

template <typename TClass, typename TResult, typename TArg0>
static IDelegate1<TResult, TArg0>* CreateDelegate(TClass* obj, TResult (TClass::*method)(TArg0), TArg0 arg0)
{
	return new Delegate1<TClass, TResult, TArg0>(obj, method, arg0);
}

template <typename TClass, typename TResult, typename TArg0, typename TArg1>
static IDelegate2<TResult, TArg0, TArg1>* CreateDelegate(TClass* obj, TResult (TClass::*method)(TArg0, TArg1))
{
	return new Delegate2<TClass, TResult, TArg0, TArg1>(obj, method);
}

template <typename TClass, typename TResult, typename TArg0, typename TArg1>
static IDelegate2<TResult, TArg0, TArg1>* CreateDelegate(TClass* obj, TResult (TClass::*method)(TArg0, TArg1), TArg0 arg0, TArg1 arg1)
{
	return new Delegate2<TClass, TResult, TArg0, TArg1>(obj, method, arg0, arg1);
}

template <typename TClass, typename TResult, typename TArg0, typename TArg1, typename TArg2>
static IDelegate3<TResult, TArg0, TArg1, TArg2>* CreateDelegate(TClass* obj, TResult (TClass::*method)(TArg0, TArg1, TArg2))
{
	return new Delegate3<TClass, TResult, TArg0, TArg1, TArg2>(obj, method);
}

template <typename TClass, typename TResult, typename TArg0, typename TArg1, typename TArg2>
static IDelegate3<TResult, TArg0, TArg1, TArg2>* CreateDelegate(TClass* obj, TResult (TClass::*method)(TArg0, TArg1, TArg2), TArg0 arg0, TArg1 arg1, TArg2 arg2)
{
	return new Delegate3<TClass, TResult, TArg0, TArg1, TArg2>(obj, method, arg0, arg1, arg2);
}

template <typename TClass, typename TResult, typename TArg0, typename TArg1, typename TArg2, typename TArg3>
static IDelegate4<TResult, TArg0, TArg1, TArg2, TArg3>* CreateDelegate(TClass* obj, TResult (TClass::*method)(TArg0, TArg1, TArg2, TArg3))
{
	return new Delegate4<TClass, TResult, TArg0, TArg1, TArg2, TArg3>(obj, method);
}

template <typename TClass, typename TResult, typename TArg0, typename TArg1, typename TArg2, typename TArg3>
static IDelegate4<TResult, TArg0, TArg1, TArg2, TArg3>* CreateDelegate(TClass* obj, TResult (TClass::*method)(TArg0, TArg1, TArg2, TArg3), TArg0 arg0, TArg1 arg1, TArg2 arg2, TArg3 arg3)
{
	return new Delegate4<TClass, TResult, TArg0, TArg1, TArg2, TArg3>(obj, method, arg0, arg1, arg2, arg3);
}


} //namespace Crown
