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

#include "Actor.h"
#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix4x4.h"
#include "Unit.h"
#include "PhysicsGraph.h"
#include "Device.h"
#include "Physics.h"

#include "PxPhysicsAPI.h"

namespace crown
{
	
//-----------------------------------------------------------------------------
Actor::Actor(PhysicsGraph& pg, int32_t node, ActorType::Enum type)
	: m_physics_graph(pg)
	, m_node(node)
	, m_type(type)
{
	Vector3 pos = m_physics_graph.m_local_poses[node].translation();
	physx::PxVec3 position(pos.x, pos.y, pos.z);

	switch (type)
	{
		case ActorType::STATIC:
		{
			m_actor = device()->physx()->createRigidStatic(physx::PxTransform(position));
			break;
		}
		case ActorType::DYNAMIC:
		{
			m_actor = device()->physx()->createRigidDynamic(physx::PxTransform(position));
			break;
		}
		default:
		{
			CE_FATAL("Unable to recognize actor type");
		}
	}
}

//-----------------------------------------------------------------------------
Vector3	Actor::local_position() const
{
	return m_physics_graph.local_position(m_node);
}

//-----------------------------------------------------------------------------
Quaternion Actor::local_rotation() const
{
	return m_physics_graph.local_rotation(m_node);
}

//-----------------------------------------------------------------------------
Matrix4x4 Actor::local_pose() const
{
	return m_physics_graph.local_pose(m_node);
}

//-----------------------------------------------------------------------------
Vector3	Actor::world_position() const
{
	return m_physics_graph.world_position(m_node);
}

//-----------------------------------------------------------------------------
Quaternion Actor::world_rotation() const
{
	return m_physics_graph.world_rotation(m_node);
}

//-----------------------------------------------------------------------------
Matrix4x4 Actor::world_pose() const
{
	return m_physics_graph.world_pose(m_node);
}

//-----------------------------------------------------------------------------
void Actor::set_local_position(Unit* unit, const Vector3& pos)
{
	unit->set_local_position(m_node, pos);
}

//-----------------------------------------------------------------------------
void Actor::set_local_rotation(Unit* unit, const Quaternion& rot)
{
	unit->set_local_rotation(m_node, rot);
}

//-----------------------------------------------------------------------------
void Actor::set_local_pose(Unit* unit, const Matrix4x4& pose)
{
	unit->set_local_pose(m_node, pose);
}

//-----------------------------------------------------------------------------
void Actor::create_sphere(Vector3& position, float radius)
{
	// FIXME FIXME FIXME
	physx::PxMaterial* mat = device()->physx()->createMaterial(0.5f, 0.5f, 0.1f);
	m_actor->createShape(physx::PxSphereGeometry(radius), *mat);
	// physx::PxRigidBodyExt::updateMassAndInertia(*m_actor, 1.0f);
}

} // namespace crown