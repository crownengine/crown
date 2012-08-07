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
#include "Widget.h"
#include "Generic.h"

namespace Crown
{

class XMLNode;

//! A base class for all widgets that display a list of items
class ItemsWidget: public Widget
{
public:
	ItemsWidget(Widget* parent);

	virtual ~ItemsWidget();

	//! Returns the inner items list, if present.
	inline const List<Generic>* GetItems() const
	{
		return mItems;
	}

	//! Sets the items list to be used. Releases the previous one.
	//! The internal pointer is of Shared type: if callers need to
	//! reference the list, they must keep a reference counted pointer
	//! as well or delegate to the ItemsWidget instance
	inline void SetItems(List<Generic>* value)
	{
		mItems = value;
		InternalItemsChanged();
	}

	virtual void OnSetProperty(const Str& name);

	inline XMLNode* GetItemTemplate() const
	 { return mItemTemplate; }

	virtual Str ToStr() const
	 { return "ItemsWidget"; }

protected:
	virtual void InternalItemsChanged() = 0;
	virtual void OnItemTemplateChanged() = 0;

private:
	List<Generic>* mItems;
	XMLNode* mItemTemplate;
};

}
