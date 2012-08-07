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

#include "TreeView.h"
#include "Window.h"
#include "Types.h"
#include "Label.h"
#include "Themes.h"
#include "Device.h"
#include "ScrollArea.h"
#include "WithProperties.h"
#include "XWMLReader.h"
#include "Renderer.h"

namespace Crown
{

TreeViewItem::TreeViewItem(Widget* parent, TreeView* tree):
	ItemsSelector(parent), mInsideChild(false), mTreeView(tree), mIsExpanded(false), mExpandButton(NULL),
	mContentWidget(NULL)
{
	SetFitToChildren(FTC_VERTICALLY);

	StackLayout* sl = new StackLayout(this);

	StackLayout* sl1 = new StackLayout(sl);
	sl1->SetOrientation(SO_HORIZONTAL);

	mExpandButton = new Button(sl1);
	mExpandButton->SetDesiredSize(8, 8);
	mExpandButton->SetMargins(2, -1, 2, -1);

	mContentWidget = new Widget(sl1);

	mTreeViewItemsLayout = new StackLayout(sl);
	mTreeViewItemsLayout->SetVisibility(WV_COLLAPSED);
  
	mItemTemplate = new TreeViewDefaultTemplate(mTreeView);

	mExpandButton->OnClickEvent += CreateDelegate(this, &TreeViewItem::ExpendedButton_OnClick);
}

TreeViewItem::~TreeViewItem()
{

}

Widget* TreeViewItem::GetContentWidget()
{
	if (mContentWidget)
		return mContentWidget;
	return this;
}

void TreeViewItem::OnMouseIn(MouseEventArgs* args)
{
	mInsideChild = true;
	args->StopPropagation(true);
}

void TreeViewItem::OnMouseOut(MouseEventArgs* args)
{
	mInsideChild = false;
	args->StopPropagation(true);
}

void TreeViewItem::OnMouseUp(MouseButtonEventArgs* args)
{
	//mItemsSelector->SetSelectedIndex(mItemIndex);
	mTreeView->SelectItem(mItemsSelector, mItemIndex);
	args->StopPropagation(true);
}

void TreeViewItem::OnDraw(DrawingClipInfo& clipInfo)
{
	Widget::DrawInit(clipInfo);
	Renderer* r = GetDevice()->GetRenderer();
	//TODO: All this should be moved to state effects/styles in XWML files.
	int mode = DM_NO_DRAW;
	if (mTreeView->mSelectorWithSelection == mItemsSelector && mTreeView->mSelectedIndex == mItemIndex)
		mode |= DM_FILL;

	if (mInsideChild)
		mode |= DM_BORDER;

	r->DrawRectangle(mContentWidget->GetRelativePosition(this), mContentWidget->GetSize(), mode, Color4(0.2f, 0.4f, 1.0f), Color4(185, 213, 255));
	
	Widget::DrawChildren(clipInfo);
}

void TreeViewItem::ExpendedButton_OnClick(Button* /*obj*/, Crown::EventArgs* /*args*/)
{
	if (mIsExpanded)
		Collapse();
	else
		Expand();

	mIsExpanded = !mIsExpanded;
}

void TreeViewItem::Expand()
{
	if (mIsExpanded)
		return;

	mTreeViewItemsLayout->SetVisibility(WV_VISIBLE);
}

void TreeViewItem::Collapse()
{
	if (!mIsExpanded)
		return;

	mTreeViewItemsLayout->SetVisibility(WV_COLLAPSED);
}

void TreeViewItem::InternalSelectionChanged()
{

}

void TreeViewItem::InternalItemsChanged()
{
	RegenerateItemsWidgets();
}

void TreeViewItem::OnItemTemplateChanged()
{
	RegenerateItemsWidgets();
}

void TreeViewItem::RegenerateItemsWidgets()
{
	for (int i=0; i<mTreeViewItems.GetSize(); i++)
	{
		mTreeViewItems[i]->Destroy();
	}

	mTreeViewItems.Clear();

	if (GetItems() == NULL)
	{
		mExpandButton->SetVisibility(WV_HIDDEN);
	}
	else
	{
		if (GetItems()->GetSize() > 0)
			mExpandButton->SetVisibility(WV_VISIBLE);
		else
			mExpandButton->SetVisibility(WV_HIDDEN);

		for (int i=0; i<GetItems()->GetSize(); i++)
		{
			TreeViewItem* lwi = NULL;
		
			if (GetItemTemplate() != NULL)
			{
				IWithProperties* withProperties;
				if (GetItems()->GetElement(i).asType(&withProperties))
				{
					//It is an item that has properties, if a template is available use it!
					lwi = new TreeViewItem(mTreeViewItemsLayout, mTreeView);
					lwi->SetLogicalParent(this);
					lwi->ApplyTemplate(GetItemTemplate());
					lwi->SetBindingContext(withProperties);
				}
			}

			if (lwi == NULL)
			{
				//Not using template, generate default
				lwi = mItemTemplate->GenerateItem(GetItems()->GetElement(i));
				lwi->AttachToParent(mTreeViewItemsLayout);
			}

			//lwi->AttachToParent(mTreeViewItemsLayout);
			lwi->mItemsSelector = this;
			lwi->mItemIndex = i;
			lwi->SetMargins(10, 0, 0, 0);

			mTreeViewItems.Append(lwi);
		}
	}

	NotifyNeedsLayout();
}

TreeViewItem* TreeViewDefaultTemplate::GenerateItem(const Generic& element)
{
	TreeViewItem* lwi = new TreeViewItem(NULL, mTreeView);

	Label* label = new Label(lwi->GetContentWidget());
	Str str;
	element.asStr(str);
	label->SetText(str);
	label->SetMargins(2, 4, 0, 4);
	
	return lwi;
}

TreeView::TreeView(Widget* parent):
	ItemsSelector(parent), mSelectorWithSelection(NULL), mSelectedIndex(-1)
{
  
  ScrollArea* scroll = new ScrollArea(GetContentWidget());
	mTreeViewItemsLayout = new StackLayout(scroll->GetContentWidget());
	mTreeViewItemsLayout->SetMargins(1, 1, 0, 1);
  
	mItemTemplate = new TreeViewDefaultTemplate(this);
}

TreeView::~TreeView()
{
	delete mItemTemplate;
}

Generic TreeView::GetSelectedItem() const
{
	if (mSelectorWithSelection)
	{
		if (mSelectorWithSelection == this)
		{
			return ItemsSelector::GetSelectedItem();
		}
		else
		{
			return mSelectorWithSelection->GetSelectedItem();
		}
	}
	else
	{
		return Generic();
	}
}

void TreeView::SelectItem(ItemsSelector* selector, int index)
{
	if (mSelectorWithSelection)
		mSelectorWithSelection->SetSelectedIndex(-1);

	mSelectorWithSelection = selector;
	mSelectedIndex = index;
	selector->SetSelectedIndex(index);
	if (mSelectorWithSelection != this)
	{
		//If mSelectorWithSelection is not the TreeView, the NotifyChange of SelectedItem must be raised
		//if it is, SetSelectedIndex does the job
		NotifyChangeEventArgs args("SelectedItem");
		NotifyChange(&args);
	}
}

void TreeView::OnDraw(DrawingClipInfo& clipInfo)
{
	Widget::DrawInit(clipInfo);
	Themes::Draw(TF_LISTVIEW, GetSize());

	Widget::DrawChildren(clipInfo);
}

void TreeView::InternalSelectionChanged()
{
}

void TreeView::InternalItemsChanged()
{
	RegenerateItemsWidgets();
}

void TreeView::OnItemTemplateChanged()
{
	RegenerateItemsWidgets();
}

void TreeView::RegenerateItemsWidgets()
{
	Log::I("TreeView Regenerate");
	for (int i=0; i<mTreeViewItems.GetSize(); i++)
	{
		mTreeViewItems[i]->Destroy();
	}

	mTreeViewItems.Clear();

	if (GetItems() != NULL)
	{
		for (int i=0; i<GetItems()->GetSize(); i++)
		{
			TreeViewItem* lwi = NULL;
		
			if (GetItemTemplate() != NULL)
			{
				IWithProperties* withProperties;
				if (GetItems()->GetElement(i).asType(&withProperties))
				{
					//It is an item that has properties, if a template is available use it!
					lwi = new TreeViewItem(mTreeViewItemsLayout, this);
					lwi->SetLogicalParent(this);
					lwi->ApplyTemplate(GetItemTemplate());
					lwi->SetBindingContext(withProperties);
				}
			}

			if (lwi == NULL)
			{
				//Not using template, generate default
				lwi = mItemTemplate->GenerateItem(GetItems()->GetElement(i));
				lwi->AttachToParent(mTreeViewItemsLayout);
			}

			//lwi->AttachToParent(mTreeViewItemsLayout);
			lwi->mItemsSelector = this;
			lwi->mItemIndex = i;

			mTreeViewItems.Append(lwi);
		}
	}

	NotifyNeedsLayout();
}

}
