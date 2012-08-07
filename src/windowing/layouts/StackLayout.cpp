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

#include "StackLayout.h"
#include "Types.h"
#include "SpecificProperties.h"

namespace Crown
{

StackLayout::StackLayout(Widget* parent):
	Widget(parent), mOrientation(SO_VERTICAL)
{
	SetMouseSensible(false);

	EnumProperty* prop = new EnumProperty("Orientation", (int*)&mOrientation);
	prop->AddValueMapping("Horizontal", SO_HORIZONTAL);
	prop->AddValueMapping("Vertical", SO_VERTICAL);
	AddProperty(prop);
}

StackLayout::~StackLayout()
{
}

void StackLayout::SetOrientation(StackOrientation value)
{
	mOrientation = value;
	NotifyNeedsLayout();
}

void StackLayout::OnSetProperty(const Str& name)
{
	if (name == "Orientation")
	{
		NotifyNeedsLayout();
	}
	else
	{
		Widget::OnSetProperty(name);
	}
}

void StackLayout::OnArrange(Point2 position, Point2 size)
{
	Move(position);
	Resize(size);

	//La misurazione è stata fatta, ogni widget figlio sa le dimensioni che desidera o se sono arbitrarie

	if (mOrientation == SO_VERTICAL)
	{
		//Calculate the remaining space, given that some widgets have an height set

		//Obiettivo: assegnare la dimensione desiderata ai widget che la hanno, 
		//					 individuare lo spazio rimanente e finchè ad almeno un widget non va bene
		//					 riprovare a calcolarlo
		List<WidgetSizeAcceptedEnum> acceptedStatus;
		const List<Widget*>& children = GetChildren();

		int fixedHeight = 0;
		int fixedHeightCount = 0;
		

		for (int i = 0; i < children.GetSize(); i++)
		{
			Widget* child = children[i];
			int measuredY = child->GetMeasuredSize().y;

			if (measuredY >= 0)
			{
				acceptedStatus.Append(WSA_FIXED);
				fixedHeightCount ++;
				//Take margins into account
				fixedHeight += measuredY + child->GetMargins().GetFixedMarginsY();
			}
			else
				acceptedStatus.Append(WSA_ACCEPTED);
		}

		float suggestedHeight = ((float)size.y - fixedHeight) / (children.GetSize() - fixedHeightCount);
		float roundupRemainder = suggestedHeight - (int)(suggestedHeight);
		float accumulator = 0.0f;
		suggestedHeight = (float)((int)suggestedHeight);

		if (suggestedHeight < 0.0f)
		{
			//No free space to assign, go over to actual assignment phase
			suggestedHeight = 0.0f;
		}
		else
		{
			for (int i = 0; i < children.GetSize(); i++)
			{
				Widget* child = children[i];

				WidgetSizeAcceptedEnum accepted = acceptedStatus[i];

				if (accepted != WSA_ACCEPTED)
					continue;
				else
				{
					//Try to propose the calculated height and see if the response changes
					//If so, restart the scan with a new suggestedHeight
					int height = (int)suggestedHeight;
					accumulator += roundupRemainder;
					if (accumulator >= 0.9999f)
					{
						accumulator -= 1.0f;
						height++;
					}

					WidgetSizeAcceptedEnum acceptedNew = child->GetSizeAcceptedY(height);
					if (acceptedNew != accepted)
					{
						//Based on acceptedNew, which is now either WSA_MIN_SIZE or WSA_MAX_SIZE, add the min or max size to the fixedHeight and start over
						switch (acceptedNew)
						{
							case WSA_MIN_SIZE:
								fixedHeight += child->GetMinimumSize().y;
							break;
							case WSA_MAX_SIZE:
								fixedHeight += child->GetMaximumSize().y;
							break;
						}
						//Consider the fixed margins as well
						fixedHeight += child->GetMargins().GetFixedMarginsY();
						fixedHeightCount += 1;

						if (children.GetSize() == fixedHeightCount)
						{
							//No more variable sized widgets to calculate, exit the acceptation for loop
							break;
						}

						suggestedHeight = ((float)size.y - fixedHeight) / (children.GetSize() - fixedHeightCount);
						roundupRemainder = suggestedHeight - (int)(suggestedHeight);
						accumulator = 0.0f;
						suggestedHeight = (float)((int)suggestedHeight);
						//Replace the acceptedStatus and start over
						acceptedStatus[i] = acceptedNew;
						i = -1;
					}
				}
			}
		}

		accumulator = 0.0f;

		position = Point2::ZERO;
		int xSpace = size.x;
		size.y = 0;

		for (int i = 0; i < children.GetSize(); i++)
		{
			Widget* child = children[i];
			Point2 pos = position;
			int occupiedSpace = 0;
			bool applyMargins = false;

			switch (acceptedStatus[i])
			{
				case WSA_ACCEPTED:
				{
					//Use the suggested height
					int height = (int)suggestedHeight;
					accumulator += roundupRemainder;
					if (accumulator >= 0.9999f)
					{
						accumulator -= 1.0f;
						height++;
					}

					occupiedSpace = height;
					size.y = height;
					child->ErodeMarginsY(pos, size);
				}break;

				case WSA_MIN_SIZE:
				{
					//Pass in the min size plus the margins
					size.y = child->GetMinimumSize().y;
					applyMargins = true;
				}break;

				case WSA_MAX_SIZE:
				{
					//Pass in the min size plus the margins
					size.y = child->GetMaximumSize().y;
					applyMargins = true;
				}break;

				case WSA_FIXED:
				{
					//Pass in the min size plus the margins
					size.y = child->GetMeasuredSize().y;
					applyMargins = true;
				}break;
			}

			if (applyMargins)
			{
				occupiedSpace = size.y + child->GetMargins().GetFixedMarginsY();
				if (child->GetMargins().top > 0)
				{
					pos.y += child->GetMargins().top;
				}
			}

			size.x = xSpace;
			child->FitMeasuredSizeX(pos, size);
			child->OnArrange(pos, size);
			position.y += occupiedSpace;
		}
	}
	else
	{
		//Calculate the remaining space, given that some widgets have an height set

		//Obiettivo: assegnare la dimensione desiderata ai widget che la hanno, 
		//					 individuare lo spazio rimanente e finchè ad almeno un widget non va bene
		//					 riprovare a calcolarlo
		List<WidgetSizeAcceptedEnum> acceptedStatus;
		const List<Widget*>& children = GetChildren();

		int fixedHeight = 0;
		int fixedHeightCount = 0;
		

		for (int i = 0; i < children.GetSize(); i++)
		{
			Widget* child = children[i];
			int measuredX = child->GetMeasuredSize().x;

			if (measuredX >= 0)
			{
				acceptedStatus.Append(WSA_FIXED);
				fixedHeightCount ++;
				//Take margins into account
				fixedHeight += measuredX + child->GetMargins().GetFixedMarginsX();
			}
			else
				acceptedStatus.Append(WSA_ACCEPTED);
		}

		float suggestedHeight = ((float)size.x - fixedHeight) / (children.GetSize() - fixedHeightCount);
		float roundupRemainder = suggestedHeight - (int)(suggestedHeight);
		float accumulator = 0.0f;
		suggestedHeight = (float)((int)suggestedHeight);

		if (suggestedHeight < 0.0f)
		{
			//No free space to assign, go over to actual assignment phase
			suggestedHeight = 0.0f;
		}
		else
		{
			for (int i = 0; i < children.GetSize(); i++)
			{
				Widget* child = children[i];

				WidgetSizeAcceptedEnum accepted = acceptedStatus[i];

				if (accepted != WSA_ACCEPTED)
					continue;
				else
				{
					//Try to propose the calculated height and see if the response changes
					//If so, restart the scan with a new suggestedHeight
					int width = (int)suggestedHeight;
					accumulator += roundupRemainder;
					if (accumulator >= 0.9999f)
					{
						accumulator -= 1.0f;
						width++;
					}

					WidgetSizeAcceptedEnum acceptedNew = child->GetSizeAcceptedX(width);
					if (acceptedNew != accepted)
					{
						//Based on acceptedNew, which is now either WSA_MIN_SIZE or WSA_MAX_SIZE, add the min or max size to the fixedHeight and start over
						switch (acceptedNew)
						{
							case WSA_MIN_SIZE:
								fixedHeight += child->GetMinimumSize().x;
							break;
							case WSA_MAX_SIZE:
								fixedHeight += child->GetMaximumSize().x;
							break;
						}
						//Consider the fixed margins as well
						fixedHeight += child->GetMargins().GetFixedMarginsX();
						fixedHeightCount += 1;

						if (children.GetSize() == fixedHeightCount)
						{
							//No more variable sized widgets to calculate, exit the acceptation for loop
							break;
						}

						suggestedHeight = ((float)size.x - fixedHeight) / (children.GetSize() - fixedHeightCount);
						roundupRemainder = suggestedHeight - (int)(suggestedHeight);
						accumulator = 0.0f;
						suggestedHeight = (float)((int)suggestedHeight);
						//Replace the acceptedStatus and start over
						acceptedStatus[i] = acceptedNew;
						i = -1;
					}
				}
			}
		}

		accumulator = 0.0f;

		position = Point2::ZERO;
		int ySpace = size.y;
		size.x = 0;

		for (int i = 0; i < children.GetSize(); i++)
		{
			Widget* child = children[i];
			Point2 pos = position;
			int occupiedSpace = 0;
			bool applyMargins = false;

			switch (acceptedStatus[i])
			{
				case WSA_ACCEPTED:
				{
					//Use the suggested height
					int height = (int)suggestedHeight;
					accumulator += roundupRemainder;
					if (accumulator >= 0.9999f)
					{
						accumulator -= 1.0f;
						height++;
					}

					occupiedSpace = height;
					size.x = height;
					child->ErodeMarginsX(pos, size);
				}break;

				case WSA_MIN_SIZE:
				{
					//Pass in the min size plus the margins
					size.x = child->GetMinimumSize().x;
					applyMargins = true;
				}break;

				case WSA_MAX_SIZE:
				{
					//Pass in the min size plus the margins
					size.x = child->GetMaximumSize().x;
					applyMargins = true;
				}break;

				case WSA_FIXED:
				{
					//Pass in the min size plus the margins
					size.x = child->GetMeasuredSize().x;
					applyMargins = true;
				}break;
			}

			if (applyMargins)
			{
				occupiedSpace = size.x + child->GetMargins().GetFixedMarginsX();
				if (child->GetMargins().left > 0)
				{
					pos.x += child->GetMargins().left;
				}
			}

			size.y = ySpace;
			child->FitMeasuredSizeY(pos, size);
			child->OnArrange(pos, size);
			position.x += occupiedSpace;
		}
	}
}

void StackLayout::OnMeasureFitX()
{
	int msx = 0;

	const List<Widget*> children = GetChildren();

	for (int i = 0; i < children.GetSize(); i++)
	{
		Widget* child = children[i];

		int w = child->GetMeasuredSize().x;
		if (child->GetMargins().left > 0)
			w += child->GetMargins().left;
		if (child->GetMargins().right > 0)
			w += child->GetMargins().right;
		
		if (mOrientation == SO_VERTICAL)
			msx = Math::Max(msx, w);
		else
			msx += w;
	}

	mMeasuredSize.x = msx;
}

void StackLayout::OnMeasureFitY()
{
	int msy = 0;

	const List<Widget*> children = GetChildren();

	for (int i = 0; i < children.GetSize(); i++)
	{
		Widget* child = children[i];

		int h = child->GetMeasuredSize().y;
		if (child->GetMargins().top > 0)
			h += child->GetMargins().top;
		if (child->GetMargins().bottom > 0)
			h += child->GetMargins().bottom;
		
		if (mOrientation == SO_HORIZONTAL)
			msy = Math::Max(msy, h);
		else
			msy += h;
	}

	mMeasuredSize.y = msy;
}

}
