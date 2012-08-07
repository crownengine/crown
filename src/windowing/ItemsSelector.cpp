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

#include "ItemsSelector.h"
#include "SpecificProperties.h"

namespace Crown
{

ItemsSelector::ItemsSelector(Widget* parent): 
	ItemsWidget(parent), mSelectedIndex(-1)
{
	AddProperty(new ActionProperty<ItemsSelector, SelectionChangedEventArgs*>("OnSelectionChanged", &OnSelectionChangedEvent, this));
}

ItemsSelector::~ItemsSelector()
{
	
}

Generic ItemsSelector::GetSelectedItem() const
{
	if (mSelectedIndex < 0)
	{
		return Generic();
	}
	else
	{
		return GetItems()->GetElement(mSelectedIndex);
	}
}

Generic ItemsSelector::GetPropertyValue(const Str& name) const
{
	if (name == "SelectedItem")
	{
		return GetSelectedItem();
	}
	else
	{
		return ItemsWidget::GetPropertyValue(name);
	}
}

void ItemsSelector::SetPropertyValue(const Str& name, const Generic& value)
{
	if (name == "SelectedItem")
	{
		Log::E("Property SelectedItem is read-only");
	}
	else
	{
		ItemsWidget::SetPropertyValue(name, value);
	}
}

void ItemsSelector::SetSelectedIndex(int value)
{
	if (mSelectedIndex != value)
	{
		int prevSelectedIndex = mSelectedIndex;
		mSelectedIndex = value;
		InternalSelectionChanged();

		SelectionChangedEventArgs args(this, prevSelectedIndex, mSelectedIndex);
		OnSelectionChangedEvent.Fire(this, &args);

		NotifyChangeEventArgs args1("SelectedIndex");
		NotifyChange(&args1);

		NotifyChangeEventArgs args2("SelectedItem");
		NotifyChange(&args2);
	}
}

}