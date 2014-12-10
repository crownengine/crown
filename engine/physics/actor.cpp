/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "actor.h"
#include "device.h"
#include "log.h"
#include "matrix4x4.h"
#include "mesh_resource.h"
#include "physics_resource.h"
#include "quaternion.h"
#include "resource_manager.h"
#include "scene_graph.h"
#include "unit.h"
#include "vector3.h"
#include "world.h"
#include "physics_world.h"
#include "quaternion.h"
#include "string_utils.h"

#include "PxPhysicsAPI.h"
#include "PxCooking.h"
#include "PxDefaultStreams.h"

using physx::PxActorFlag;
using physx::PxActorType;
using physx::PxBoxGeometry;
using physx::PxCapsuleGeometry;
using physx::PxConvexFlag;
using physx::PxConvexMesh;
using physx::PxConvexMeshDesc;
using physx::PxConvexMeshGeometry;
using physx::PxD6Axis;
using physx::PxD6Joint;
using physx::PxD6JointCreate;
using physx::PxD6Motion;
using physx::PxDefaultMemoryInputData;
using physx::PxDefaultMemoryOutputStream;
using physx::PxFilterData;
using physx::PxForceMode;
using physx::PxMat44;
using physx::PxPlaneGeometry;
using physx::PxQuat;
using physx::PxReal;
using physx::PxRigidActor;
using physx::PxRigidBody;
using physx::PxRigidBodyExt;
using physx::PxRigidBodyFlag;
using physx::PxRigidDynamic;
using physx::PxRigidDynamicFlag;
using physx::PxRigidStatic;
using physx::PxShape;
using physx::PxShapeFlag;
using physx::PxSphereGeometry;
using physx::PxTransform;
using physx::PxU16;
using physx::PxU32;
using physx::PxVec3;
using physx::PxTransformFromPlaneEquation;
using physx::PxPlane;

