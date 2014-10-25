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

#include "camera.h"
#include "types.h"
#include "math_utils.h"
#include "quaternion.h"
#include "unit.h"
#include "scene_graph.h"
#include "assert.h"
#include "vector4.h"
#include "frustum.h"

namespace crown
{

Camera::Camera(SceneGraph& sg, int32_t node, ProjectionType::Enum type, float near, float far)
	: _scene_graph(sg)
	, _node(node)
	, _projection_type(type)
	, _near(near)
	, _far(far)
{
	update_projection_matrix();
}

Vector3 Camera::local_position() const
{
	return _scene_graph.local_position(_node);
}

Quaternion Camera::local_rotation() const
{
	return _scene_graph.local_rotation(_node);
}

Matrix4x4 Camera::local_pose() const
{
	return _scene_graph.local_pose(_node);
}

Vector3 Camera::world_position() const
{
	return _scene_graph.world_position(_node);
}

Quaternion Camera::world_rotation() const
{
	return _scene_graph.world_rotation(_node);
}

Matrix4x4 Camera::world_pose() const
{
	return _scene_graph.world_pose(_node);
}

void Camera::set_local_position(Unit* unit, const Vector3& pos)
{
	unit->set_local_position(_node, pos);
}

void Camera::set_local_rotation(Unit* unit, const Quaternion& rot)
{
	unit->set_local_rotation(_node, rot);
}

void Camera::set_local_pose(Unit* unit, const Matrix4x4& pose)
{
	unit->set_local_pose(_node, pose);
}

void Camera::set_projection_type(ProjectionType::Enum type)
{
	_projection_type = type;
	update_projection_matrix();
}

ProjectionType::Enum Camera::projection_type() const
{
	return _projection_type;
}

const Matrix4x4& Camera::projection_matrix() const
{
	return _projection;
}

Matrix4x4 Camera::view_matrix() const
{
	Matrix4x4 view = camera->world_pose();
	matrix4x4::invert(view);
	return view;
}

float Camera::fov() const
{
	return _FOV;
}

void Camera::set_fov(float fov)
{
	_FOV = fov;
	update_projection_matrix();
}

float Camera::aspect() const
{
	return _aspect;
}

void Camera::set_aspect(float aspect)
{
	_aspect = aspect;
	update_projection_matrix();
}

float Camera::near_clip_distance() const
{
	return _near;
}

void Camera::set_near_clip_distance(float near)
{
	_near = near;
	update_projection_matrix();
}

float Camera::far_clip_distance() const
{
	return _far;
}

void Camera::set_far_clip_distance(float far)
{
	_far = far;
	update_projection_matrix();
}

void Camera::set_orthographic_metrics(float left, float right, float bottom, float top)
{
	_left = left;
	_right = right;
	_bottom = bottom;
	_top = top;

	update_projection_matrix();
}

void Camera::set_viewport_metrics(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
	_view_x = x;
	_view_y = y;
	_view_width = width;
	_view_height = height;
}

Vector3 Camera::screen_to_world(const Vector3& pos)
{
	using namespace matrix4x4;

	Matrix4x4 world_inv = world_pose();
	invert(world_inv);
	Matrix4x4 mvp = world_inv * _projection;
	invert(mvp);

	Vector4 ndc( (2 * (pos.x - 0)) / _view_width - 1,
				 (2 * (_view_height - pos.y)) / _view_height - 1,
				 (2 * pos.z) - 1, 1);

	Vector4 tmp = ndc * mvp;
	tmp *= 1.0f / tmp.w;

	return Vector3(tmp.x, tmp.y, tmp.z);
}

Vector3 Camera::world_to_screen(const Vector3& pos)
{
	using namespace matrix4x4;

	Matrix4x4 world_inv = world_pose();
	invert(world_inv);

	Vector3 ndc = pos * (world_inv * _projection);

	return Vector3( (_view_x + _view_width * (ndc.x + 1.0f)) / 2.0f,
					(_view_y + _view_height * (ndc.y + 1.0f)) / 2.0f,
					(ndc.z + 1.0f) / 2.0f);
}

void Camera::update_projection_matrix()
{
	switch (_projection_type)
	{
		case ProjectionType::ORTHOGRAPHIC:
		{
			matrix4x4::set_orthographic(_projection, _left, _right, _bottom, _top, _near, _far);
			break;
		}
		case ProjectionType::PERSPECTIVE:
		{
			matrix4x4::set_perspective(_projection, _FOV, _aspect, _near, _far);
			break;
		}
		default:
		{
			CE_FATAL("Oops, unknown projection type");
			break;
		}
	}
}

void Camera::update_frustum()
{
	// TODO
	//m_frustum.from_matrix(_projection * m_view);
}

} // namespace crown
