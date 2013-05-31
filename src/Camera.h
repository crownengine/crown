/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
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
#include "Mat3.h"
#include "Vec3.h"

namespace crown
{

/// Represents the point of view into the game world.
class Camera
{
public:

	/// Construct the camera placed at the given world-space @a position
	/// with the given @a fov field of view and @a aspect ratio.
					Camera(const Vec3& position, float fov, float aspect);

	/// Returns the world-space position of the camera 
	const Vec3&		position() const;

	/// Sets the world-space @a position of the camera
	void			set_position(const Vec3& position);

	/// Returns the lookat-point of the camera
	const Vec3&		look_at() const;

	/// Sets the lookat-point of the camera
	void			set_look_at(const Vec3& lookat);

	/// Sets the rotation of the camera about the world's @a x axis and @a y axis
	void			set_rotation(const float x, const float y);

	/// Returns the up-vector of the camera
	const Vec3&		up() const;

	/// Returns the field of view of the camera in degrees
	float			fov() const;

	/// Sets the field of view of the camera
	void			set_fov(float fov);

	/// Returns the aspect ratio of the camera
	float			aspect() const;

	/// Sets the aspect ration of the camera
	void			set_aspect(float aspect);

	/// Returns the near clipping distance of the camera
	float			near_clip_distance() const;

	/// Sets the near clipping distance of the camera
	void			set_near_clip_distance(float near);

	/// Returns the far clipping distance of the camera
	float			far_clip_distance() const;

	/// Sets the far clipping distance of the camera
	void			set_far_clip_distance(float far);

	/// Returns the view-frustum of the camera
	const Frustum&	frustum() const;

	/// Returns the renderer-independent projection matrix used by the camera
	const Mat4&		projection_matrix() const;

	/// Returns the renderer-independent view matrix used by the camera
	const Mat4&		view_matrix() const;

	/// Moves the camera towards look direction by @a meters meters
	void			move_forward(float meters);

	/// Moves the camera towards the opposite look direction by @a meters meters
	void			move_backward(float meters);

	/// Moves the camera along the axis perpendicular to the look direction by @a meters meters
	void			strafe_left(float meters);

	/// Moves the camera along the axis perpendicular to the look direction by @a meters meters
	void			strafe_right(float meters);

protected:

	void			update_projection_matrix();
	void			update_view_matrix();
	void			update_frustum();

	Vec3			m_position;
	Vec3			m_look_at;
	Vec3			m_up;

	Vec2			m_rot_factor;
	float			m_angle_x;
	float			m_angle_y;

	Mat4			m_view;
	Mat4			m_projection;

	Frustum			m_frustum;

	float			m_FOV;
	float			m_aspect;

	float			m_near;
	float			m_far;
};

} // namespace crown