namespace crown
{

Actor::Actor(PhysicsWorld& pw, const PhysicsResource* res, uint32_t actor_idx, SceneGraph& sg, int32_t node, UnitId unit_id)
	: m_world(pw)
	, m_resource(res)
	, m_index(actor_idx)
	, m_scene_graph(sg)
	, m_node(node)
	, m_unit(unit_id)
{
	create_objects();
}

Actor::~Actor()
{
	destroy_objects();
}

void Actor::create_objects()
{
	const PhysicsActor* actor = physics_resource::actor(m_resource, m_index);

	PxScene* scene = m_world.physx_scene();
	PxPhysics* physics = m_world.physx_physics();
	const PhysicsConfigResource* config = m_world.resource();
	const PhysicsActor2* actor_class = physics_config_resource::actor(config, actor->actor_class);

	// Create rigid body
	const PxMat44 pose((PxReal*) matrix4x4::to_float_ptr(m_scene_graph.world_pose(m_node)));

	if (actor_class->flags & PhysicsActor2::DYNAMIC)
	{
		m_actor = physics->createRigidDynamic(PxTransform(pose));
		if (actor_class->flags & PhysicsActor2::KINEMATIC)
		{
			static_cast<PxRigidDynamic*>(m_actor)->setRigidDynamicFlag(PxRigidDynamicFlag::eKINEMATIC, true);
		}

		// PxD6Joint* joint = PxD6JointCreate(*physics, m_actor, PxTransform(pose), NULL, PxTransform(pose));
		// joint->setMotion(PxD6Axis::eX, PxD6Motion::eFREE);
		// joint->setMotion(PxD6Axis::eY, PxD6Motion::eFREE);
		// joint->setMotion(PxD6Axis::eSWING2, PxD6Motion::eFREE);
	}
	else
	{
		m_actor = physics->createRigidStatic(PxTransform(pose));
	}

	// Create shapes
	uint32_t shape_index = physics_resource::shape_index(m_resource, m_index);
	for (uint32_t i = 0; i < actor->num_shapes; i++)
	{
		const PhysicsShape* shape = physics_resource::shape(m_resource, shape_index);
		const PhysicsShape2* shape_class = physics_config_resource::shape(config, shape->shape_class);
		const PhysicsMaterial* material = physics_config_resource::material(config, shape->material);

		PxMaterial* mat = physics->createMaterial(material->static_friction, material->dynamic_friction, material->restitution);

		PxShape* px_shape = NULL;
		switch(shape->type)
		{
			case PhysicsShapeType::SPHERE:
			{
				px_shape = m_actor->createShape(PxSphereGeometry(shape->data_0), *mat);
				break;
			}
			case PhysicsShapeType::CAPSULE:
			{
				px_shape = m_actor->createShape(PxCapsuleGeometry(shape->data_0, shape->data_1), *mat);
				break;
			}
			case PhysicsShapeType::BOX:
			{
				px_shape = m_actor->createShape(PxBoxGeometry(shape->data_0, shape->data_1, shape->data_2), *mat);
				break;
			}
			case PhysicsShapeType::PLANE:
			{
				px_shape = m_actor->createShape(PxPlaneGeometry(), *mat);
				break;
			}
			case PhysicsShapeType::CONVEX_MESH:
			{
				// MeshResource* resource = (MeshResource*) device()->resource_manager()->get(MESH_TYPE, shape->resource.name);

				// PxConvexMeshDesc convex_mesh_desc;
				// convex_mesh_desc.points.count		= resource->num_vertices();
				// convex_mesh_desc.points.stride		= sizeof(PxVec3);
				// convex_mesh_desc.points.data		= (PxVec3*) resource->vertices();
				// convex_mesh_desc.triangles.count	= resource->num_indices();
				// convex_mesh_desc.triangles.stride	= 3 * sizeof(PxU16);
				// convex_mesh_desc.triangles.data 	= (PxU16*) resource->indices();
				// convex_mesh_desc.flags				= PxConvexFlag::eCOMPUTE_CONVEX;
				// convex_mesh_desc.vertexLimit		= MAX_PHYSX_VERTICES;

				// PxDefaultMemoryOutputStream buf;
				// if(!m_world.physx_cooking()->cookConvexMesh(convex_mesh_desc, buf))
				// 	CE_FATAL("");
				// PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
				// PxConvexMesh* convex_mesh = physics->createConvexMesh(input);

				// px_shape = m_actor->createShape(PxConvexMeshGeometry(convex_mesh), *mat);
				break;
			}
			default:
			{
				CE_FATAL("Oops, unknown shape type");
			}
		}

		// Setup shape pose
		if (shape->type == PhysicsShapeType::PLANE)
		{
			px_shape->setLocalPose(PxTransformFromPlaneEquation(
				PxPlane(shape->data_0, shape->data_1, shape->data_2, shape->data_3)));
		}
		else
		{
			px_shape->setLocalPose(PxTransform(
				PxVec3(shape->position.x, shape->position.y, shape->position.z),
				PxQuat(shape->rotation.x, shape->rotation.y, shape->rotation.z, shape->rotation.w).getNormalized()));
		}

		// Setup collision filters
		PxFilterData filter_data;
		filter_data.word0 = physics_config_resource::filter(config, shape_class->collision_filter)->me;
		filter_data.word1 = physics_config_resource::filter(config, shape_class->collision_filter)->mask;
		px_shape->setSimulationFilterData(filter_data);

		if (shape_class->trigger)
		{
			px_shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
			px_shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
		}

		shape_index++;
	}

	if (is_dynamic())
	{
		PxRigidBodyExt::updateMassAndInertia(*static_cast<PxRigidBody*>(m_actor), actor->mass);
	}
	m_actor->userData = this;
	scene->addActor(*m_actor);
}

void Actor::destroy_objects()
{
	if (m_actor)
	{
		m_world.physx_scene()->removeActor(*m_actor);
		m_actor->release();
	}
}

Vector3 Actor::world_position() const
{
	const PxTransform tr = m_actor->getGlobalPose();
	return Vector3(tr.p.x, tr.p.y, tr.p.z);
}

Quaternion Actor::world_rotation() const
{
	const PxTransform tr = m_actor->getGlobalPose();
	return Quaternion(tr.q.x, tr.q.y, tr.q.z, tr.q.w);
}

Matrix4x4 Actor::world_pose() const
{
	const PxTransform tr = m_actor->getGlobalPose();
	return Matrix4x4(Quaternion(tr.q.x, tr.q.y, tr.q.z, tr.q.w), Vector3(tr.p.x, tr.p.y, tr.p.z));
}

void Actor::teleport_world_position(const Vector3& p)
{
	PxTransform tr = m_actor->getGlobalPose();
	tr.p.x = p.x;
	tr.p.y = p.y;
	tr.p.z = p.z;
	m_actor->setGlobalPose(tr);
}

void Actor::teleport_world_rotation(const Quaternion& r)
{
	PxTransform tr = m_actor->getGlobalPose();
	tr.q.x = r.x;
	tr.q.y = r.y;
	tr.q.z = r.z;
	tr.q.w = r.w;
	m_actor->setGlobalPose(tr);
}

void Actor::teleport_world_pose(const Matrix4x4& m)
{
	using namespace matrix4x4;

	const PxVec3 x(m.x.x, m.x.y, m.x.z);
	const PxVec3 y(m.y.x, m.y.y, m.y.z);
	const PxVec3 z(m.z.x, m.z.y, m.z.z);
	const PxVec3 t(translation(m).x, translation(m).y, translation(m).z);
	m_actor->setGlobalPose(PxTransform(PxMat44(x, y, z, t)));
}

Vector3 Actor::center_of_mass() const
{
	if (is_static())
		return Vector3(0, 0, 0);

	const PxTransform tr = static_cast<PxRigidBody*>(m_actor)->getCMassLocalPose();
	return Vector3(tr.p.x, tr.p.y, tr.p.z);
}

void Actor::enable_gravity()
{
	m_actor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, false);
}

