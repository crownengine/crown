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
#include "PhysicsTypes.h"

#include "PxPhysics.h"
#include "PxScene.h"
#include "PxRigidActor.h"

namespace crown
{

class Vector3;
class Quaternion;
class Matrix4x4;
class Unit;
class PhysicsGraph;

struct Actor
{
					Actor(PhysicsGraph& pg, int32_t node, ActorType::Enum type);

	Vector3			local_position() const;
	Quaternion		local_rotation() const;
	Matrix4x4		local_pose() const;

	Vector3			world_position() const;
	Quaternion		world_rotation() const;
	Matrix4x4		world_pose() const;

	void			set_local_position(Unit* unit, const Vector3& pos);
	void			set_local_rotation(Unit* unit, const Quaternion& rot);
	void			set_local_pose(Unit* unit, const Matrix4x4& pose);

	void			create_sphere(Vector3& position, float radius);

public:

	physx::PxRigidActor* m_actor;

	PhysicsGraph& m_physics_graph;
	int32_t m_node;
	ActorType::Enum m_type;
};

} // namespace crown