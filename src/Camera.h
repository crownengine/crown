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
#include "Frustum.h"
#include "Mat4.h"
#include "Vec3.h"

namespace crown
{

class Camera
{

public:

	//! Constructor
	Camera(const Vec3& position, const Angles& axis, bool visible, float fov, float aspect, bool active);

	//! Destructor
	virtual ~Camera();

	//! Sets the camera's position
	void SetPosition(const Vec3& position);

	//! Returns the camera's lookat-point
	const Vec3& GetLookAt() const;

	//! Sets the camera's lookat-point
	void SetLookAt(const Vec3& lookat);

	//! Returns the camera's up vector
	const Vec3& GetUpVector() const;

	//! Returns whether the camera is active
	bool IsActive() const;

	//! Sets whether the camera is active
	void SetActive(bool active);

	//! Returns the camera's Field Of View
	float GetFOV() const;

	//! Sets the camera's Field Of View
	void SetFOV(float fov);

	//! Returns whether the camera automatically adjusts aspect ratio based on the Viewport
	bool GetAutoAspect() const;

	//! Sets whether the camera automatically adjusts aspect ratio based on the Viewport
	void SetAutoAspect(bool autoAspect);

	//! Returns the camera's aspect ratio
	float GetAspect() const;

	//! Sets the camera's aspect ratio
	void SetAspect(float aspect);

	//! Returns the camera's near clipping distance
	float GetNearClipDistance() const;

	//! Sets the camera's near clipping distance
	void SetNearClipDistance(float near);

	//! Returns the camera's far clipping distance
	float GetFarClipDistance() const;

	//! Sets the camera's far clipping distance
	void SetFarClipDistance(float far);

	//! Returns the camera's view frustum
	const Frustum& GetFrustum() const;

	//! Returns the projection matrix
	const Mat4& GetProjectionMatrix() const;

	//! Returns the view matrix
	const Mat4& GetViewMatrix() const;

	//! Loads the view and projection matrix
	virtual void Render();

	const Vec3& GetPosition() const { return mPosition; }

protected:

	void UpdateProjectionMatrix();
	void UpdateViewMatrix();
	void UpdateFrustum();

	Vec3 mPosition;
	Vec3 mLookAt;
	Vec3 mUp;

	Mat4 mView;
	Mat4 mProjection;

	Frustum mFrustum;

	float mFOV;
	float mAspect;

	float mNear;
	float mFar;

	bool mActive		: 1;
	bool mAutoAspect	: 1;
};

} // namespace crown

