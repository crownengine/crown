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
#include "Button.h"

namespace Crown
{

class TreeView;

class TreeViewItem: public ItemsSelector
{
public:
	TreeViewItem(Widget* parent, TreeView* tree);
	virtual ~TreeViewItem();

	virtual void OnDraw(DrawingClipInfo& clipInfo);
	virtual Widget* GetContentWidget();

	virtual Str ToStr() const
	 { return "TreeViewItem"; }

private:
	bool mInsideChild;
	ItemsSelector* mItemsSelector;
	TreeView* mTreeView;
	int mItemIndex;
	bool mIsExpanded;
	Button* mExpandButton;
	Widget* mContentWidget;
	
	StackLayout* mTreeViewItemsLayout;
	List<Widget*> mTreeViewItems;
	ITemplate<TreeViewItem>* mItemTemplate;

	virtual void OnMouseIn(MouseEventArgs* args);
	virtual void OnMouseOut(MouseEventArgs* args);
	virtual void OnMouseUp(MouseButtonEventArgs* args);

	void ExpendedButton_OnClick(Button* /*obj*/, Crown::EventArgs* /*args*/);

	void Expand();
	void Collapse();

	virtual void InternalSelectionChanged();
	virtual void InternalItemsChanged();
	virtual void OnItemTemplateChanged();

	void RegenerateItemsWidgets();

	friend class TreeView;
};

class TreeViewDefaultTemplate: public ITemplate<TreeViewItem>
{
public:
	TreeViewDefaultTemplate(TreeView* tw):
		mTreeView(tw)
	{

	}

	TreeViewItem* GenerateItem(const Generic& element);

private:
	TreeView* mTreeView;
};

class TreeView: public ItemsSelector
{
public:
	TreeView(Widget* parent);
	virtual ~TreeView();

	virtual Generic GetSelectedItem() const;

	void OnDraw(DrawingClipInfo& clipInfo);

	virtual Str ToStr() const
	 { return "TreeView"; }

private:
	StackLayout* mTreeViewItemsLayout;
	List<Widget*> mTreeViewItems;
	ITemplate<TreeViewItem>* mItemTemplate;
	ItemsSelector* mSelectorWithSelection;
	int mSelectedIndex;

	void SelectItem(ItemsSelector* selector, int index);

	virtual void InternalSelectionChanged();
	virtual void InternalItemsChanged();
	virtual void OnItemTemplateChanged();

	void RegenerateItemsWidgets();

	friend class TreeViewItem;
};

}
