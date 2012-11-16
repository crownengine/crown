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

#include "Generic.h"
#include "Str.h"
#include "Dictionary.h"
#include "Observable.h"
#include "Property.h"
#include "Log.h"
#include "Weak.h"

namespace crown
{

class StrSerializable
{
public:
	virtual bool DeserializeFromStr(Str& input) = 0;
	virtual Str SerializeToStr() const = 0;
};

class IWithProperties: public WeakReferenced
{
public:
	virtual Generic		GetPropertyValue(const Str& name) const = 0;
	virtual void		SetPropertyValue(const Str& name, const Generic& value) = 0;
};

class WithProperties: public Observable, public IWithProperties
{
public:
	WithProperties()
	{

	}
	
	virtual ~WithProperties()
	{
		for (int32_t i=0; i<mProperties.GetSize(); i++)
		{
			delete mProperties[i];
		}
	}

	//typedef Dictionary<Str, Generic> PropertiesDictionary;

	virtual Generic GetPropertyValue(const Str& name) const
	{
		// TODO: Due to Object deletion
//		if (name == "ToStr")
//		{
//			return ToStr();
//		}
		for(int32_t i = 0; i < mProperties.GetSize(); i++)
		{
			if (mProperties[i]->GetName() == name)
			{
				const_cast<WithProperties*>(this)->OnGetProperty(name);
				return mProperties[i]->GetValue();
			}
		}

		return Generic();
	}

	virtual void SetPropertyValue(const Str& name, const Generic& value)
	{
		for(int32_t i = 0; i < mProperties.GetSize(); i++)
		{
			if (mProperties[i]->GetName() == name)
			{
				mProperties[i]->SetValue(value);
				OnSetProperty(name);
				NotifyChangeEventArgs args(name);
				NotifyChange(&args);
				return;
			}
		}
		Log::I("Property '" + name + "' not found for this object");
	}

	virtual void OnSetProperty(const Str& /*name*/)
	{
	}

	virtual void OnGetProperty(const Str& /*name*/)
	{
	}

	void AddProperty(Property* prop)
	{
		mProperties.Append(prop);
	}

	Property* GetProperty(const Str& name)
	{
		for(int32_t i = 0; i < mProperties.GetSize(); i++)
		{
			if (mProperties[i]->GetName() == name)
			{
				return mProperties[i];
			}
		}
		return NULL;
	}

private:
	List<Property*> mProperties;
};

}

