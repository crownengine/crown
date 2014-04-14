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
#include "SceneGraph.h"
#include "Assert.h"
#include "Vector4.h"
#include "Frustum.h"

namespace crown
{

//-----------------------------------------------------------------------------
Camera::Camera(SceneGraph& sg, int32_t node)
	: m_scene_graph(sg)
	, m_node(node)
	, m_projection_type(ProjectionType::PERSPECTIVE)
{
}

//-----------------------------------------------------------------------------
Vector3 Camera::local_position() const
{
	return m_scene_graph.local_position(m_node);
}

//-----------------------------------------------------------------------------
Quaternion Camera::local_rotation() const
{
	return m_scene_graph.local_rotation(m_node);
}

//-----------------------------------------------------------------------------
Matrix4x4 Camera::local_pose() const
{
	return m_scene_graph.local_pose(m_node);
}

//-----------------------------------------------------------------------------
Vector3 Camera::world_position() const
{
	return m_scene_graph.world_position(m_node);
}

//-----------------------------------------------------------------------------
Quaternion Camera::world_rotation() const
{
	return m_scene_graph.world_rotation(m_node);
}

//-----------------------------------------------------------------------------
Matrix4x4 Camera::world_pose() const
{
	return m_scene_graph.world_pose(m_node);
}

//-----------------------------------------------------------------------------
void Camera::set_local_position(Unit* unit, const Vector3& pos)
{
	unit->set_local_position(m_node, pos);
}

//-----------------------------------------------------------------------------
void Camera::set_local_rotation(Unit* unit, const Quaternion& rot)
{
	unit->set_local_rotation(m_node, rot);
}

//-----------------------------------------------------------------------------
void Camera::set_local_pose(Unit* unit, const Matrix4x4& pose)
{
	unit->set_local_pose(m_node, pose);
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

//-----------------------------------------------------------------------
const Matrix4x4& Camera::projection_matrix() const
{
	return m_projection;
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
void Camera::set_orthographic_metrics(float left, float right, float bottom, float top)
{
	m_left = left;
	m_right = right;
	m_bottom = bottom;
	m_top = top;

	update_projection_matrix();
}

//-----------------------------------------------------------------------------
void Camera::set_viewport_metrics(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
	m_view_x = x;
	m_view_y = y;
	m_view_width = width;
	m_view_height = height;
}

//-----------------------------------------------------------------------------
Vector3 Camera::screen_to_world(const Vector3& pos)
{
	using namespace matrix4x4;

	Matrix4x4 world_inv = world_pose();
	invert(world_inv);
	Matrix4x4 mvp = m_projection * world_inv;
	invert(mvp);

	Vector4 ndc( (2 * (pos.x - 0)) / m_view_width - 1,
				 (2 * (m_view_height - pos.y)) / m_view_height - 1,
				 (2 * pos.z) - 1, 1);

	Vector4 tmp = mvp * ndc;
	tmp *= 1.0 / tmp.w;

	return Vector3(tmp.x, tmp.y, tmp.z);
}

//-----------------------------------------------------------------------------
Vector3 Camera::world_to_screen(const Vector3& pos)
{
	using namespace matrix4x4;

	Matrix4x4 world_inv = world_pose();
	invert(world_inv);

	Vector3 ndc = (m_projection * world_inv) * pos;

	return Vector3( (m_view_x + m_view_width * (ndc.x + 1.0)) / 2.0,
					(m_view_y + m_view_height * (ndc.y + 1.0)) / 2.0,
					(ndc.z + 1.0) / 2.0);
}

//-----------------------------------------------------------------------------
void Camera::update_projection_matrix()
{
	switch (m_projection_type)
	{
		case ProjectionType::ORTHOGRAPHIC:
		{
			matrix4x4::set_orthographic_rh(m_projection, m_left, m_right, m_bottom, m_top, m_near, m_far);
			break;
		}
		case ProjectionType::PERSPECTIVE:
		{
			matrix4x4::set_perspective_rh(m_projection, m_FOV, m_aspect, m_near, m_far);
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