void Actor::disable_gravity()
{
	m_actor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
}

void Actor::enable_collision()
{
	// const PxU32 num_shapes = m_actor->getNbShapes();
	// PxU32 idx = 0;

	// while (idx != num_shapes)
	// {
	// 	PxShape* shapes[8];
	// 	const PxU32 written = m_actor->getShapes(shapes, 8, idx);

	// 	for (PxU32 i = 0; i < written; i++)
	// 	{
	// 		PxFilterData fdata;
	// 		fdata.word0 = 0;
	// 		fdata.word1 = 0;
	// 		shapes[i]->setSimulationFilterData(fdata);
	// 	}

	// 	idx += written;
	// }
}

void Actor::disable_collision()
{
}

void Actor::set_collision_filter(const char* name)
{
	set_collision_filter(murmur32(name, strlen(name)));
}

void Actor::set_collision_filter(StringId32 filter)
{
	const PhysicsCollisionFilter* pcf = physics_config_resource::filter(m_world.resource(), filter);

	const PxU32 num_shapes = m_actor->getNbShapes();
	PxU32 idx = 0;

	while (idx != num_shapes)
	{
		PxShape* shapes[8];
		const PxU32 written = m_actor->getShapes(shapes, 8, idx);

		for (PxU32 i = 0; i < written; i++)
		{
			PxFilterData fdata;
			fdata.word0 = pcf->me;
			fdata.word1 = pcf->mask;
			shapes[i]->setSimulationFilterData(fdata);
		}

		idx += written;
	}
}

void Actor::set_kinematic(bool kinematic)
{
	if (is_static())
		return;

	static_cast<PxRigidBody*>(m_actor)->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, kinematic);
}

void Actor::move(const Vector3& pos)
{
	if (!is_kinematic())
		return;

	const PxVec3 position(pos.x, pos.y, pos.z);
	static_cast<PxRigidDynamic*>(m_actor)->setKinematicTarget(PxTransform(position));
}

bool Actor::is_static() const
{
	return m_actor->getType() & PxActorType::eRIGID_STATIC;
}

bool Actor::is_dynamic() const
{
	return m_actor->getType() & PxActorType::eRIGID_DYNAMIC;
}

