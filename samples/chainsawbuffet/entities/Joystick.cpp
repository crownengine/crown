#include "Joystick.h"
#include "Globals.h"

Joystick::Joystick(Scene* scene, Camera* camera):
	SolidSceneNode(scene, NULL,  Vec3(0, 0, 0), Angles(0, 0, 0), true, CG_NONE), mDragging(false), 
	mInitialPosition(80, 400, 0), mMouseStartDragPosition(0, 0, 0),
	mDelta(0, 0, 0)
{
	Globals::joystick = this;
	mJoystickSprite = Sprite::LoadSpriteFromImage("res/chainsawbuffet/joystick.bmp");
	mJoystickSprite->GetFrame(0)->SetOffset(64, 64);

	mPosition = mInitialPosition;

	GetDevice()->GetInputManager()->RegisterMouseListener(this);
}

Joystick::~Joystick()
{
	Globals::joystick = NULL;
	//delete mBodySprite;
}

const Vec3& Joystick::GetDelta() const
{
	return mDelta;
}

void Joystick::Render()
{
	//Do an update
	mPosition = mInitialPosition + mDelta;

	SolidSceneNode::Render();
	mJoystickSprite->draw(0);
}

void Joystick::ButtonPressed(const MouseEvent& event)
{
	if (event.button == MB_LEFT)
	{
		Point2 mousePosition = GetDevice()->GetInputManager()->GetMouse()->GetCursorXY();
		mMouseStartDragPosition = Vec3((real)mousePosition.x, (real)mousePosition.y, 0.0);

		mDragging = ((mMouseStartDragPosition - mInitialPosition).GetSquaredLength() <= 60*60);
	}
}

void Joystick::ButtonReleased(const MouseEvent& event)
{
	if (event.button == MB_LEFT)
	{
		mDragging = false;
		mDelta = Vec3::ZERO;
	}
}

void Joystick::CursorMoved(const MouseEvent& event)
{
	if (mDragging)
	{
		mDelta = GetDevice()->GetInputManager()->GetMouse()->GetCursorXY().ToVec3() - mMouseStartDragPosition;
		if (mDelta.GetSquaredLength() > 900)
		{
			mDelta = mDelta.Normalize() * 30;
		}
	}
}

