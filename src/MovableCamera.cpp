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

#include "Device.h"
#include "Mat3.h"
#include "Types.h"
#include "MovableCamera.h"
#include "InputManager.h"

namespace crown
{

MovableCamera::MovableCamera(const Vec3& position, const Angles& axis,
				bool visible, float fov, float aspect, bool active, float speed, float sensibility) :
	Camera(position, axis, visible, fov, aspect, active),
	mSpeed(speed),
	mMouseSensibility(sensibility),
	mUpPressed(false),
	mRightPressed(false),
	mDownPressed(false),
	mLeftPressed(false)
{
	GetInputManager()->RegisterKeyboardListener(this);

	mRotFactor = Vec2(0.0f, 0.0f);
	mAngleX = 0.0f;
	mAngleY = 0.0f;

	GetInputManager()->set_cursor_relative_xy(Vec2(0.5f, 0.5f));
}

MovableCamera::~MovableCamera()
{
}

float MovableCamera::GetMouseSensibility() const
{
	return mMouseSensibility;
}

void MovableCamera::SetActive(bool active)
{
	Camera::SetActive(active);
	GetInputManager()->set_cursor_relative_xy(Vec2(0.5f, 0.5f));
}

void MovableCamera::SetMouseSensibility(float sensibility)
{
	mMouseSensibility = sensibility;
}

float MovableCamera::GetSpeed() const
{
	return mSpeed;
}

void MovableCamera::SetSpeed(float speed)
{
	mSpeed = speed;
}

void MovableCamera::Render()
{
	if (!mActive)
	{
		return;
	}

	SetViewByMouse();

	if (mUpPressed)
	{
		MoveForward();
		UpdateViewMatrix();
	}

	if (mRightPressed)
	{
		StrafeRight();
		UpdateViewMatrix();
	}

	if (mDownPressed)
	{
		MoveBackward();
		UpdateViewMatrix();
	}

	if (mLeftPressed)
	{
		StrafeLeft();
		UpdateViewMatrix();
	}

	Camera::Render();
}

void MovableCamera::KeyPressed(const KeyboardEvent& event)
{
	switch (event.key)
	{
		case 'w':
		{
			mUpPressed = true;
			break;
		}
		case 'a':
		{
			mLeftPressed = true;
			break;
		}
		case 's':
		{
			mDownPressed = true;
			break;
		}
		case 'd':
		{
			mRightPressed = true;
			break;
		}
		default:
		{
			break;
		}
	}
}

void MovableCamera::KeyReleased(const KeyboardEvent& event)
{
	switch (event.key)
	{
		case 'w':
		{
			mUpPressed = false;
			break;
		}
		case 'a':
		{
			mLeftPressed = false;
			break;
		}
		case 's':
		{
			mDownPressed = false;
			break;
		}
		case 'd':
		{
			mRightPressed = false;
			break;
		}
		default:
		{
			break;
		}
	}
}

void MovableCamera::MoveForward()
{
	mPosition += mLookAt * mSpeed;
}

void MovableCamera::MoveBackward()
{
	mPosition -= mLookAt * mSpeed;
}

void MovableCamera::StrafeLeft()
{
	Vec3 left = mUp.cross(mLookAt);
	left.normalize();
	mPosition += left * mSpeed;
}

void MovableCamera::StrafeRight()
{
	Vec3 left = mUp.cross(mLookAt);
	left.normalize();
	mPosition -= left * mSpeed;
}

void MovableCamera::SetViewByMouse()
{
	static Vec2 lastPos = GetInputManager()->get_cursor_relative_xy();
	Vec2 currentPos = GetInputManager()->get_cursor_relative_xy();
	GetInputManager()->set_cursor_relative_xy(Vec2(0.5f, 0.5f));

	if (lastPos == currentPos)
	{
		return;
	}

	Vec2 delta = lastPos - currentPos;

	GetInputManager()->set_cursor_relative_xy(Vec2(0.5f, 0.5f));
	lastPos = GetInputManager()->get_cursor_relative_xy();

	mAngleX += delta.y * mMouseSensibility;
	mAngleY += delta.x * mMouseSensibility;

	mAngleX = math::clamp_to_range(-89.999f * math::DEG_TO_RAD, 89.999f * math::DEG_TO_RAD, mAngleX);
	mAngleY = math::fmod(mAngleY, math::TWO_PI);

	Vec3 right(1, 0, 0);
	Vec3 look;

	look.x = 0.0f;
	look.y = math::sin(mAngleX);
	look.z = -math::cos(mAngleX);

	Vec3 up = right.cross(look);
	up.normalize();

	Mat3 m;
	m.build_rotation_y(mAngleY);
	look = m * look;
	mUp = m * up;

	Camera::SetLookAt(look);
}

} // namespace crown

