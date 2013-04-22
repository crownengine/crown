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
//-----------------------------------------------------------------------
MovableCamera::MovableCamera(const Vec3& position, bool visible, float fov, float aspect, bool active, float speed, float sensibility) :
	Camera(position, visible, fov, aspect, active),
	mSpeed(speed),
	mSensibility(sensibility)
{
	mRotFactor = Vec2(0.0f, 0.0f);
	mAngleX = 0.0f;
	mAngleY = 0.0f;
}

//-----------------------------------------------------------------------
MovableCamera::~MovableCamera()
{
}

//-----------------------------------------------------------------------
float MovableCamera::GetSensibility() const
{
	return mSensibility;
}

//-----------------------------------------------------------------------
void MovableCamera::SetActive(bool active)
{
	Camera::SetActive(active);
	get_input_manager()->set_cursor_relative_xy(Vec2(0.5f, 0.5f));
}

//-----------------------------------------------------------------------
void MovableCamera::SetSensibility(float sensibility)
{
	mSensibility = sensibility;
}

//-----------------------------------------------------------------------
float MovableCamera::GetSpeed() const
{
	return mSpeed;
}

//-----------------------------------------------------------------------
void MovableCamera::SetSpeed(float speed)
{
	mSpeed = speed;
}

//-----------------------------------------------------------------------
void MovableCamera::Render()
{
	if (!mActive)
	{
		return;
	}

	UpdateViewMatrix();

	Camera::Render();
}

//-----------------------------------------------------------------------
void MovableCamera::MoveForward()
{
	mPosition += mLookAt * mSpeed;
}

//-----------------------------------------------------------------------
void MovableCamera::MoveBackward()
{
	mPosition -= mLookAt * mSpeed;
}

//-----------------------------------------------------------------------
void MovableCamera::StrafeLeft()
{
	Vec3 left = mUp.cross(mLookAt);
	left.normalize();
	mPosition += left * mSpeed;
}

//-----------------------------------------------------------------------
void MovableCamera::StrafeRight()
{
	Vec3 left = mUp.cross(mLookAt);
	left.normalize();
	mPosition -= left * mSpeed;
}

//-----------------------------------------------------------------------
void MovableCamera::SetRotation(const float x, const float y)
{
	Vec3 right(1, 0, 0);
	Vec3 look;

	look.x = 0.0f;
	look.y = math::sin(x);
	look.z = -math::cos(x);

	Vec3 up = right.cross(look);
	up.normalize();

	Mat3 m;
	m.build_rotation_y(y);
	look = m * look;
	mUp = m * up;

	Camera::SetLookAt(look);
}

} // namespace crown

