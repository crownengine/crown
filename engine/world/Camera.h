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
#include "MathTypes.h"
#include "WorldTypes.h"

namespace crown
{

struct ProjectionType
{
	enum Enum
	{
		ORTHOGRAPHIC,
		PERSPECTIVE
	};
};

struct Quaternion;
struct Unit;
struct SceneGraph;

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

	SceneGraph& m_scene_graph;
	int32_t m_node;

	ProjectionType::Enum m_projection_type;
	Matrix4x4 m_projection;

	Frustum m_frustum;
	float m_FOV;
	float m_aspect;
	float m_near;
	float m_far;

	// Orthographic projection only
	float m_left;
	float m_right;
	float m_bottom;
	float m_top;

	uint16_t m_view_x;
	uint16_t m_view_y;
	uint16_t m_view_width;
	uint16_t m_view_height;
};

} // namespace crown
