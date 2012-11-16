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

#include "Property.h"
#include "WindowsManager.h"
#include "Auto.h"
#include "XWMLReader.h"
#include "WindowContext.h"

namespace Crown
{

class XMLNode;

class IntProperty: public Property
{
public:
	IntProperty(const Str& name, int32_t* valuePtr);

	PropertyType GetType() const;
	Generic GetValue() const;
	void SetValue(const Generic& value);

private:
	int32_t* mValuePtr;
};

class UIntProperty: public Property
{
public:
	UIntProperty(const Str& name, uint32_t* valuePtr);

	PropertyType GetType() const;
	Generic GetValue() const;
	void SetValue(const Generic& value);

private:
	uint32_t* mValuePtr;
};

class UShortProperty: public Property
{
public:
	UShortProperty(const Str& name, uint16_t* valuePtr);

	PropertyType GetType() const;
	Generic GetValue() const;
	void SetValue(const Generic& value);

private:
	uint16_t* mValuePtr;
};

class FloatProperty: public Property
{
public:
	FloatProperty(const Str& name, float* valuePtr);

	PropertyType GetType() const;
	Generic GetValue() const;
	void SetValue(const Generic& value);

private:
	float* mValuePtr;
};

class Point32_t2Property: public Property
{
public:
	Point32_t2Property(const Str& name, Point32_t2* valuePtr);

	PropertyType GetType() const;
	Generic GetValue() const;
	void SetValue(const Generic& value);

private:
	Point32_t2* mValuePtr;
};

class StrProperty: public Property
{
public:
	StrProperty(const Str& name, Str* valuePtr);

	PropertyType GetType() const;
	Generic GetValue() const;
	void SetValue(const Generic& value);

private:
	Str* mValuePtr;
};

class MarginsProperty: public Property
{
public:
	MarginsProperty(const Str& name, Margins* valuePtr);

	PropertyType GetType() const;
	Generic GetValue() const;
	void SetValue(const Generic& value);

private:
	Margins* mValuePtr;
};

class TemplateProperty: public Property
{
public:
	TemplateProperty(const Str& name, XMLNode** valuePtr);

	PropertyType GetType() const;
	Generic GetValue() const;
	void SetValue(const Generic& value);

private:
	XMLNode** mValuePtr;
};

class IWithPropertiesProperty: public Property
{
public:
	IWithPropertiesProperty(const Str& name, IWithProperties** valuePtr);

	PropertyType GetType() const;
	Generic GetValue() const;
	void SetValue(const Generic& value);

private:
	IWithProperties** mValuePtr;
};

//class GenericListProperty: public Property
//{
//public:
//	GenericListProperty(const Str& name, Shared<IList<Generic> >* valuePtr);

//	PropertyType GetType() const;
//	Generic GetValue() const;
//	void SetValue(const Generic& value);

//private:
//	Shared<IList<Generic> >* mValuePtr;
//};

class EnumProperty: public Property
{
public:
	EnumProperty(const Str& name, int32_t* valuePtr);

	PropertyType GetType() const;
	Generic GetValue() const;
	void SetValue(const Generic& value);

	void AddValueMapping(const Str& valueName, int32_t enumValue);

private:
	struct EnumPropertyPair
	{
		Str valueName;
		int32_t enumValue;

		bool operator==(const EnumPropertyPair& other)
		{
			return (valueName == other.valueName && enumValue == other.enumValue);
		}
	};

	int32_t* mValuePtr;
	List<EnumPropertyPair> mValuesMapping;
};

template <typename TSource, typename TArg1>
class ActionProperty: public Property, public WeakReferenced
{
private:
	//! Finds the specified action and register it to be executed when the given event fires
	bool FindActionAndAddToEvent(const Str& actionStr, MulticastEvent<TSource, TArg1>& multicastEvent);
	void GenericActionInvoker(TSource* button, TArg1 e, ActionDelegate* action, List<Str>* args);

public:
	ActionProperty(const Str& name, MulticastEvent<TSource, TArg1>* valuePtr, Widget* widget):
		Property(name), mValuePtr(valuePtr), mWidget(widget)
	{

	}

	PropertyType GetType() const
	{
		return PT_Action;
	}

	Generic GetValue() const
	{
		return mActionStr;
	}

	void SetValue(const Generic& value)
	{
		mActionStr = value.asStr();
		FindActionAndAddToEvent(mActionStr, *mValuePtr);
	}

private:
	MulticastEvent<TSource, TArg1>* mValuePtr;
	Widget* mWidget;
	Str mActionStr;
};

template <typename TSource, typename TArg1>
bool ActionProperty<TSource, TArg1>::FindActionAndAddToEvent(const Str& actionStr, MulticastEvent<TSource, TArg1>& multicastEvent)
{
	Window* wnd = mWidget->GetWindow();
	if (!wnd)
	{
		Log::E("Unable to find an Action in a widget not connected to a WindowsManager");
		return false;
	}

	Auto<List<Str> > args = new List<Str>();
	Str actionName;
	if (!XWMLReader::ParseAction(actionStr, actionName, args.GetPoint32_ter()))
	{
		Log::E("Action '" + actionStr + "' is in an invalid format");
		return false;
	}

	WindowContext* context = wnd->GetWindowContext();
	ActionDelegate* action = NULL;
	if (context != NULL)
	{
		action = context->GetRegisteredAction(actionName);
	}
	if (action == NULL)
	{
		action = wnd->GetWindowsManager()->GetRegisteredAction(actionName);
	}

	if (action == NULL)
	{
		Log::E("Action '" + actionName + "' is not registered in the current WindowContext or in the WindowsManager");
		return false;
	}

	multicastEvent += CreateDelegate(this, &ActionProperty::GenericActionInvoker, (TSource*)NULL, (TArg1)NULL, action, args.GetPoint32_ter(true));
	return true;
}

template <typename TSource, typename TArg1>
void ActionProperty<TSource, TArg1>::GenericActionInvoker(TSource* sourceWidget, TArg1 e, ActionDelegate* action, List<Str>* args)
{
	action->Invoke(sourceWidget, args);
}

}
