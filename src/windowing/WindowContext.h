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

#include "WithProperties.h"
#include "WindowsManager.h"

namespace Crown
{

class Converter;

typedef Dictionary<Str, Converter*> ConverterDictionary;

//! A class that serves as a base-class to define a context for windows, where actions are registered and take place
class WindowContext: public WithProperties
{
public:
	WindowContext(WindowsManager* windowsManager);
	virtual ~WindowContext();

	inline Window* GetAssociatedWindow() const
	 { return mAssociatedWindow; }

	inline WindowsManager* GetWindowsManager() const
	{ return mWindowsManager; }

	Widget* FindWidgetByName(const Str& name) const;

	virtual void OnLoad() = 0;

	void RegisterAction(const Str& name, ActionDelegate* slot);
	ActionDelegate* GetRegisteredAction(const Str& name);

	void RegisterConverter(const Str& name, Converter* converter);
	Converter* GetRegisteredConverter(const Str& name);

private:
	WindowsManager* mWindowsManager;
	Window* mAssociatedWindow;
	ActionDictionary mRegisteredActions;
	ConverterDictionary mRegisteredConverters;

	void SetAssociatedWindow(Window* associatedWindow);

	friend class XWMLReader;
};

}
