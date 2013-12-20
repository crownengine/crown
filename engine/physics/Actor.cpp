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
#include "Device.h"
#include "Physics.h"
#include "Log.h"
#include "SceneGraph.h"
#include "PxPhysicsAPI.h"

using physx::PxRigidDynamicFlag;
using physx::PxMat44;
using physx::PxTransform;
using physx::PxActorFlag;
using physx::PxVec3;
using physx::PxReal;
using physx::PxRigidBody;
using physx::PxRigidDynamic;
using physx::PxPlaneGeometry;
using physx::PxSphereGeometry;
using physx::PxBoxGeometry;
using physx::PxRigidBodyExt;

using physx::PxD6Joint;
using physx::PxD6JointCreate;
using physx::PxD6Axis;
using physx::PxD6Motion;

namespace crown
{
	
//-----------------------------------------------------------------------------
Actor::Actor(PxScene* scene, SceneGraph& sg, int32_t node, ActorType::Enum type, const Vector3& pos, const Quaternion& rot)
	: m_scene(scene)
	, m_scene_graph(sg)
	, m_node(node)
	, m_type(type)
{
	Matrix4x4 m(rot, pos);
	m.transpose();
	PxMat44 pose((PxReal*)(m.to_float_ptr()));

	switch (type)
	{
		case ActorType::STATIC:
		{
			m_actor = device()->physx()->createRigidStatic(PxTransform(pose));
			break;
		}
		case ActorType::DYNAMIC_PHYSICAL:
		case ActorType::DYNAMIC_KINEMATIC:
		{
			m_actor = device()->physx()->createRigidDynamic(PxTransform(pose));

			if (type == ActorType::DYNAMIC_KINEMATIC)
			{
				static_cast<PxRigidDynamic*>(m_actor)->setRigidDynamicFlag(PxRigidDynamicFlag::eKINEMATIC, true);
			}
			break;
		}
		default:
		{
			CE_FATAL("Oops, unknown actor type");
			break;
		}
	}

	m_actor->userData = this;
	m_mat = device()->physx()->createMaterial(0.5f, 0.5f, 0.5f);
	
	create_box(Vector3(0, 0, 0), .5, .5, .5);

	PxRigidBodyExt::setMassAndUpdateInertia(*static_cast<PxRigidDynamic*>(m_actor), 500.0f);

	PxD6Joint* joint = PxD6JointCreate(*device()->physx(), m_actor, PxTransform(pose), NULL, PxTransform(pose));
	joint->setMotion(PxD6Axis::eX, PxD6Motion::eFREE);
	joint->setMotion(PxD6Axis::eY, PxD6Motion::eFREE);
	//joint->setMotion(PxD6Axis::eZ, PxD6Motion::eFREE);
	//joint->setMotion(PxD6Axis::eSWING1, PxD6Motion::eFREE);
	joint->setMotion(PxD6Axis::eSWING2, PxD6Motion::eFREE);

	m_scene->addActor(*m_actor);
}

//-----------------------------------------------------------------------------
Actor::~Actor()
{
	if (m_actor)
	{
		m_scene->removeActor(*m_actor);
		m_actor->release();
	}
}

//-----------------------------------------------------------------------------
void Actor::create_sphere(const Vector3& position, float radius)
{
	m_actor->createShape(PxSphereGeometry(radius), *m_mat);
}

//-----------------------------------------------------------------------------
void Actor::create_box(const Vector3& position, float a, float b, float c)
{
	m_actor->createShape(PxBoxGeometry(a, b, c), *m_mat);
}

//-----------------------------------------------------------------------------
void Actor::create_plane(const Vector3& /*position*/, const Vector3& /*normal*/)
{
	m_actor->createShape(PxPlaneGeometry(), *m_mat);
}

//-----------------------------------------------------------------------------
void Actor::enable_gravity()
{
	m_actor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, false);
}

//-----------------------------------------------------------------------------
void Actor::disable_gravity()
{
	m_actor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
}

//-----------------------------------------------------------------------------
bool Actor::is_static() const
{
	return m_type == ActorType::STATIC;
}

//-----------------------------------------------------------------------------
bool Actor::is_dynamic() const
{
	return m_type == ActorType::DYNAMIC_PHYSICAL || m_type == ActorType::DYNAMIC_KINEMATIC;
}

//-----------------------------------------------------------------------------
bool Actor::is_kinematic() const
{
	return m_type == ActorType::DYNAMIC_KINEMATIC;
}

//-----------------------------------------------------------------------------
bool Actor::is_physical() const
{
	return m_type == ActorType::DYNAMIC_PHYSICAL;
}

//-----------------------------------------------------------------------------
float Actor::linear_damping() const
{
	return ((PxRigidDynamic*)m_actor)->getLinearDamping();
}

//-----------------------------------------------------------------------------
void Actor::set_linear_damping(float rate)
{
	((PxRigidDynamic*)m_actor)->setLinearDamping(rate);
}

//-----------------------------------------------------------------------------
float Actor::angular_damping() const
{
	return ((PxRigidDynamic*)m_actor)->getAngularDamping();
}

//-----------------------------------------------------------------------------
void Actor::set_angular_damping(float rate)
{
	((PxRigidDynamic*)m_actor)->setAngularDamping(rate);
}

//-----------------------------------------------------------------------------
Vector3 Actor::linear_velocity() const
{
	PxVec3 vel = ((PxRigidBody*)m_actor)->getLinearVelocity();
	Vector3 velocity(vel.x, vel.y, vel.z);
	return velocity;
}

//-----------------------------------------------------------------------------
void Actor::set_linear_velocity(const Vector3& vel)
{
	PxVec3 velocity(vel.x, vel.y, vel.z);
	((PxRigidBody*)m_actor)->setLinearVelocity(velocity);
}

//-----------------------------------------------------------------------------
Vector3 Actor::angular_velocity() const
{
	PxVec3 vel = ((PxRigidBody*)m_actor)->getAngularVelocity();
	Vector3 velocity(vel.x, vel.y, vel.z);
	return velocity;
}

//-----------------------------------------------------------------------------
void Actor::set_angular_velocity(const Vector3& vel)
{
	PxVec3 velocity(vel.x, vel.y, vel.z);
	((PxRigidBody*)m_actor)->setAngularVelocity(velocity);
}

//-----------------------------------------------------------------------------
bool Actor::is_sleeping()
{
	return ((PxRigidDynamic*)m_actor)->isSleeping();
}

//-----------------------------------------------------------------------------
void Actor::wake_up()
{
	((PxRigidDynamic*)m_actor)->wakeUp();
}

//-----------------------------------------------------------------------------
void Actor::update_pose()
{
	// Read world pose
	Matrix4x4 wp = m_scene_graph.world_pose(m_node);
	const PxMat44 pose((PxReal*) (wp.to_float_ptr()));
	const PxTransform world_transform(pose);

	switch (m_type)
	{
		case ActorType::DYNAMIC_KINEMATIC:
		{
			static_cast<PxRigidDynamic*>(m_actor)->setKinematicTarget(world_transform);
			break;
		}
		default: break;
	}
}

//-----------------------------------------------------------------------------
void Actor::update(const Matrix4x4& pose)
{
	if (m_type == ActorType::DYNAMIC_PHYSICAL)
	{
		m_scene_graph.set_world_pose(m_node, pose);
	}
}

} // namespace crown