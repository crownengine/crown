/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "math_types.h"
#include "world_types.h"

namespace crown
{

struct ProjectionType
{
	enum Enum
	{
		ORTHOGRAPHIC,
		PERSPECTIVE,

		COUNT
	};
};

/// Represents the point of view into the game world.
///
/// @ingroup World
struct Camera
{
	Camera(SceneGraph& sg, int32_t node, ProjectionType::Enum type, float near, float far);

	/// Returns the local position of the camera.
	Vector3 local_position() const;

	/// Returns the local rotation of the camera.
	Quaternion local_rotation() const;

	/// Returns the local pose of the camera.
	Matrix4x4 local_pose() const;

	/// Returns the world position of the camera.
	Vector3 world_position() const;

	/// Returns the world rotation of the camera.
	Quaternion world_rotation() const;

	/// Returns the world pose of the camera.
	Matrix4x4 world_pose() const;

	/// Sets the local position of the camera.
	void set_local_position(Unit* unit, const Vector3& pos);

	/// Sets the local rotation of the camera.
	void set_local_rotation(Unit* unit, const Quaternion& rot);

	/// Sets the local pose of the camera.
	void set_local_pose(Unit* unit, const Matrix4x4& pose);

	/// Sets the projection type of the camera.
	void set_projection_type(ProjectionType::Enum type);

	/// Returns the projection type of the camera.
	ProjectionType::Enum projection_type() const;

	/// Returns the projection matrix of the camera.
	const Matrix4x4& projection_matrix() const;

	/// Returns the view matrix of the camera.
	Matrix4x4 view_matrix() const;

	/// Returns the field-of-view of the camera in degrees.
	float fov() const;

	/// Sets the field-of-view of the camera in degrees.
	void set_fov(float fov);

	/// Returns the aspect ratio of the camera. (Perspective projection only.)
	float aspect() const;

	/// Sets the aspect ratio of the camera. (Perspective projection only.)
	void set_aspect(float aspect);

	/// Returns the near clip distance of the camera.
	float near_clip_distance() const;

	/// Sets the near clip distance of the camera.
	void set_near_clip_distance(float near);

	/// Returns the far clip distance of the camera.
	float far_clip_distance() const;

	/// Sets the far clip distance of the camera.
	void set_far_clip_distance(float far);

	/// Sets the coordinates for orthographic clipping planes. (Orthographic projection only.)
	void set_orthographic_metrics(float left, float right, float bottom, float top);

	/// Sets the coordinates for the camera viewport in pixels.
	void set_viewport_metrics(uint16_t x, uint16_t y, uint16_t width, uint16_t height);

	/// Returns @a pos from screen-space to world-space coordinates.
	Vector3 screen_to_world(const Vector3& pos);

	/// Returns @a pos from world-space to screen-space coordinates.
	Vector3 world_to_screen(const Vector3& pos);

public:

	void update_projection_matrix();
	void update_frustum();

public:

	SceneGraph& _scene_graph;
	int32_t _node;

	ProjectionType::Enum _projection_type;
	Matrix4x4 _projection;

	Frustum _frustum;
	float _FOV;
	float _aspect;
	float _near;
	float _far;

	// Orthographic projection only
	float _left;
	float _right;
	float _bottom;
	float _top;

	uint16_t _view_x;
	uint16_t _view_y;
	uint16_t _view_width;
	uint16_t _view_height;
};

} // namespace crown
