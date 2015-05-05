/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "camera.h"
#include "types.h"
#include "math_utils.h"
#include "quaternion.h"
#include "unit.h"
#include "scene_graph.h"
#include "error.h"
#include "vector4.h"
#include "frustum.h"

namespace crown
{

Camera::Camera(SceneGraph& sg, UnitId id, ProjectionType::Enum type, float near, float far)
	: _scene_graph(sg)
	, _unit_id(id)
	, _projection_type(type)
	, _near(near)
	, _far(far)
{
	update_projection_matrix();
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
	Matrix4x4 view = _scene_graph.world_pose(_scene_graph.get(_unit_id));
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

	Matrix4x4 mvp = view_matrix() * _projection;
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

	Vector3 ndc = pos * (view_matrix() * _projection);

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
