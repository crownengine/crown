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
#include "List.h"
#include "Delegate.h"

namespace crown
{

/*
 * The MulticastMulticastEvent class represents an MulticastEvent. When it fires, it calls the handlers that have been added to it, passing some information arguments
 * Handlers must have the same signature of the MulticastEvent, and can be added to the MulticastEvent through the += operator
 */

template <typename TSource, typename TArg1>
class MulticastEvent
{
public:
	MulticastEvent();
	~MulticastEvent();

	typedef IDelegate2<void, TSource*, TArg1> HandlerType;

	void Fire(TSource* source, TArg1 arg1);

	//! Register handler delegate to be called when the event fires. This operation transfers ownership
	void operator+=(HandlerType* handler);

	//! Construct and register handler delegate to be called when the event fires.
	template <typename THandlerClass>
	void AddHandler(THandlerClass* obj, void (THandlerClass::*handlerMethod)(TSource*, TArg1))
	{
		HandlerType* handler = new Delegate2<THandlerClass, void, TSource*, TArg1>(obj, handlerMethod);
		*this += handler;
	}

private:
	List<HandlerType*> mHandlers;
};

/* -- MulticastEvent -- */

template <typename TSource, typename TArg1>
MulticastEvent<TSource, TArg1>::MulticastEvent()
{
}

template <typename TSource, typename TArg1>
MulticastEvent<TSource, TArg1>::~MulticastEvent()
{
	for (int i=0; i<mHandlers.GetSize(); i++)
	{
		delete mHandlers[i];
	}
}

template <typename TSource, typename TArg1>
void MulticastEvent<TSource, TArg1>::Fire(TSource* source, TArg1 arg1)
{
	for (int i=0; i<mHandlers.GetSize(); i++)
	{
		HandlerType* handler = mHandlers[i];
		handler->Invoke(source, arg1);
	}
}

template <typename TSource, typename TArg1>
void MulticastEvent<TSource, TArg1>::operator+=(HandlerType* handler)
{
	if (mHandlers.Find(handler) == -1)
		mHandlers.Append(handler);
}

} //namespace crown
