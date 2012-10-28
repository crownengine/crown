#ifndef __JOYSTICK_H__
#define __JOYSTICK_H__

#include "SolidSceneNode.h"

using namespace Crown;

class Joystick: public SolidSceneNode, public MouseListener
{
public:
	Joystick(Scene* scene, Camera* camera);
	virtual ~Joystick();

	virtual void Render();

	virtual void ButtonPressed(const MouseEvent& event);
	virtual void ButtonReleased(const MouseEvent& event);
	virtual void CursorMoved(const MouseEvent& event);

	//! Returns the Joystick delta
	const Vec3& GetDelta() const;

private:
	Auto<Sprite> mJoystickSprite;
	bool mDragging;
	Vec3 mInitialPosition;
	Vec3 mMouseStartDragPosition;
	Vec3 mDelta;
};

#endif //__JOYSTICK_H__