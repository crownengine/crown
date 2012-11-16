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

#include "Bind.h"
#include "Window.h"

namespace Crown
{

Converter::Converter()
{

}

Converter::~Converter()
{

}

Generic Converter::Convert(Generic source)
{
	return source;
}

Bind::Bind(IWithProperties* source, IWithProperties* destination, Str sourceProperty, Str destinationProperty, Converter* converter):
	mSource(NULL), mDestination(destination), mSourceProperty(sourceProperty), mDestinationProperty(destinationProperty),
	mConverter(converter), mFlags(BF_NORMAL | BF_DIRTY)
{
	SetSource(source);
}

Bind::Bind(Widget* widget, Str sourceProperty, Str destinationProperty, Converter* converter):
	mSource(NULL), mDestination(widget), mSourceProperty(sourceProperty), mDestinationProperty(destinationProperty),
	mConverter(converter), mFlags(BF_USE_BINDING_CONTEXT | BF_DIRTY)
{
	SetSource(widget->GetBindingContext());
}

Bind::Bind(const Str& sourceName, Widget* destination, Str sourceProperty, Str destinationProperty, Converter* converter):
  mSource(NULL), mSourceName(sourceName), mDestination(destination), mSourceProperty(sourceProperty), mDestinationProperty(destinationProperty),
	mConverter(converter), mFlags(BF_NAMED_WIDGET | BF_DIRTY)
{
	//Source will be set the on the first Apply
}

Bind::~Bind()
{

}

void Bind::Apply()
{
	if (!IsEnabled() || !Math::TestBitmask(mFlags, BF_DIRTY))
	{
		return;
	}

	if (Math::TestBitmask(mFlags, BF_NAMED_WIDGET))
	{
		if (mSource.IsNull())
		{
			Window* w = ((Widget*)mDestination.GetPoint32_ter())->GetWindow();
			SetSource(w->FindChildByName(mSourceName));
		}
	}

	if (mSource.IsNull())
	{
		Log::E("No binding source set for this binding");
		return;
	}

	//if (mSourceProperty == "twCrwLibrary")
	//{
	//	int32_t a = 5;
	//}

	//Log::I("Bind::Apply: " + mSource->ToStr() + "." + mSourceProperty + " -> " + mDestination->ToStr() + "." + mDestinationProperty);

	//SB NOTE: Disable this Bind when Applying the value, to prevent Apply loops
	SetEnabled(false);
	if (mConverter)
	{
		mDestination->SetPropertyValue(mDestinationProperty, mConverter->Convert(mSource->GetPropertyValue(mSourceProperty)));
	}
	else
	{
		mDestination->SetPropertyValue(mDestinationProperty, mSource->GetPropertyValue(mSourceProperty));
	}
	SetEnabled(true);

	mFlags = Math::UnsetBitmask(mFlags, BF_DIRTY);
}

void Bind::SetSource(IWithProperties* source)
{
	//Log::I("Bind::SetSource: " + mSourceProperty + " -> " + mDestinationProperty);
	//if (mSourceProperty == "twCrwLibrary")
	//{
	//	int32_t a = 5;
	//}

	if (!IsEnabled())
	{
		return;
	}

	Observable* sourceAsObservable;
	if (mSource.IsValid())
	{
		//Remove the old observing connection if present
		sourceAsObservable = dynamic_cast<Observable*>(mSource.GetPoint32_ter());
		if (sourceAsObservable != NULL)
		{
			sourceAsObservable->RemoveObserver(this);
		}
	}

	//Try casting to Observable to see if we can bind to it
	mSource = source;

	sourceAsObservable = dynamic_cast<Observable*>(source);
	if (sourceAsObservable != NULL)
	{
		sourceAsObservable->AddObserver(this);
	}

	mFlags = Math::SetBitmask(mFlags, BF_DIRTY);
}

void Bind::SetSourceIfUsingBindingContext(IWithProperties* source)
{
	if (Math::TestBitmask(mFlags, BF_USE_BINDING_CONTEXT))
	{
		SetSource(source);
	}
}

void Bind::OnNotifyChange(Observable* obj, NotifyChangeEventArgs* args)
{
	//if (mSourceProperty == "twCrwLibrary")
	//{
	//	int32_t a = 5;
	//}

	if (args->GetPropertyName() == mSourceProperty)
	{
		//Apply();
		mFlags = Math::SetBitmask(mFlags, BF_DIRTY);
	}
}

bool Bind::IsEnabled() const
{
	return !Math::TestBitmask(mFlags, BF_DISABLED);
}

void Bind::SetEnabled(bool value)
{
	if (value)
	{
		mFlags = (uint16_t)Math::UnsetBitmask(mFlags, BF_DISABLED);
	}
	else
	{
		mFlags = (uint16_t)Math::SetBitmask(mFlags, BF_DISABLED);
	}
}

}