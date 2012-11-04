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

#include "Camera.h"
#include "Device.h"
#include "Types.h"
#include "Renderer.h"

namespace Crown
{

Camera::Camera(const Vec3& position, const Angles& axis,
			   bool visible, float fov, float aspect, bool active) :
	mLookAt(0, 0, -1),
	mUp(0, 1, 0),
	mFOV(fov),
	mAspect(aspect),
	mNear(0.1f),
	mFar(1000.0f),
	mActive(active),
	mAutoAspect(true)
{
	UpdateProjectionMatrix();
	UpdateViewMatrix();
	UpdateFrustum();
}

Camera::~Camera()
{
}

void Camera::SetPosition(const Vec3& position)
{
	mPosition = position;
	UpdateViewMatrix();
}

const Vec3& Camera::GetLookAt() const
{
	return mLookAt;
}

void Camera::SetLookAt(const Vec3& lookat)
{
	mLookAt = lookat;
	UpdateViewMatrix();
}

const Vec3& Camera::GetUpVector() const
{
	return mUp;
}

bool Camera::IsActive() const
{
	return mActive;
}

void Camera::SetActive(bool active)
{
	mActive = active;
}

float Camera::GetFOV() const
{
	return mFOV;
}

void Camera::SetFOV(float fov)
{
	mFOV = fov;
	UpdateProjectionMatrix();
}

bool Camera::GetAutoAspect() const
{
	return mAutoAspect;
}

void Camera::SetAutoAspect(bool autoAspect)
{
	mAutoAspect = autoAspect;
}

float Camera::GetAspect() const
{
	return mAspect;
}

void Camera::SetAspect(float aspect)
{
	mAspect = aspect;
	UpdateProjectionMatrix();
}

float Camera::GetNearClipDistance() const
{
	return mNear;
}

void Camera::SetNearClipDistance(float near)
{
	mNear = near;
	UpdateProjectionMatrix();
}

float Camera::GetFarClipDistance() const
{
	return mFar;
}

void Camera::SetFarClipDistance(float far)
{
	mFar = far;
	UpdateProjectionMatrix();
}

const Mat4& Camera::GetProjectionMatrix() const
{
	return mProjection;
}

const Mat4& Camera::GetViewMatrix() const
{
	return mView;
}

const Frustum& Camera::GetFrustum() const
{
	return mFrustum;
}

void Camera::Render()
{
	GetDevice()->GetRenderer()->SetMatrix(MT_PROJECTION, mProjection);
	GetDevice()->GetRenderer()->SetMatrix(MT_VIEW, mView);
}

void Camera::UpdateProjectionMatrix()
{
	mProjection.build_projection_perspective_rh(mFOV, mAspect, mNear, mFar);
}

void Camera::UpdateViewMatrix()
{
	mView.build_look_at_rh(mPosition, mPosition + mLookAt, mUp);
}

void Camera::UpdateFrustum()
{
	mFrustum.from_matrix(mProjection * mView);
}

} // namespace Crown

