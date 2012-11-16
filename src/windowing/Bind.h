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

#include "Widget.h"

namespace Crown
{

class Converter
{
public:
	Converter();
	virtual ~Converter();

	virtual Generic Convert(Generic source);
};

enum BindFlags
{
	BF_NORMAL = 0,
	BF_DISABLED = 1,
	BF_USE_BINDING_CONTEXT = 2,
	BF_NAMED_WIDGET = 4,
	BF_DIRTY = 8
};

class Bind: public IObserver
{
public:
	Bind(IWithProperties* source, IWithProperties* destination, Str sourceProperty, Str destinationProperty, Converter* converter);
	//! Initialize the Bind with a Widget that acts as destination. The widget's BindingContext becomes the source
	Bind(Widget* widget, Str sourceProperty, Str destinationProperty, Converter* converter);
	//! Initializes the Bind with a named source to be searched in the destination widget's window descendants
	Bind(const Str& sourceName, Widget* destination, Str sourceProperty, Str destinationProperty, Converter* converter);
	~Bind();

	void Apply();

	void OnNotifyChange(Observable* obj, NotifyChangeEventArgs* args);

	void SetSource(IWithProperties* source);
	void SetSourceIfUsingBindingContext(IWithProperties* source);
	inline IWithProperties* GetSource()
	 { return mSource.GetPointer(); }

	inline void SetDestination(IWithProperties* destination)
	 { mDestination = destination; }
	inline IWithProperties* GetDestination()
	 { return mDestination.GetPointer(); }

	bool IsEnabled() const;
	void SetEnabled(bool value);

private:
	Weak<IWithProperties> mSource;
	Str mSourceName;
	Weak<IWithProperties> mDestination;
	Str mSourceProperty;
	Str mDestinationProperty;
	Converter* mConverter;
	uint16_t mFlags;
};

}
