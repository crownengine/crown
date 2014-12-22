/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "physics_world.h"
#include "vector3.h"
#include "actor.h"
#include "device.h"
#include "quaternion.h"
#include "scene_graph.h"
#include "controller.h"
#include "joint.h"
#include "physics_callback.h"
#include "proxy_allocator.h"
#include "memory.h"
#include "string_utils.h"
#include "actor.h"
#include "resource_manager.h"
#include "raycast.h"
#include "unit.h"
#include "config.h"
#include "world.h"
#include "debug_line.h"
#include "color4.h"
#include "int_setting.h"
#include "physics.h"
#include "matrix4x4.h"
#include "log.h"
#include "PxPhysicsAPI.h"

using physx::PxSceneDesc;
using physx::PxVec3;
using physx::PxTransform;
using physx::PxQuat;
using physx::PxHalfPi;

using physx::PxPlaneGeometry;
using physx::PxMaterial;
using physx::PxShape;
using physx::PxRigidStatic;
using physx::PxActiveTransform;
using physx::PxU32;
using physx::PxSceneFlag;
using physx::PxFilterFlags;
using physx::PxFilterData;
using physx::PxPairFlags;
using physx::PxFilterObjectAttributes;
using physx::PxFilterObjectIsTrigger;
using physx::PxPairFlag;
using physx::PxFilterFlag;
using physx::PxSceneLimits;
using physx::PxVisualizationParameter;
using physx::PxSphereGeometry;
using physx::PxCapsuleGeometry;
using physx::PxBoxGeometry;
using physx::PxRenderBuffer;
using physx::PxDebugLine;

namespace crown
{

namespace physics_globals
{
	using physx::PxFoundation;
	using physx::PxPhysics;
	using physx::PxCooking;
	using physx::PxCookingParams;
	using physx::PxTolerancesScale;
	using physx::PxAllocatorCallback;
	using physx::PxErrorCallback;
	using physx::PxErrorCode;

	class PhysXAllocator : public PxAllocatorCallback
	{
	public:

		PhysXAllocator()
			: m_backing("physics", default_allocator())
		{
		}

		void* allocate(size_t size, const char*, const char*, int)
		{
			return m_backing.allocate(size, 16);
		}

		void deallocate(void* p)
		{
			m_backing.deallocate(p);
		}

	private:

		ProxyAllocator m_backing;
	};

	class PhysXError : public PxErrorCallback
	{
	public:

		void reportError(PxErrorCode::Enum code, const char* message, const char* /*file*/, int /*line*/)
		{
			switch (code)
			{
				case PxErrorCode::eDEBUG_INFO:
				{
					CE_LOGI("PhysX: %s", message);
					break;
				}
				case PxErrorCode::eDEBUG_WARNING:
				case PxErrorCode::ePERF_WARNING:
				{
					CE_LOGW("PhysX: %s", message);
					break;
				}
				case PxErrorCode::eINVALID_PARAMETER:
				case PxErrorCode::eINVALID_OPERATION:
				case PxErrorCode::eOUT_OF_MEMORY:
				case PxErrorCode::eINTERNAL_ERROR:
				case PxErrorCode::eABORT:
				{
					CE_ASSERT(false, "PhysX: %s", message);
					break;
				}
				default:
				{
					CE_FATAL("Oops, unknown physx error");
					break;
				}
			}
		}
	};

