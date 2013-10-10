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

#include "Camera.h"
#include "Types.h"
#include "MathUtils.h"
#include "Quaternion.h"
#include "Unit.h"
#include "Assert.h"

namespace crown
{

//-----------------------------------------------------------------------
void Camera::create(UnitId unit, int32_t node, const Vector3& pos, const Quaternion& rot)
{
	m_unit = unit;
	m_node = node;
	m_projection_type = ProjectionType::PERSPECTIVE;

	set_local_position(pos);
	set_local_rotation(rot);
	update_projection_matrix();
}

//-----------------------------------------------------------------------------
Vector3 Camera::local_position() const
{
	return m_local_pose.translation();
}

//-----------------------------------------------------------------------------
Quaternion Camera::local_rotation() const
{
	return Quaternion(Vector3(1, 0, 0), 0.0f);
}

//-----------------------------------------------------------------------------
Matrix4x4 Camera::local_pose() const
{
	return m_local_pose;
}

//-----------------------------------------------------------------------------
Vector3 Camera::world_position() const
{
	return m_world_pose.translation();
}

//-----------------------------------------------------------------------------
Quaternion Camera::world_rotation() const
{
	return Quaternion(Vector3(1, 0, 0), 0.0f);
}

//-----------------------------------------------------------------------------
Matrix4x4 Camera::world_pose() const
{
	return m_world_pose;
}

//-----------------------------------------------------------------------------
void Camera::set_local_position(const Vector3& pos)
{
	m_local_pose.set_translation(pos);
}

//-----------------------------------------------------------------------------
void Camera::set_local_rotation(const Quaternion& rot)
{
	Matrix4x4& local_pose = m_local_pose;

	Vector3 local_translation = local_pose.translation();
	local_pose = rot.to_mat4();
	local_pose.set_translation(local_translation);
}

//-----------------------------------------------------------------------------
void Camera::set_local_pose(const Matrix4x4& pose)
{
	m_local_pose = pose;
}

//-----------------------------------------------------------------------
void Camera::set_projection_type(ProjectionType::Enum type)
{
	m_projection_type = type;
	update_projection_matrix();
}

//-----------------------------------------------------------------------
ProjectionType::Enum Camera::projection_type() const
{
	return m_projection_type;
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
void Camera::set_orthographic_metrics(uint16_t width, uint16_t height)
{
	m_width = width;
	m_height = height;
	update_projection_matrix();
}

//-----------------------------------------------------------------------------
void Camera::update_projection_matrix()
{
	switch (m_projection_type)
	{
		case ProjectionType::ORTHOGRAPHIC:
		{
			m_projection.build_projection_ortho_rh(m_width, m_height, m_near, m_far);
			break;
		}
		case ProjectionType::PERSPECTIVE:
		{
			m_projection.build_projection_perspective_rh(m_FOV, m_aspect, m_near, m_far);
			break;
		}
		default:
		{
			CE_FATAL("Oops, unknown projection type");
			break;
		}
	}
}

//-----------------------------------------------------------------------------
void Camera::update_frustum()
{
	// TODO
	//m_frustum.from_matrix(m_projection * m_view);
}

} // namespace crown
