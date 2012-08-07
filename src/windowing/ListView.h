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
#include "ItemsSelector.h"
#include "StackLayout.h"
#include "ITemplate.h"

namespace Crown
{

class ListView;
class XMLNode;

class ListViewItem: public Widget
{
public:
	ListViewItem(Widget* parent);
	virtual ~ListViewItem();

	virtual void OnDraw(DrawingClipInfo& clipInfo);

	virtual Str ToStr() const
	 { return "ListViewItem"; }

private:
	bool mInsideChild;
	ListView* mListView;
	int mItemIndex;

	virtual void OnPreviewMouseIn(MouseEventArgs* args);
	virtual void OnPreviewMouseOut(MouseEventArgs* args);
	virtual void OnMouseUp(MouseButtonEventArgs* args);

	friend class ListView;
};

class ListViewDefaultTemplate: public ITemplate<ListViewItem>
{
public:
	ListViewItem* GenerateItem(const Generic& element);
};

class ListView: public ItemsSelector
{
public:
	ListView(Widget* parent);
	virtual ~ListView();

	void OnDraw(DrawingClipInfo& clipInfo);

	virtual Str ToStr() const
	 { return "ListView"; }

private:
	StackLayout* mListViewItemsLayout;
	List<Widget*> mListViewItems;
	ITemplate<ListViewItem>* mItemTemplate;

	virtual void InternalSelectionChanged();
	virtual void InternalItemsChanged();
	virtual void OnItemTemplateChanged();

	void RegenerateItemsWidgets();
};

}