	PxFilterFlags FilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0,
									PxFilterObjectAttributes attributes1, PxFilterData filterData1,
									PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
	{
		// let triggers through
		if(PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
		{
			pairFlags = PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_TOUCH_LOST;
			return PxFilterFlag::eDEFAULT;
		}

		// trigger the contact callback for pairs (A,B) where
		// the filtermask of A contains the ID of B and vice versa.
		if((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
		{
			pairFlags |= PxPairFlag::eCONTACT_DEFAULT
			 		  | PxPairFlag::eNOTIFY_TOUCH_FOUND
					  | PxPairFlag::eNOTIFY_TOUCH_LOST
					  | PxPairFlag::eNOTIFY_CONTACT_POINTS;
			return PxFilterFlag::eDEFAULT;
		}

		return PxFilterFlag::eSUPPRESS;
	}

	// Global PhysX objects
	static PhysXAllocator* s_px_allocator;
	static PhysXError* s_px_error;
	static PxFoundation* s_foundation;
	static PxPhysics* s_physics;
	static PxCooking* s_cooking;

	void init()
	{
		s_px_allocator = CE_NEW(default_allocator(), PhysXAllocator)();
		s_px_error = CE_NEW(default_allocator(), PhysXError)();

		s_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, *s_px_allocator, *s_px_error);
		CE_ASSERT(s_foundation, "Unable to create PhysX Foundation");

		s_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *s_foundation, physx::PxTolerancesScale());
		CE_ASSERT(s_physics, "Unable to create PhysX Physics");

		bool extension = PxInitExtensions(*s_physics);
		CE_ASSERT(extension, "Unable to initialize PhysX Extensions");
		CE_UNUSED(extension);

		s_cooking = PxCreateCooking(PX_PHYSICS_VERSION, *s_foundation, PxCookingParams(PxTolerancesScale()));
		CE_ASSERT(s_cooking, "Unable to create PhysX Cooking");
	}

	void shutdown()
	{
		PxCloseExtensions();
		s_cooking->release();
		s_physics->release();
		s_foundation->release();

		CE_DELETE(default_allocator(), s_px_error);
		CE_DELETE(default_allocator(), s_px_allocator);
	}
} // namespace physics_globals

PhysicsWorld::PhysicsWorld(World& world)
	: m_world(world)
	, m_scene(NULL)
	, m_buffer(m_hits, 64)
	, m_actors_pool(default_allocator(), CE_MAX_ACTORS, sizeof(Actor), CE_ALIGNOF(Actor))
	, m_controllers_pool(default_allocator(), CE_MAX_CONTROLLERS, sizeof(Controller), CE_ALIGNOF(Controller))
	, m_joints_pool(default_allocator(), CE_MAX_JOINTS, sizeof(Joint), CE_ALIGNOF(Joint))
	, m_raycasts_pool(default_allocator(), CE_MAX_RAYCASTS, sizeof(Raycast), CE_ALIGNOF(Raycast))
	, m_events(default_allocator())
	, m_callback(m_events)
{
	// Create the scene
	PxSceneLimits scene_limits;
	scene_limits.maxNbActors = CE_MAX_ACTORS;
	CE_ASSERT(scene_limits.isValid(), "Scene limits is not valid");

	PxSceneDesc scene_desc(physics_globals::s_physics->getTolerancesScale());
	scene_desc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	scene_desc.limits = scene_limits;
	scene_desc.filterShader = physics_globals::FilterShader;
	scene_desc.simulationEventCallback = &m_callback;
	scene_desc.flags = 	PxSceneFlag::eENABLE_ACTIVETRANSFORMS
					  | PxSceneFlag::eENABLE_KINEMATIC_STATIC_PAIRS
					  | PxSceneFlag::eENABLE_KINEMATIC_PAIRS;

	if(!scene_desc.cpuDispatcher)
	{
		m_cpu_dispatcher = physx::PxDefaultCpuDispatcherCreate(1);
		CE_ASSERT(m_cpu_dispatcher != NULL, "Failed to create PhysX cpu dispatcher");

		scene_desc.cpuDispatcher = m_cpu_dispatcher;
	}

	CE_ASSERT(scene_desc.isValid(), "Scene is not valid");
	m_scene = physics_globals::s_physics->createScene(scene_desc);

	// Create controller manager
	m_controller_manager = PxCreateControllerManager(*m_scene);
	CE_ASSERT(m_controller_manager != NULL, "Failed to create PhysX controller manager");

	m_resource = (PhysicsConfigResource*) device()->resource_manager()->get("physics_config", "global");

#if defined(CROWN_DEBUG)
	m_scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1);
	m_scene->setVisualizationParameter(PxVisualizationParameter::eACTOR_AXES, 1);
	m_scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1);
#endif
}

PhysicsWorld::~PhysicsWorld()
{
	m_cpu_dispatcher->release();
	m_controller_manager->release();
	m_scene->release();
}

ActorId	PhysicsWorld::create_actor(const ActorResource* ar, SceneGraph& sg, int32_t node, UnitId unit_id)
{
	Actor* actor = CE_NEW(m_actors_pool, Actor)(*this, ar, sg, node, unit_id);
	return id_array::create(m_actors, actor);
}

void PhysicsWorld::destroy_actor(ActorId id)
{
	CE_DELETE(m_actors_pool, id_array::get(m_actors, id));
	id_array::destroy(m_actors, id);
}

ControllerId PhysicsWorld::create_controller(const ControllerResource* cr, SceneGraph& sg, int32_t node)
{
	Controller* controller = CE_NEW(m_controllers_pool, Controller)(cr, sg, node, physics_globals::s_physics, m_controller_manager);
	return id_array::create(m_controllers, controller);
}

void PhysicsWorld::destroy_controller(ControllerId id)
{
	CE_DELETE(m_controllers_pool, id_array::get(m_controllers, id));
	id_array::destroy(m_controllers, id);
}

JointId	PhysicsWorld::create_joint(const JointResource* jr, const Actor& actor_0, const Actor& actor_1)
{
	Joint* joint = CE_NEW(m_joints_pool, Joint)(physics_globals::s_physics, jr, actor_0, actor_1);
	return id_array::create(m_joints, joint);
}

