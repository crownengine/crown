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

#include "Widget.h"
#include <GL/glew.h>
#include <stdio.h>
#include "Types.h"
#include "WindowsManager.h"
#include "Device.h"
#include <limits>
#include "Bind.h"
#include "SpecificProperties.h"
#include "XWMLReader.h"
#include "Renderer.h"

using namespace std;

namespace Crown
{

bool Widget::mDrawDebugBorder = false;

Widget::Widget(Widget* parent):
	mDesiredPosition(0, 0), mDesiredSize(-1, -1),
	mMaximumSize(numeric_limits<int>::max(), numeric_limits<int>::max()),	mMinimumSize(0, 0),
	mPosition(0, 0), mSize(0, 0), mMargins(-1, -1, -1, -1), mParent(parent), mLogicalParent(parent), mVisibility(WV_VISIBLE),
	mBindingContext(NULL), mIsMouseOver(false), mIsMouseSensible(true), mFitToChildren(FTC_NONE)
{
	if (mParent)
	{
		mParent->AddChild(this);
	}

	//By using specific properties such as IntProperty, we define how the Generic value should be interpreted for each property
	//TODO: Use converters where necessary
	AddProperty(new IntProperty("Width", &mDesiredSize.x));
	AddProperty(new IntProperty("Height", &mDesiredSize.y));
	AddProperty(new IntProperty("MinimumWidth", &mMinimumSize.x));
	AddProperty(new IntProperty("MinimumHeight", &mMinimumSize.y));
	AddProperty(new IntProperty("MaximumWidth", &mMaximumSize.x));
	AddProperty(new IntProperty("MaximumHeight", &mMaximumSize.y));
	AddProperty(new Point2Property("DesiredSize", &mDesiredSize));
	AddProperty(new Point2Property("MinimumSize", &mMinimumSize));
	AddProperty(new Point2Property("MaximumSize", &mMaximumSize));
	AddProperty(new StrProperty("Name", &mName));
	AddProperty(new MarginsProperty("Margins", &mMargins));

	EnumProperty* prop = new EnumProperty("FitToChildren", (int*)&mFitToChildren);
	prop->AddValueMapping("None", FTC_NONE);
	prop->AddValueMapping("Horizontally", FTC_HORIZONTALLY);
	prop->AddValueMapping("Vertically", FTC_VERTICALLY);
	prop->AddValueMapping("Both", FTC_BOTH);
	AddProperty(prop);
}

Widget::~Widget()
{
	for (int i=0; i<mChildren.GetSize(); i++)
	{
		delete mChildren[i];
	}

	for (int i=0; i<mBinds.GetSize(); i++)
	{
		delete mBinds[i];
	}
}

void Widget::PrintLoop(int depth)
{
	Log::I(Str(depth, ' ') + ToStr());
	for (int i=0; i<mChildren.GetSize(); i++)
	{
		mChildren[i]->PrintLoop(depth + 1);
	}
}

void Widget::AddChild(Widget* child)
{
	if (child != NULL)
	{
		mChildren.Append(child);
		NotifyNeedsLayout();
	}
}

Widget* Widget::FindChildByName(const Str& name)
{
	for(int i = 0; i < mChildren.GetSize(); i++)
	{
		if (mChildren[i]->mName == name)
			return mChildren[i];
	}

	for(int i = 0; i < mChildren.GetSize(); i++)
	{
		Widget* w = mChildren[i]->FindChildByName(name);
		if (w != NULL)
			return w;
	}
	
	return NULL;
}

IWithProperties* Widget::GetBindingContext() const
{
	if (mBindingContext == NULL && mParent != NULL)
	{
		return mParent->GetBindingContext();
	}
	return const_cast<IWithProperties*>(mBindingContext.GetPointer());
}

void Widget::SetBindingContext(IWithProperties* newContext)
{
	mBindingContext = newContext;

	UpdateBindsToNewBindingContext(newContext);
	
	//ApplyBinds();
}

void Widget::UpdateBindsToNewBindingContext(IWithProperties* newContext)
{
	for (int i=0; i<mBinds.GetSize(); i++)
	{
		Bind* bind = mBinds[i];
		bind->SetSourceIfUsingBindingContext(newContext);
	}

	NotifyChangeEventArgs args("BindingContext");
	NotifyChange(&args);

	for (int i=0; i<mChildren.GetSize(); i++)
	{
		mChildren[i]->UpdateBindsToNewBindingContext(newContext);
	}
}

void Widget::AddBind(Bind* bind)
{
	mBinds.Append(bind);
}

void Widget::ApplyBinds()
{
	for (int i=0; i<mBinds.GetSize(); i++)
	{
		mBinds[i]->Apply();
	}

	Log::IndentIn();
	for (int i=0; i<mChildren.GetSize(); i++)
	{
		mChildren[i]->ApplyBinds();
	}
	Log::IndentOut();
}

void Widget::ApplyTemplate(XMLNode* templateNode)
{
	DestroyContent();
	XWMLReader::LoadWidgetPropertiesAndChildren(templateNode, this);
}

void Widget::DestroyContent()
{
	GetContentWidget()->DestroyChildren();
}

void Widget::DestroyChildren()
{
	int size = mChildren.GetSize();
	while (mChildren.GetSize() > 0)
	{
		mChildren[0]->Destroy();
		if (size != mChildren.GetSize() + 1)
		{
			throw InvalidOperationException("Infinite loop detected, mChildren should decrease in length");
		}
		size--;
	}
}

Generic Widget::GetPropertyValue(const Str& name) const
{
	if (name == "BindingContext")
	{
		return Generic(GetBindingContext());
	}
	else if (name == "Children")
	{
		// FIXME
		//return Generic(new ListGenericWrapper<Widget*>(&mChildren));
		return Generic();
	}
	else
	{
		return WithProperties::GetPropertyValue(name);
	}
}

void Widget::SetPropertyValue(const Str& name, const Generic& value)
{
	if (name == "BindingContext")
	{
		IWithProperties* newBindingContext;
		if (value.asType(&newBindingContext))
		{
			SetBindingContext(newBindingContext);
		}
		else
		{
			Log::E("Value not valid for property 'BindingContext'");
		}
	}
	else
	{
		WithProperties::SetPropertyValue(name, value);
	}
}

Point2 Widget::GetScreenPosition()
{
	Widget* widget = this;
	Point2 position = mPosition + mTranslation;

	while (widget->mParent != NULL)
	{
		widget = widget->mParent;
		position += widget->mPosition + widget->mTranslation;
	}

	return position;
}

Point2 Widget::GetRelativePosition(Widget* w)
{
	Widget* widget = this;
	Point2 position = mPosition + mTranslation;

	while (widget->mParent != NULL && widget->mParent != w)
	{
		widget = widget->mParent;
		position += widget->mPosition + widget->mTranslation;
	}

	return position;
}

Window* Widget::GetWindow()
{
	Widget* p = this;

	while (p->mParent != NULL)
	{
		p = p->mParent;
	}

	return dynamic_cast<Window*>(p);
}

void Widget::NotifyNeedsLayout()
{
	Window* w = GetWindow();
		if (w)
			w->NotifyNeedsLayout();
}

void Widget::AttachToParent(Widget* value)
{
	if (!mParent && value)
	{
		mParent = value;
		mParent->mChildren.Append(this);
		NotifyNeedsLayout();
	}
}

bool Widget::IsChildOf(const Widget* parent)
{
	Widget* w = this;

	while (w)
	{
		if (parent == w)
		{
			return true;
		}

		w = w->mParent;
	}

	return false;
}

void Widget::Resize(int width, int height)
{
	bool changed = false;
	if (width != mSize.x || height != mSize.y)
		changed = true;
	mSize = Point2(width, height);
	if (changed)
	{
		Window* w = GetWindow();
		if (w)
			w->GetWindowsManager()->SizeChangedHelper(this);
	}
}

void Widget::Move(int x, int y)
{
	mPosition = Point2(x, y);
}

void Widget::SetDesiredPosition(int x, int y)
{
	mDesiredPosition = Point2(x, y);
	NotifyNeedsLayout();
}

void Widget::SetDesiredSize(int x, int y)
{
	mDesiredSize = Point2(x, y);
	NotifyNeedsLayout();
}

void Widget::SetMinimumSize(int x, int y)
{
	mMinimumSize = Point2(x, y);
	NotifyNeedsLayout();
}

void Widget::SetMaximumSize(int x, int y)
{
	mMaximumSize = Point2(x, y);
	NotifyNeedsLayout();
}

void Widget::SetMargins(int left, int top, int right, int bottom)
{
	mMargins.left = left;
	mMargins.top = top;
	mMargins.right = right;
	mMargins.bottom = bottom;
	NotifyNeedsLayout();
}

bool Widget::HasTextInputFocus()
{
	Window* w = GetWindow();
	if (w)
	{
		WindowsManager* windowsManager = w->GetWindowsManager();
		return windowsManager->GetTextInputFocusWidget() == this;
	}
	return false;
}

bool Widget::MouseCapture()
{
	Window* root = GetWindow();

	if (root == NULL)
	{
		return false;
	}

	return root->GetWindowsManager()->MouseCapture(this);
}

void Widget::MouseRelease()
{
	Window* root = GetWindow();

	if (root == NULL)
	{
		return;
	}

	return root->GetWindowsManager()->MouseRelease();
}

void Widget::GetFocus()
{
	if (HasTextInputFocus())
		return;
	Window* w = GetWindow();
	if (w == NULL)
		return;
	w->GetWindowsManager()->_SetTextInputFocusWidget(this);
}

void Widget::Destroy()
{
	Window* w = GetWindow();
	if (w)
	{
		WindowsManager* windowsManager = w->GetWindowsManager();
		windowsManager->ReleaseWidget(this);
	}
	if (mParent)
	{
		int indexOnParent = mParent->mChildren.Find(this);
		mParent->mChildren.Remove(indexOnParent);
		mParent = NULL;
	}
	Trash();
}

Widget* Widget::GetContentWidget()
{
	return this;
}

void Widget::OnPreviewMouseIn(MouseEventArgs* /*args*/)
{
	mIsMouseOver = true;
}

void Widget::OnPreviewMouseOut(MouseEventArgs* /*args*/)
{
	mIsMouseOver = false;
}

void Widget::OnPreviewMouseMove(MouseMoveEventArgs* /*args*/)
{
}

void Widget::OnPreviewMouseDown(MouseButtonEventArgs* /*args*/)
{
}

void Widget::OnPreviewMouseUp(MouseButtonEventArgs* /*args*/)
{
}

void Widget::OnPreviewKeyDown(KeyboardEventArgs* /*args*/)
{
}

void Widget::OnPreviewKeyUp(KeyboardEventArgs* /*args*/)
{
}

void Widget::OnPreviewTextInput(TextInputEventArgs* /*args*/)
{
}

void Widget::OnMouseIn(MouseEventArgs* /*args*/)
{	
}

void Widget::OnMouseOut(MouseEventArgs* /*args*/)
{
}

void Widget::OnMouseMove(MouseMoveEventArgs* /*args*/)
{
}

void Widget::OnMouseDown(MouseButtonEventArgs* /*args*/)
{
}

void Widget::OnMouseUp(MouseButtonEventArgs* /*args*/)
{
}

void Widget::OnKeyDown(KeyboardEventArgs* /*args*/)
{
}

void Widget::OnKeyUp(KeyboardEventArgs* /*args*/)
{
}

void Widget::OnTextInput(TextInputEventArgs* /*args*/)
{
}

void Widget::OnGotFocus(WindowingEventArgs* /*args*/)
{
}

void Widget::OnLostFocus(WindowingEventArgs* /*args*/)
{
}

void Widget::OnSizeChanged(WindowingEventArgs* /*args*/)
{
}

void Widget::OnMouseInHelper(bool doPreview, MouseEventArgs* args)
{
	if (doPreview)
		OnPreviewMouseIn(args);
	else
		OnMouseIn(args);
}

void Widget::OnMouseOutHelper(bool doPreview, MouseEventArgs* args)
{
	if (doPreview)
		OnPreviewMouseOut(args);
	else
		OnMouseOut(args);
}

void Widget::OnMouseMoveHelper(bool doPreview, MouseMoveEventArgs* args)
{
	if (doPreview)
		OnPreviewMouseMove(args);
	else
		OnMouseMove(args);
}

void Widget::OnMouseDownHelper(bool doPreview, MouseButtonEventArgs* args)
{
	if (doPreview)
		OnPreviewMouseDown(args);
	else
		OnMouseDown(args);
}

void Widget::OnMouseUpHelper(bool doPreview, MouseButtonEventArgs* args)
{
	if (doPreview)
		OnPreviewMouseUp(args);
	else
		OnMouseUp(args);
}

void Widget::OnKeyDownHelper(bool doPreview, KeyboardEventArgs* args)
{
	if (doPreview)
		OnPreviewKeyDown(args);
	else
		OnKeyDown(args);
}

void Widget::OnKeyUpHelper(bool doPreview, KeyboardEventArgs* args)
{
	if (doPreview)
		OnPreviewKeyUp(args);
	else
		OnKeyUp(args);
}

void Widget::OnTextInputHelper(bool doPreview, TextInputEventArgs* args)
{
	if (doPreview)
		OnPreviewTextInput(args);
	else
		OnTextInput(args);
}

void Widget::OnGotFocusHelper(bool doPreview, WindowingEventArgs* args)
{
	if (doPreview)
		;
	else
		OnGotFocus(args);
}

void Widget::OnLostFocusHelper(bool doPreview, WindowingEventArgs* args)
{
	if (doPreview)
		;
	else
		OnLostFocus(args);
}

void Widget::OnSizeChangedHelper(bool doPreview, WindowingEventArgs* args)
{
	if (doPreview)
		;
	else
		OnSizeChanged(args);
}

void Widget::DrawChildren(const DrawingClipInfo& clipInfo)
{
	Renderer* r = GetDevice()->GetRenderer();

	for (int i=0; i<mChildren.GetSize(); i++)
	{
	  if (mChildren[i]->mVisibility != WV_VISIBLE)
			continue;

		const Point2& childPos = mChildren[i]->GetPosition();
		const Point2& childTrans = mChildren[i]->GetTranslation();
		const Point2& childSize = mChildren[i]->GetSize();

		DrawingClipInfo childClipInfo;
		childClipInfo.screenX = clipInfo.screenX + childPos.x + childTrans.x;
		childClipInfo.screenY = clipInfo.screenY + childPos.y + childTrans.y;

		childClipInfo.sx = childClipInfo.screenX;
		childClipInfo.sy = childClipInfo.screenY;
		childClipInfo.sw = childSize.x;
		childClipInfo.sh = childSize.y;

		if (childClipInfo.sx < clipInfo.sx)
		{
			childClipInfo.sw = Math::Max<int>(0, childClipInfo.sx + childClipInfo.sw - clipInfo.sx);
			childClipInfo.sx = clipInfo.sx;
		}

		if (childClipInfo.sy < clipInfo.sy)
		{
			childClipInfo.sh = Math::Max<int>(0, childClipInfo.sy + childClipInfo.sh - clipInfo.sy);
			childClipInfo.sy = clipInfo.sy;
		}

		if (childClipInfo.sx + childClipInfo.sw > clipInfo.sx + clipInfo.sw)
			childClipInfo.sw = Math::Max<int>(0, clipInfo.sx + clipInfo.sw - childClipInfo.sx);

		if (childClipInfo.sy + childClipInfo.sh > clipInfo.sy + clipInfo.sh)
			childClipInfo.sh = Math::Max<int>(0, clipInfo.sy + clipInfo.sh - childClipInfo.sy);

		if (childClipInfo.sw == 0 || childClipInfo.sh == 0)
			continue;

		r->SetScissorBox(childClipInfo.sx, childClipInfo.sy, childClipInfo.sw, childClipInfo.sh);

		//Log::I("screenX: " + Str(childClipInfo.screenX) + ", screenY: " + Str(childClipInfo.screenY) + ", sx" + Str(childClipInfo.sx) + ", sy" + Str(childClipInfo.sy) + ", sw" + Str(childClipInfo.sw) + ", sh" + Str(childClipInfo.sh));

		r->PushMatrix();
		glTranslatef((float)(childPos.x + childTrans.x), (float)(childPos.y + childTrans.y), 0.0f);
		mChildren[i]->OnDraw(childClipInfo); 
		r->PopMatrix();
	}

	//Upon exit, set the scissorbox to the whole widget, because when drawing children it was altered
	r->SetScissorBox(clipInfo.sx, clipInfo.sy, clipInfo.sw, clipInfo.sh);
}

void Widget::DebugDrawBorder()
{
	glDisable(GL_SCISSOR_TEST);
	glEnable(GL_LINE_STIPPLE);
	glLineStipple(2, 0x5555);
	glBegin(GL_LINE_LOOP);

	if (mIsMouseOver)
	{
		glColor3i(1, 1, 0);
	}
	else
	{
		glColor3i(1, 0, 1);
	}

	glVertex3i(-1     , -1     , 0);
	glVertex3i(mSize.x, -1     , 0);
	glVertex3i(mSize.x, mSize.y, 0);
	glVertex3i(-1     , mSize.y, 0);
	glEnd();
	glDisable(GL_LINE_STIPPLE);
	glEnable(GL_SCISSOR_TEST);
}

void Widget::DrawInit(DrawingClipInfo& /*clipInfo*/)
{
	//glTranslatef(mTranslation.x, mTranslation.y, 0.0f);
	//clipInfo.screenX += mTranslation.x;
	//clipInfo.screenY += mTranslation.y;
}

void Widget::OnDraw(DrawingClipInfo& clipInfo)
{
	DrawInit(clipInfo);
	if (mDrawDebugBorder)
	{
		DebugDrawBorder();
	}
	DrawChildren(clipInfo);
}

/*
 * The goal of this function is to determine whether the widget has
 * dimensions set by hand, by children if fitting to children, or
 * is decided by the layout
 */
void Widget::OnMeasure(bool fitChildrenX, bool fitChildrenY)
{
	if (mVisibility == WV_COLLAPSED)
	{
		mMeasuredSize.x = 0;
		mMeasuredSize.y = 0;
		return;
	}

	fitChildrenX |= Math::TestBitmask(mFitToChildren, FTC_HORIZONTALLY);
	fitChildrenY |= Math::TestBitmask(mFitToChildren, FTC_VERTICALLY);

	if (fitChildrenX && mDesiredSize.x >= 0)
	{
		fitChildrenX = false;
	}

	if (fitChildrenY && mDesiredSize.y >= 0)
	{
		fitChildrenY = false;
	}

	for (int i = 0; i < mChildren.GetSize(); i++)
	{
		mChildren[i]->OnMeasure(fitChildrenX, fitChildrenY);
	}

	if (fitChildrenX)
		OnMeasureFitX();
	else
		mMeasuredSize.x = mDesiredSize.x;
	
	if (mMeasuredSize.x >= 0)
		mMeasuredSize.x = Math::Min(Math::Max(mMeasuredSize.x, mMinimumSize.x), mMaximumSize.x);

	if (fitChildrenY)
		OnMeasureFitY();
	else
		mMeasuredSize.y = mDesiredSize.y;

	if (mMeasuredSize.y >= 0)
		mMeasuredSize.y = Math::Min(Math::Max(mMeasuredSize.y, mMinimumSize.y), mMaximumSize.y);
}

/*
 * Computes the X MeasuredSize component based on child dimensions, which are all computed recursively
 * before the call.
 * It must be overriden by custom layouts to determine the layout size based on children sizes when
 * doing a FitToChildren.
 */
void Widget::OnMeasureFitX()
{
	mMeasuredSize.x = Math::Max(0, mDesiredSize.x);

	for (int i = 0; i < mChildren.GetSize(); i++)
	{
		Widget* child = mChildren[i];

		int w = child->mMeasuredSize.x;
		if (child->mMargins.left > 0)
			w += child->mMargins.left;
		if (child->mMargins.right > 0)
			w += child->mMargins.right;
		
		mMeasuredSize.x = Math::Max(mMeasuredSize.x, w);
	}
}

void Widget::OnMeasureFitY()
{
	mMeasuredSize.y = Math::Max(0, mDesiredSize.y);

	for (int i = 0; i < mChildren.GetSize(); i++)
	{
		Widget* child = mChildren[i];

		int h = child->mMeasuredSize.y;
		if (child->mMargins.top > 0)
			h += child->mMargins.top;
		if (child->mMargins.bottom > 0)
			h += child->mMargins.bottom;
		
		mMeasuredSize.y = Math::Max(mMeasuredSize.y, h);
	}
}

void Widget::OnArrange(Point2 position, Point2 size)
{
	Move(position);
	if (mVisibility == WV_COLLAPSED)
	{
		Resize(Point2::ZERO);
		return;
	}
	Resize(size);

	for (int i=0; i<mChildren.GetSize(); i++)
	{
		Point2 childPosition = mChildren[i]->GetDesiredPosition();
		Point2 childSize = mChildren[i]->GetMeasuredSize();
    
    Point2 vv;
    vv.x = 0;
    vv.y = 0;
    childSize = size;
    
    mChildren[i]->FitMeasuredSizeX(vv, childSize);
    mChildren[i]->FitMeasuredSizeY(vv, childSize);

//		if (childSize.x < 0.0f)
//		{
//			childSize.x = size.x - childPosition.x;
//			mChildren[i]->ErodeMarginsX(childPosition, childSize);
//		}
//		else
//			mChildren[i]->EnlargeMarginsX(childPosition, childSize);
//
//		if (childSize.y < 0.0f)
//		{
//			childSize.y = size.y - childPosition.y;
//			mChildren[i]->ErodeMarginsY(childPosition, childSize);
//		}
//		else
//			mChildren[i]->EnlargeMarginsY(childPosition, childSize);
		
		mChildren[i]->OnArrange(vv, childSize);
	}
}

int Widget::EnlargeMarginsX(Point2& position, Point2& size)
{
	int v = size.x;
	if (mMargins.left > 0)
	{
		position.x += mMargins.left;
		v += mMargins.left;
	}
	if (mMargins.right > 0)
	{
		v += mMargins.right;
	}
	return v;
}

int Widget::EnlargeMarginsY(Point2& position, Point2& size)
{
	int v = size.y;
	if (mMargins.top > 0)
	{
		position.y += mMargins.top;
		v += mMargins.top;
	}
	if (mMargins.bottom > 0)
	{
		v += mMargins.bottom;
	}
	return v;
}

void Widget::ErodeMarginsX(Point2& position, Point2& size)
{
	if (mMargins.left > 0)
	{
		position.x += mMargins.left;
		size.x -= mMargins.left;
	}

	if (mMargins.right > 0)
	{
		size.x -= mMargins.right;
	}
}

void Widget::ErodeMarginsY(Point2& position, Point2& size)
{
	if (mMargins.top > 0)
	{
		position.y += mMargins.top;
		size.y -= mMargins.top;
	}

	if (mMargins.bottom > 0)
	{
		size.y -= mMargins.bottom;
	}
}

void Widget::FitMeasuredSizeX(Point2& position, Point2& size)
{
	Point2 origSize = size;
	Point2 origPosition = position;

	if (mMeasuredSize.x < 0)
	{
		//Se i margini sono entrambi settati, utilizza quelli
		if (mMargins.left >= 0)
		{
			position.x += mMargins.left;
			size.x -= mMargins.left;
		}
		if (mMargins.right >= 0)
		{
			size.x -= mMargins.right;
		}
	}
	else
	{
		if (mMargins.left >= 0)
		{
			if (mMargins.right < 0)
			{
				position.x += mMargins.left;
			}
			else if (mMargins.left + mMargins.right + mMeasuredSize.x < size.x)
			{
				//I margini non sono sufficienti a coprire la dimensione data, posizionati nel mezzo
				//TODO: Inserire qui le politiche di allineamento
				position.x += (size.x - mMeasuredSize.x) / 2;
			}
			else
			{
				position.x += mMargins.left;
			}
		}
		else
		{
			if (mMargins.right < 0)
			{
				position.x += (size.x - mMeasuredSize.x) / 2;
			}
			else
			{
				position.x += size.x - mMeasuredSize.x - mMargins.right;
			}
		}

		size.x = mMeasuredSize.x;
	}

	if (size.x < mMinimumSize.x)
	{
		mMeasuredSize.x = mMinimumSize.x;
		position = origPosition;
		size = origSize;
		FitMeasuredSizeX(position, size);
	}
	if (size.x > mMaximumSize.x)
	{
		mMeasuredSize.x = mMaximumSize.x;
		position = origPosition;
		size = origSize;
		FitMeasuredSizeX(position, size);
	}
}

void Widget::FitMeasuredSizeY(Point2& position, Point2& size)
{
	Point2 origSize = size;
	Point2 origPosition = position;

	if (mMeasuredSize.y < 0)
	{
		//Se i margini sono entrambi settati, utilizza quelli
		if (mMargins.top >= 0)
		{
			position.y += mMargins.top;
			size.y -= mMargins.top;
		}
		if (mMargins.bottom >= 0)
		{
			size.y -= mMargins.bottom;
		}
	}
	else
	{
		if (mMargins.top >= 0)
		{
			if (mMargins.bottom < 0)
			{
				position.y += mMargins.top;
			}
			else if (mMargins.top + mMargins.bottom + mMeasuredSize.y < size.y)
			{
				//I margini non sono sufficienti a coprire la dimensione data, posizionati nel mezzo
				//TODO: Inserire qui le politiche di allineamento
				position.y += (size.y - mMeasuredSize.y) / 2;
			}
			else
			{
				position.y += mMargins.top;
			}
		}
		else
		{
			if (mMargins.bottom < 0)
			{
				position.y += (size.y - mMeasuredSize.y) / 2;
			}
			else
			{
				position.y += size.y - mMeasuredSize.y - mMargins.bottom;
			}
		}
		size.y = mMeasuredSize.y;
	}

	if (size.y < mMinimumSize.y)
	{
		mMeasuredSize.y = mMinimumSize.y;
		position = origPosition;
		size = origSize;
		FitMeasuredSizeX(position, size);
	}
	if (size.y > mMaximumSize.y)
	{
		mMeasuredSize.y = mMaximumSize.y;
		position = origPosition;
		size = origSize;
		FitMeasuredSizeX(position, size);
	}
}

WidgetSizeAcceptedEnum Widget::GetSizeAcceptedX(int sizeX)
{
	if (mMeasuredSize.x >= 0)	//A desired size is set, use that one regardless of the available space
		return WSA_FIXED;

	int w = sizeX;

	//Apply margins if they are not 'auto'
	//'auto' margins have effect only if the desired size is set, so just ignore them here
	if (mMargins.left >= 0)
		w -= mMargins.left;
	if (mMargins.right >= 0)
		w -= mMargins.right;
	
	if (w < mMinimumSize.x)
		return WSA_MIN_SIZE;
	if (w > mMaximumSize.x)
		return WSA_MAX_SIZE;
	return WSA_ACCEPTED;
}

WidgetSizeAcceptedEnum Widget::GetSizeAcceptedY(int sizeY)
{
	if (mMeasuredSize.y >= 0)	//A desired size is set, use that one regardless of the available space
		return WSA_FIXED;

	int h = sizeY;

	//Apply margins if they are not 'auto'
	//'auto' margins have effect only if the desired size is set, so just ignore them here
	if (mMargins.top >= 0)
		h -= mMargins.top;
	if (mMargins.bottom >= 0)
		h -= mMargins.bottom;

	if (h < mMinimumSize.y)
		return WSA_MIN_SIZE;
	if (h > mMaximumSize.y)
		return WSA_MAX_SIZE;
	return WSA_ACCEPTED;
}

} //namespace Crown
