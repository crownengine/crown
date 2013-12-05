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
#include "Log.h"
#include "SceneGraph.h"

#include "PxPhysicsAPI.h"

namespace crown
{
	
//-----------------------------------------------------------------------------
Actor::Actor(PhysicsGraph& pg, int32_t sg_node, ActorType::Enum type, const Vector3& pos, const Quaternion& rot)
	: m_physics_graph(pg)
	, m_sg_node(sg_node)
	, m_type(type)
{
	Matrix4x4 m(rot, pos);
	physx::PxMat44 pose((physx::PxReal*)(m.to_float_ptr()));

	switch (type)
	{
		case ActorType::STATIC:
		{
			m_actor = device()->physx()->createRigidStatic(physx::PxTransform(pose));
			break;
		}
		case ActorType::DYNAMIC:
		{
			m_actor = device()->physx()->createRigidDynamic(physx::PxTransform(pose));
			Log::d("Created dynamic");
			break;
		}
		default:
		{
			CE_FATAL("Unable to recognize actor type");
		}
	}

	m_mat = device()->physx()->createMaterial(0.5f, 0.5f, 0.5f);

	// FIXME
	create_sphere(Vector3(0, 0, 0), 2.0f);
}

//-----------------------------------------------------------------------------
Actor::~Actor()
{
	if (m_actor)
	{
		m_actor->release();
	}
}

//-----------------------------------------------------------------------------
void Actor::create_sphere(const Vector3& position, float radius)
{
	Shape shape(m_actor->createShape(physx::PxSphereGeometry(radius), *m_mat));
	m_physics_graph.create(m_sg_node, shape);
}

//-----------------------------------------------------------------------------
void Actor::create_box(const Vector3& position, float a, float b, float c)
{
	Shape shape(m_actor->createShape(physx::PxBoxGeometry(a, b, c), *m_mat));
	m_physics_graph.create(m_sg_node, shape);
}

//-----------------------------------------------------------------------------
void Actor::create_plane(const Vector3& position, const Vector3& normal)
{
	// TODO
}

//-----------------------------------------------------------------------------
void Actor::enable_gravity()
{
	m_actor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, false);
}

//-----------------------------------------------------------------------------
void Actor::disable_gravity()
{
	m_actor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
}

//-----------------------------------------------------------------------------
bool Actor::is_static() const
{
	return m_type == ActorType::STATIC;
}

//-----------------------------------------------------------------------------
bool Actor::is_dynamic() const
{
	return m_type == ActorType::DYNAMIC;
}

//-----------------------------------------------------------------------------
float Actor::linear_damping() const
{
	return ((physx::PxRigidDynamic*)m_actor)->getLinearDamping();
}

//-----------------------------------------------------------------------------
void Actor::set_linear_damping(float rate)
{
	((physx::PxRigidDynamic*)m_actor)->setLinearDamping(rate);
}

//-----------------------------------------------------------------------------
float Actor::angular_damping() const
{
	return ((physx::PxRigidDynamic*)m_actor)->getAngularDamping();
}

//-----------------------------------------------------------------------------
void Actor::set_angular_damping(float rate)
{
	((physx::PxRigidDynamic*)m_actor)->setAngularDamping(rate);
}

//-----------------------------------------------------------------------------
Vector3 Actor::linear_velocity() const
{
	physx::PxVec3 vel = ((physx::PxRigidBody*)m_actor)->getLinearVelocity();
	Vector3 velocity(vel.x, vel.y, vel.z);
	return velocity;
}

//-----------------------------------------------------------------------------
void Actor::set_linear_velocity(const Vector3& vel)
{
	physx::PxVec3 velocity(vel.x, vel.y, vel.z);
	((physx::PxRigidBody*)m_actor)->setLinearVelocity(velocity);
}

//-----------------------------------------------------------------------------
Vector3 Actor::angular_velocity() const
{
	physx::PxVec3 vel = ((physx::PxRigidBody*)m_actor)->getAngularVelocity();
	Vector3 velocity(vel.x, vel.y, vel.z);
	return velocity;
}

//-----------------------------------------------------------------------------
void Actor::set_angular_velocity(const Vector3& vel)
{
	physx::PxVec3 velocity(vel.x, vel.y, vel.z);
	((physx::PxRigidBody*)m_actor)->setAngularVelocity(velocity);
}

//-----------------------------------------------------------------------------
bool Actor::is_sleeping()
{
	return ((physx::PxRigidDynamic*)m_actor)->isSleeping();
}

//-----------------------------------------------------------------------------
void Actor::wake_up()
{
	((physx::PxRigidDynamic*)m_actor)->wakeUp();
}


} // namespace crown