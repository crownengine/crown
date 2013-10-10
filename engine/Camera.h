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
#include "Matrix4x4.h"
#include "IdTable.h"

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

typedef Id UnitId;
class Quaternion;
class Unit;

/// Represents the point of view into the game world.
struct Camera
{
	void					create(UnitId unit, int32_t node, const Vector3& pos, const Quaternion& rot);

	Vector3					local_position() const;
	Quaternion				local_rotation() const;
	Matrix4x4				local_pose() const;

	Vector3					world_position() const;
	Quaternion				world_rotation() const;
	Matrix4x4				world_pose() const;

	void					set_local_position(const Vector3& pos);
	void					set_local_rotation(const Quaternion& rot);
	void					set_local_pose(const Matrix4x4& pose);

	void					set_projection_type(ProjectionType::Enum type);
	ProjectionType::Enum	projection_type() const;

	float					fov() const;
	void					set_fov(float fov);

	float					aspect() const;
	void					set_aspect(float aspect);

	float					near_clip_distance() const;
	void					set_near_clip_distance(float near);
	float					far_clip_distance() const;
	void					set_far_clip_distance(float far);

public:

	void					update_projection_matrix();
	void					update_frustum();

public:

	UnitId					m_unit;
	int32_t					m_node;
	Matrix4x4				m_local_pose;
	Matrix4x4				m_world_pose;

	ProjectionType::Enum	m_projection_type;
	Matrix4x4				m_projection;

	Frustum					m_frustum;
	float					m_FOV;
	float					m_aspect;
	float					m_near;
	float					m_far;
};

} // namespace crown

