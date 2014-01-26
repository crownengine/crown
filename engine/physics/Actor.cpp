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
#include "Log.h"
#include "PhysicsResource.h"
#include "SceneGraph.h"
#include "PxPhysicsAPI.h"

#include "Device.h"
#include "ResourceManager.h"


using physx::PxActorFlag;
using physx::PxActorType;
using physx::PxBoxGeometry;
using physx::PxD6Axis;
using physx::PxD6Joint;
using physx::PxD6JointCreate;
using physx::PxD6Motion;
using physx::PxFilterData;
using physx::PxForceMode;
using physx::PxMat44;
using physx::PxPlaneGeometry;
using physx::PxReal;
using physx::PxRigidActor;
using physx::PxRigidBody;
using physx::PxRigidBodyExt;
using physx::PxRigidDynamic;
using physx::PxRigidDynamicFlag;
using physx::PxRigidStatic;
using physx::PxShape;
using physx::PxShapeFlag;
using physx::PxSphereGeometry;
using physx::PxTransform;
using physx::PxU32;
using physx::PxVec3;

namespace crown
{

//-----------------------------------------------------------------------------
Actor::Actor(const PhysicsResource* res, const PhysicsConfigResource* config, uint32_t index, PxPhysics* physics, PxScene* scene, SceneGraph& sg, int32_t node, const Vector3& pos, const Quaternion& rot)
	: m_resource(res)
	, m_config(config)
	, m_index(index)
	, m_scene(scene)
	, m_scene_graph(sg)
	, m_node(node)
{
	const PhysicsActor& actor = m_resource->actor(m_index);
	const PhysicsActor2& actor_class = config->actor(actor.actor_class);

	const PxMat44 pose((PxReal*) (sg.world_pose(node).to_float_ptr()));

	if (actor_class.flags & PhysicsActor2::DYNAMIC)
	{
		m_actor = physics->createRigidDynamic(PxTransform(pose));
		if (actor_class.flags & PhysicsActor2::KINEMATIC)
		{
			static_cast<PxRigidDynamic*>(m_actor)->setRigidDynamicFlag(PxRigidDynamicFlag::eKINEMATIC, true);
		}

		PxD6Joint* joint = PxD6JointCreate(*physics, m_actor, PxTransform(pose), NULL, PxTransform(pose));
		joint->setMotion(PxD6Axis::eX, PxD6Motion::eFREE);
		joint->setMotion(PxD6Axis::eY, PxD6Motion::eFREE);
		joint->setMotion(PxD6Axis::eSWING2, PxD6Motion::eFREE);
	}
	else
	{
		m_actor = physics->createRigidStatic(PxTransform(pose));
	}

	m_actor->userData = this;

	create_shapes(res, config, physics);

	// FIXME collisions works only if enable_collision() is called here first
	// collision enabled by default
	enable_collision();

	m_actor->setActorFlag(PxActorFlag::eSEND_SLEEP_NOTIFIES, true);
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
void Actor::create_shapes(const PhysicsResource* res, const PhysicsConfigResource* config, PxPhysics* physics)
{
	const PhysicsActor& actor = m_resource->actor(m_index);
	uint32_t shape_index = m_resource->shape_index(m_index);
	for (uint32_t i = 0; i < actor.num_shapes; i++)
	{
		const PhysicsShape& shape = m_resource->shape(shape_index);
		const PhysicsShape2& shape_class = config->shape(shape.shape_class);
		const PhysicsMaterial& material = config->material(shape.material);

		PxMaterial* mat = physics->createMaterial(material.static_friction, material.dynamic_friction, material.restitution);

		PxShape* px_shape = NULL;
		switch(shape.type)
		{
			case PhysicsShapeType::SPHERE:
			{
				px_shape = m_actor->createShape(PxSphereGeometry(shape.data_0), *mat);
				break;
			}
			case PhysicsShapeType::BOX:
			{
				px_shape = m_actor->createShape(PxBoxGeometry(shape.data_0, shape.data_1, shape.data_2), *mat);
				break;
			}
			case PhysicsShapeType::PLANE:
			{
				// FIXME
				px_shape = m_actor->createShape(PxPlaneGeometry(), *mat);
				break;
			}
			default:
			{
				CE_FATAL("Oops, unknown shape type");
			}
		}

		if (shape_class.trigger)
		{
			px_shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
			px_shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);		
		}

		shape_index++;
	}

	// PxFilterData filter;
	// filter.word0 = (PxU32) m_group;
	// filter.word1 = (PxU32) m_mask;
	// shape->SetSimulationFilterData()
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
void Actor::enable_collision()
{
	// PxFilterData filter_data;
	// filter_data.word0 = (PxU32) m_group;
	// filter_data.word1 = (PxU32) m_mask;

	// const PxU32 num_shapes = m_actor->getNbShapes();

	// PxShape** shapes = (PxShape**) default_allocator().allocate((sizeof(PxShape*) * num_shapes));
	// m_actor->getShapes(shapes, num_shapes);

	// for(PxU32 i = 0; i < num_shapes; i++)
	// {
	// 	PxShape* shape = shapes[i];
	// 	shape->setSimulationFilterData(filter_data);
	// }

	// default_allocator().deallocate(shapes);
}

//-----------------------------------------------------------------------------
void Actor::disable_collision()
{
	// PxFilterData filter_data;
	// filter_data.word0 = (PxU32) CollisionGroup::GROUP_0;
	// filter_data.word1 = (PxU32) CollisionGroup::GROUP_0;

	// const PxU32 num_shapes = m_actor->getNbShapes();

	// PxShape** shapes = (PxShape**) default_allocator().allocate((sizeof(PxShape*) * num_shapes));
	// m_actor->getShapes(shapes, num_shapes);

	// for(PxU32 i = 0; i < num_shapes; i++)
	// {
	// 	PxShape* shape = shapes[i];
	// 	shape->setSimulationFilterData(filter_data);
	// }

	// default_allocator().deallocate(shapes);	
}

//-----------------------------------------------------------------------------
void Actor::set_kinematic()
{
	static_cast<PxRigidDynamic*>(m_actor)->setRigidDynamicFlag(PxRigidDynamicFlag::eKINEMATIC, true);
}

//-----------------------------------------------------------------------------
void Actor::clear_kinematic()
{
	static_cast<PxRigidDynamic*>(m_actor)->setRigidDynamicFlag(PxRigidDynamicFlag::eKINEMATIC, false);
}

//-----------------------------------------------------------------------------
bool Actor::is_static() const
{
	return m_actor->getType() & PxActorType::eRIGID_STATIC;
}

//-----------------------------------------------------------------------------
bool Actor::is_dynamic() const
{
	return m_actor->getType() & PxActorType::eRIGID_DYNAMIC;
}

//-----------------------------------------------------------------------------
bool Actor::is_kinematic() const
{
	if (!is_dynamic()) return false;
	return static_cast<PxRigidDynamic*>(m_actor)->getRigidDynamicFlags() & PxRigidDynamicFlag::eKINEMATIC;
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
void Actor::add_impulse(const Vector3& impulse)
{
	Vector3 p = m_scene_graph.world_pose(m_node).translation();

	PxRigidBodyExt::addForceAtPos(*static_cast<PxRigidDynamic*>(m_actor),
								  PxVec3(impulse.x, impulse.y, impulse.z),
								  PxVec3(p.x, p.y, p.z),
								  PxForceMode::eIMPULSE,
								  true);
}

//-----------------------------------------------------------------------------
void Actor::add_impulse_at(const Vector3& impulse, const Vector3& pos)
{
	PxRigidBodyExt::addForceAtLocalPos(*static_cast<PxRigidDynamic*>(m_actor),
									   PxVec3(impulse.x, impulse.y, impulse.z),
									   PxVec3(pos.x, pos.y, pos.z),
									   PxForceMode::eIMPULSE,
									   true);
}

//-----------------------------------------------------------------------------
void Actor::push(const Vector3& vel, const float mass)
{
	// FIXME FIXME FIXME
	Vector3 p = m_scene_graph.world_pose(m_node).translation();

	Vector3 mq(vel.x * mass, vel.y * mass, vel.z * mass);
	Vector3 f(mq.x / 0.017, mq.y / 0.017, mq.z / 0.017);

	PxRigidBodyExt::addForceAtPos(*static_cast<PxRigidDynamic*>(m_actor),
								  PxVec3(f.x, f.y, f.z),
								  PxVec3(p.x, p.y, p.z));
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
StringId32 Actor::name()
{
	const PhysicsActor& a = m_resource->actor(m_index);
	return a.name;
}

//-----------------------------------------------------------------------------
void Actor::update(const Matrix4x4& pose)
{
	m_scene_graph.set_world_pose(m_node, pose);
}

} // namespace crown