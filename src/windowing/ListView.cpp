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

#include "ListView.h"
#include "Window.h"
#include "Types.h"
#include "Label.h"
#include "Themes.h"
#include "Device.h"
#include "ScrollArea.h"
#include "Auto.h"
#include "XWMLReader.h"
#include "Renderer.h"

namespace Crown
{

bool DeserializeFromStr(List<Generic>& out, Str& input);

ListViewItem::ListViewItem(Widget* parent):
	Widget(parent), mInsideChild(false)
{
	SetFitToChildren(FTC_VERTICALLY);
}

ListViewItem::~ListViewItem()
{

}

void ListViewItem::OnPreviewMouseIn(MouseEventArgs* /*args*/)
{
	mInsideChild = true;
}

void ListViewItem::OnPreviewMouseOut(MouseEventArgs* /*args*/)
{
	mInsideChild = false;
}

void ListViewItem::OnMouseUp(MouseButtonEventArgs* /*args*/)
{
	mListView->SetSelectedIndex(mItemIndex);
}

void ListViewItem::OnDraw(DrawingClipInfo& clipInfo)
{
	Widget::DrawInit(clipInfo);
	Renderer* r = GetDevice()->GetRenderer();
	int32_t mode = DM_NO_DRAW;
	if (mListView->GetSelectedIndex() == mItemIndex)
		mode |= DM_FILL;

	if (mInsideChild)
		mode |= DM_BORDER;

	r->DrawRectangle(Point32_t2::ZERO, GetSize(), mode, Color4(0.2f, 0.4f, 1.0f), Color4(185, 213, 255));
	
	Widget::DrawChildren(clipInfo);
}

ListViewItem* ListViewDefaultTemplate::GenerateItem(const Generic& element)
{
	ListViewItem* lwi = new ListViewItem(NULL);

	Label* label = new Label(lwi->GetContentWidget());
	label->SetText(element.asStr());
	label->SetMargins(2, 2, 2, 2);
	
	return lwi;
}

ListView::ListView(Widget* parent):
	ItemsSelector(parent)
{
  ScrollArea* scroll = new ScrollArea(GetContentWidget());
	mListViewItemsLayout = new StackLayout(scroll->GetContentWidget());
	mListViewItemsLayout->SetMargins(1, 1, 0, 1);
  
	mItemTemplate = new ListViewDefaultTemplate();
}

ListView::~ListView()
{
	delete mItemTemplate;
}

void ListView::OnDraw(DrawingClipInfo& clipInfo)
{
	Widget::DrawInit(clipInfo);
	Themes::Draw(TF_LISTVIEW, GetSize());

	Widget::DrawChildren(clipInfo);
}

void ListView::RegenerateItemsWidgets()
{
	for (int32_t i=0; i<mListViewItems.GetSize(); i++)
	{
		mListViewItems[i]->Destroy();
	}

	mListViewItems.Clear();

	if (GetItems() != NULL)
	{
		for (int32_t i=0; i<GetItems()->GetSize(); i++)
		{
			ListViewItem* lwi = NULL;
		
			if (GetItemTemplate() != NULL)
			{
				IWithProperties* withProperties;
				if (GetItems()->GetElement(i).asType(&withProperties))
				{
					//It is an item that has properties, if a template is available use it!
					lwi = new ListViewItem(mListViewItemsLayout);
					lwi->SetLogicalParent(this);
					lwi->ApplyTemplate(GetItemTemplate());
					lwi->SetBindingContext(withProperties);
				}
			}

			if (lwi == NULL)
			{
				//Not using template, generate default
				lwi = mItemTemplate->GenerateItem(GetItems()->GetElement(i));
				lwi->AttachToParent(mListViewItemsLayout);
			}

			//lwi->AttachToParent(mListViewItemsLayout);
			lwi->mListView = this;
			lwi->mItemIndex = i;

			mListViewItems.Append(lwi);
		}
	}

	NotifyNeedsLayout();
}

void ListView::InternalSelectionChanged()
{
}

void ListView::InternalItemsChanged()
{
	RegenerateItemsWidgets();
}

void ListView::OnItemTemplateChanged()
{
	RegenerateItemsWidgets();
}

bool DeserializeFromStr(List<Generic>& out, Str& input)
{
	List<Str> items;

	if (input[0] != '[' || input[input.GetLength() - 1] != ']')
		return false;

	input = input.GetSubstring(1, input.GetLength() - 1);
	input.Split(',', items);

	for(int32_t i=0; i<items.GetSize(); i++)
		out.Append(items[i]);
	
	return true;
}

}