bool Actor::is_kinematic() const
{
	if (!is_dynamic())
		return false;

	return static_cast<PxRigidDynamic*>(m_actor)->getRigidDynamicFlags() & PxRigidDynamicFlag::eKINEMATIC;
}

bool Actor::is_nonkinematic() const
{
	return is_dynamic() && !is_kinematic();
}

float Actor::linear_damping() const
{
	if (is_static())
		return 0;

	return static_cast<PxRigidDynamic*>(m_actor)->getLinearDamping();
}

void Actor::set_linear_damping(float rate)
{
	if (is_static())
		return;

	static_cast<PxRigidDynamic*>(m_actor)->setLinearDamping(rate);
}

float Actor::angular_damping() const
{
	if (is_static())
		return 0;

	return static_cast<PxRigidDynamic*>(m_actor)->getAngularDamping();
}

void Actor::set_angular_damping(float rate)
{
	if (is_static())
		return;

	static_cast<PxRigidDynamic*>(m_actor)->setAngularDamping(rate);
}

Vector3 Actor::linear_velocity() const
{
	if (is_static())
		return Vector3(0, 0, 0);

	const PxVec3 vel = static_cast<PxRigidBody*>(m_actor)->getLinearVelocity();
	return Vector3(vel.x, vel.y, vel.z);
}

void Actor::set_linear_velocity(const Vector3& vel)
{
	if (!is_nonkinematic())
		return;

	const PxVec3 velocity(vel.x, vel.y, vel.z);
	static_cast<PxRigidBody*>(m_actor)->setLinearVelocity(velocity);
}

Vector3 Actor::angular_velocity() const
{
	if (is_static())
		return Vector3(0, 0, 0);

	const PxVec3 vel = static_cast<PxRigidBody*>(m_actor)->getAngularVelocity();
	return Vector3(vel.x, vel.y, vel.z);
}

void Actor::set_angular_velocity(const Vector3& vel)
{
	if (!is_nonkinematic())
		return;

	const PxVec3 velocity(vel.x, vel.y, vel.z);
	static_cast<PxRigidBody*>(m_actor)->setAngularVelocity(velocity);
}

void Actor::add_impulse(const Vector3& impulse)
{
	if (!is_nonkinematic())
		return;

	static_cast<PxRigidDynamic*>(m_actor)->addForce(PxVec3(impulse.x, impulse.y, impulse.z), PxForceMode::eIMPULSE);
}

void Actor::add_impulse_at(const Vector3& impulse, const Vector3& pos)
{
	if (!is_nonkinematic())
		return;

	PxRigidBodyExt::addForceAtPos(*static_cast<PxRigidDynamic*>(m_actor),
									   PxVec3(impulse.x, impulse.y, impulse.z),
									   PxVec3(pos.x, pos.y, pos.z),
									   PxForceMode::eIMPULSE);
}

void Actor::add_torque_impulse(const Vector3& i)
{
	if (!is_nonkinematic())
		return;

	static_cast<PxRigidBody*>(m_actor)->addTorque(PxVec3(i.x, i.y, i.z), PxForceMode::eIMPULSE);
}

void Actor::push(const Vector3& vel, float mass)
{
	add_impulse(vel * mass);
}

void Actor::push_at(const Vector3& vel, float mass, const Vector3& pos)
{
	add_impulse_at(vel * mass, pos);
}

bool Actor::is_sleeping()
{
	if (is_static())
		return true;

	return static_cast<PxRigidDynamic*>(m_actor)->isSleeping();
}

void Actor::wake_up()
{
	if (is_static())
		return;

	static_cast<PxRigidDynamic*>(m_actor)->wakeUp();
}

UnitId Actor::unit_id() const
{
	return m_unit;
}

Unit* Actor::unit()
{
	return (m_unit.id == INVALID_ID) ? NULL : m_world.world().get_unit(m_unit);
}

void Actor::update(const Matrix4x4& pose)
{
	m_scene_graph.set_world_pose(m_node, pose);
}

} // namespace crown