void PhysicsWorld::destroy_joint(JointId id)
{
	CE_DELETE(m_joints_pool, id_array::get(m_joints, id));
	id_array::destroy(m_joints, id);
}

RaycastId PhysicsWorld::create_raycast(CollisionMode::Enum mode, CollisionType::Enum filter)
{
	Raycast* raycast = CE_NEW(m_raycasts_pool, Raycast)(m_scene, mode, filter);
	return id_array::create(m_raycasts, raycast);
}

void PhysicsWorld::destroy_raycast(RaycastId id)
{
	CE_DELETE(m_raycasts_pool, id_array::get(m_raycasts, id));
	id_array::destroy(m_raycasts, id);
}

Actor* PhysicsWorld::get_actor(ActorId id)
{
	return id_array::get(m_actors, id);
}

Controller* PhysicsWorld::get_controller(ControllerId id)
{
	return id_array::get(m_controllers, id);
}

Raycast* PhysicsWorld::get_raycast(RaycastId id)
{
	return id_array::get(m_raycasts, id);
}

Vector3 PhysicsWorld::gravity() const
{
	PxVec3 g = m_scene->getGravity();
	return Vector3(g.x, g.y, g.z);
}

void PhysicsWorld::set_gravity(const Vector3& g)
{
	m_scene->setGravity(PxVec3(g.x, g.y, g.z));
}

void PhysicsWorld::overlap_test(CollisionType::Enum filter, ShapeType::Enum type,
								const Vector3& pos, const Quaternion& rot, const Vector3& size, Array<Actor*>& actors)
{
	PxTransform transform(PxVec3(pos.x, pos.y, pos.z), PxQuat(rot.x, rot.y, rot.z, rot.w));

	switch(type)
	{
		case ShapeType::SPHERE:
		{
			PxSphereGeometry geometry(size.x);
			m_scene->overlap(geometry, transform, m_buffer);
			break;
		}
		case ShapeType::CAPSULE:
		{
			PxCapsuleGeometry geometry(size.x, size.y);
			m_scene->overlap(geometry, transform, m_buffer);
			break;
		}
		case ShapeType::BOX:
		{
			PxBoxGeometry geometry(size.x, size.y, size.z);
			m_scene->overlap(geometry, transform, m_buffer);
			break;
		}
		default: CE_FATAL("Only spheres, capsules and boxs are supported in overlap test"); break;
	}

	for (uint32_t i = 0; i < m_buffer.getNbAnyHits(); i++)
	{
		PxOverlapHit oh = m_buffer.getAnyHit(i);

		array::push_back(actors, (Actor*)(oh.actor->userData));
	}
}

void PhysicsWorld::update(float dt)
{
	// Run with fixed timestep
	m_scene->simulate(1.0 / 60.0);

	while (!m_scene->fetchResults());

	// Update transforms
	PxU32 num_active_transforms;
	const PxActiveTransform* active_transforms = m_scene->getActiveTransforms(num_active_transforms);

	// Update each actor with its new transform
	for (PxU32 i = 0; i < num_active_transforms; i++)
	{
		// Actors with userData == NULL are controllers
		if (active_transforms[i].userData == NULL) continue;

		const PxTransform tr = active_transforms[i].actor2World;
		const Vector3 pos(tr.p.x, tr.p.y, tr.p.z);
		const Quaternion rot(tr.q.x, tr.q.y, tr.q.z, tr.q.w);

		static_cast<Actor*>(active_transforms[i].userData)->update(Matrix4x4(rot, pos));
	}

	// Update controllers
	for (uint32_t i = 0; i < id_array::size(m_controllers); i++)
	{
		m_controllers[i]->update();
	}
}

void PhysicsWorld::draw_debug(DebugLine& lines)
{
	CE_UNUSED(lines);
#if CROWN_DEBUG
	const PxRenderBuffer& rb = m_scene->getRenderBuffer();
	for(PxU32 i = 0; i < rb.getNbLines(); i++)
	{
		const PxDebugLine& pxline = rb.getLines()[i];
		lines.add_line(Vector3(pxline.pos0.x, pxline.pos0.y, pxline.pos0.z),
						Vector3(pxline.pos1.x, pxline.pos1.y, pxline.pos1.z),
						Color4(pxline.color0));
	}

	lines.commit();
	lines.clear();
#endif // CROWN_DEBUG
}

PxPhysics* PhysicsWorld::physx_physics() { return physics_globals::s_physics; }
PxCooking* PhysicsWorld::physx_cooking() { return physics_globals::s_cooking; }
PxScene* PhysicsWorld::physx_scene() { return m_scene; }

} // namespace crown
