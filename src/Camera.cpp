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
#include "Types.h"
#include "MathUtils.h"

namespace crown
{

//-----------------------------------------------------------------------------
Camera::Camera(const Vec3& position, float fov, float aspect) :
	m_position(position),
	m_look_at(0, 0, -1),
	m_up(0, 1, 0),

	m_rot_factor(0.0f, 0.0f),
	m_angle_x(0.0f),
	m_angle_y(0.0f),

	m_FOV(fov),
	m_aspect(aspect),
	m_near(0.1f),
	m_far(1000.0f)
{
	update_projection_matrix();
	update_view_matrix();
	update_frustum();
}

//-----------------------------------------------------------------------------
const Vec3& Camera::position() const
{
	return m_position;
}

//-----------------------------------------------------------------------------
void Camera::set_position(const Vec3& position)
{
	m_position = position;

	update_view_matrix();
}

//-----------------------------------------------------------------------------
const Vec3& Camera::look_at() const
{
	return m_look_at;
}

//-----------------------------------------------------------------------------
void Camera::set_look_at(const Vec3& lookat)
{
	m_look_at = lookat;

	update_view_matrix();
}

//-----------------------------------------------------------------------
void Camera::set_rotation(const float x, const float y)
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
	m_up = m * up;

	set_look_at(look);
}

//-----------------------------------------------------------------------------
const Vec3& Camera::up() const
{
	return m_up;
}

//-----------------------------------------------------------------------------
float Camera::fov() const
{
	return m_FOV;
}

//-----------------------------------------------------------------------------
void Camera::set_fov(float fov)
{
	m_FOV = fov;

	update_projection_matrix();
}

//-----------------------------------------------------------------------------
float Camera::aspect() const
{
	return m_aspect;
}

//-----------------------------------------------------------------------------
void Camera::set_aspect(float aspect)
{
	m_aspect = aspect;

	update_projection_matrix();
}

//-----------------------------------------------------------------------------
float Camera::near_clip_distance() const
{
	return m_near;
}

//-----------------------------------------------------------------------------
void Camera::set_near_clip_distance(float near)
{
	m_near = near;

	update_projection_matrix();
}

//-----------------------------------------------------------------------------
float Camera::far_clip_distance() const
{
	return m_far;
}

//-----------------------------------------------------------------------------
void Camera::set_far_clip_distance(float far)
{
	m_far = far;

	update_projection_matrix();
}

//-----------------------------------------------------------------------------
const Mat4& Camera::projection_matrix() const
{
	return m_projection;
}

//-----------------------------------------------------------------------------
const Mat4& Camera::view_matrix() const
{
	return m_view;
}

//-----------------------------------------------------------------------------
const Frustum& Camera::frustum() const
{
	return m_frustum;
}

//-----------------------------------------------------------------------------
void Camera::update_projection_matrix()
{
	m_projection.build_projection_perspective_rh(m_FOV, m_aspect, m_near, m_far);
}

//-----------------------------------------------------------------------------
void Camera::update_view_matrix()
{
	m_view.build_look_at_rh(m_position, m_position + m_look_at, m_up);
}

//-----------------------------------------------------------------------------
void Camera::update_frustum()
{
	m_frustum.from_matrix(m_projection * m_view);
}

//-----------------------------------------------------------------------------
void Camera::move_forward(float meters)
{
	set_position(m_position + m_look_at * meters);
}

//-----------------------------------------------------------------------
void Camera::move_backward(float meters)
{
	set_position(m_position + m_look_at * -meters);
}

//-----------------------------------------------------------------------
void Camera::strafe_left(float meters)
{
	Vec3 left = m_up.cross(m_look_at);
	left.normalize();

	set_position(m_position + left * meters);
}

//-----------------------------------------------------------------------
void Camera::strafe_right(float meters)
{
	Vec3 left = m_up.cross(m_look_at);
	left.normalize();

	set_position(m_position + left * -meters);
}

} // namespace crown

