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
#include "Delegate.h"

namespace Crown
{

class Point2;
template <typename T>
class List;
class Widget;
template <typename TKey, typename TValue>
class Dictionary;
//TODO: Move this declaration to somewhere more appropriate, It just doesn't seem right to declare this here
typedef Dictionary<Str, IDelegate2<void, Widget*, List<Str>*>*> ActionDictionary;
typedef IDelegate2<void, Widget*, List<Str>*> ActionDelegate;

enum PropertyType
{
	PT_Undefined = 0,
	/* -- Basic types -- */
	PT_Int,
	PT_UInt,
	PT_UShort,
	PT_Float,
	PT_Bool,

	/* -- Composite Types -- */
	PT_Point2,
	PT_Str,
	PT_Margins,
	PT_Template,
	PT_Action,
	PT_GenericList
};

class Property
{
public:
	Property(const Str& name);
	virtual ~Property() {}

	inline const Str& GetName()
	  { return mName; }

	virtual PropertyType GetType() const = 0;
	virtual Generic GetValue() const = 0;
	virtual void SetValue(const Generic& value) = 0;

private:
	Str mName;
};

class PropertyPipe: public Property
{
public:
	PropertyPipe(const Str& name, Property* prop);

	virtual PropertyType GetType() const;
	virtual Generic GetValue() const;
	virtual void SetValue(const Generic& value);
private:
	Property* mProperty;
};

struct Margins
{
	Margins(int l, int t, int r, int b)
	{
		left = l;
		top = t;
		right = r;
		bottom = b;
	}

	int left;
	int top;
	int right;
	int bottom;

	//! Calculate the fixed margins in the x direction.
	int GetFixedMarginsX() const
	{
		return (left > 0 ? left : 0) + (right > 0 ? right : 0);
	}

	//! Calculate the fixed margins in the y direction.
	int GetFixedMarginsY() const
	{
		return (top > 0 ? top : 0) + (bottom > 0 ? bottom : 0);
	}
};

}
