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
#include "ItemsWidget.h"
#include "WindowingEventArgs.h"
#include "SignalSlot.h"

namespace Crown
{

class ItemsSelector : public ItemsWidget
{
public:
	ItemsSelector(Widget* parent);
	virtual ~ItemsSelector();

	virtual Generic GetSelectedItem() const;

	inline int GetSelectedIndex() const
	{
		return mSelectedIndex;
	}
	virtual void SetSelectedIndex(int value);

	virtual Generic GetPropertyValue(const Str& name) const;
	virtual void SetPropertyValue(const Str& name, const Generic& value);

	MulticastEvent<ItemsSelector, SelectionChangedEventArgs*> OnSelectionChangedEvent;

	virtual Str ToStr() const
	 { return "ItemsSelector"; }

protected:
	virtual void InternalSelectionChanged() = 0;

private:
	int mSelectedIndex;
};

}
