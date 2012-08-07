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

#include "WindowContext.h"
#include "Window.h"
#include "Bind.h"

namespace Crown
{

WindowContext::WindowContext(WindowsManager* windowsManager):
	mWindowsManager(windowsManager), mAssociatedWindow(NULL)
{
	assert(mWindowsManager);
}

WindowContext::~WindowContext()
{
	//TODO: Verify, should Actions be deleted too? Memory Leak!
	ConverterDictionary::Enumerator e = mRegisteredConverters.getBegin();
	while (e.next())
	{
		delete e.current().value;
	}
}

Widget* WindowContext::FindWidgetByName(const Str& name) const
{
	assert(mAssociatedWindow);
	return mAssociatedWindow->FindChildByName(name);
}

void WindowContext::SetAssociatedWindow(Window* associatedWindow)
{
	if (mAssociatedWindow != NULL)
	{
		Log::E("WindowContext can only be associated to a Window once and for all");
	}
	else
	{
		mAssociatedWindow = associatedWindow;
		mAssociatedWindow->mWindowContext = this;
		//Made the Binding Context default to the WindowContext of the window
		mAssociatedWindow->SetBindingContext(this);
	}
}

void WindowContext::RegisterAction(const Str& name, ActionDelegate* slot)
{
	if (mRegisteredActions.Contains(name))
	{
		Log::E("WindowContext already has an Action registered with name '" + name + "'");
	}
	else
	{
		mRegisteredActions[name] = slot;
	}
}

ActionDelegate* WindowContext::GetRegisteredAction(const Str& name)
{
	if (mRegisteredActions.Contains(name))
	{
		//Return a duplicate because the caller will usually pass the delegate to an event, which transfers ownership
		//TODO: Verify memory leak. Maybe Duplicate could be avoided
		return (ActionDelegate*)mRegisteredActions[name]->Duplicate();		
	}
	return NULL;
}

void WindowContext::RegisterConverter(const Str& name, Converter* converter)
{
	if (mRegisteredConverters.Contains(name))
	{
		Log::E("WindowContext already has a Converter registered with name '" + name + "'");
	}
	else
	{
		mRegisteredConverters[name] = converter;
	}
}

Converter* WindowContext::GetRegisteredConverter(const Str& name)
{
	if (mRegisteredConverters.Contains(name))
	{
		return mRegisteredConverters[name];
	}
	return NULL;
}

}