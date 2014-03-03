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

#include "PhysicsWorld.h"
#include "Vector3.h"
#include "Actor.h"
#include "Device.h"
#include "Quaternion.h"
#include "SceneGraph.h"
#include "Controller.h"
#include "Trigger.h"
#include "Joint.h"
#include "PhysicsCallback.h"
#include "ProxyAllocator.h"
#include "Hash.h"
#include "StringUtils.h"
#include "Actor.h"
#include "ResourceManager.h"

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

namespace crown
{

namespace physics_system
{
	using physx::PxFoundation;
	using physx::PxPhysics;
	using physx::PxCooking;
	using physx::PxCookingParams;
	using physx::PxTolerancesScale;
	using physx::PxAllocatorCallback;
	using physx::PxErrorCallback;
	using physx::PxErrorCode;

	//-----------------------------------------------------------------------------
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

	//-----------------------------------------------------------------------------
	class PhysXError : public PxErrorCallback
	{
	public:

		void reportError(PxErrorCode::Enum code, const char* message, const char* /*file*/, int /*line*/)
		{
			switch (code)
			{
				case PxErrorCode::eDEBUG_INFO:
				{
					Log::i("PhysX: %s", message);
					break;
				}
				case PxErrorCode::eDEBUG_WARNING: 
				case PxErrorCode::ePERF_WARNING:
				{
					Log::w("PhysX: %s", message);
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

	//-----------------------------------------------------------------------------
	PxFilterFlags FilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0, 
									PxFilterObjectAttributes attributes1, PxFilterData filterData1,
									PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
	{
		// let triggers through
		if(PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
		{
			pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
			return PxFilterFlag::eDEFAULT;
		}

		// generate contacts for all that were not filtered above
		pairFlags = PxPairFlag::eCONTACT_DEFAULT;

		// trigger the contact callback for pairs (A,B) where 
		// the filtermask of A contains the ID of B and vice versa.
		if((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
		{
			pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_CONTACT_POINTS;
			return PxFilterFlag::eDEFAULT;
		}

		return PxFilterFlag::eDEFAULT;
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
} // namespace physics_system

//-----------------------------------------------------------------------------
PhysicsWorld::PhysicsWorld()
	: m_scene(NULL)
	, m_actors_pool(default_allocator(), CE_MAX_ACTORS, sizeof(Actor), CE_ALIGNOF(Actor))
	, m_controllers_pool(default_allocator(), CE_MAX_CONTROLLERS, sizeof(Controller), CE_ALIGNOF(Controller))
	, m_triggers_pool(default_allocator(), CE_MAX_TRIGGERS, sizeof(Trigger), CE_ALIGNOF(Trigger))
	, m_joints_pool(default_allocator(), CE_MAX_JOINTS, sizeof(Joint), CE_ALIGNOF(Joint))
	, m_events(default_allocator())
	, m_callback(m_events)
{
	// Create the scene
	PxSceneLimits scene_limits;
	scene_limits.maxNbActors = CE_MAX_ACTORS;
	CE_ASSERT(scene_limits.isValid(), "Scene limits is not valid");

	PxSceneDesc scene_desc(physics_system::s_physics->getTolerancesScale());
	scene_desc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	scene_desc.limits = scene_limits;
	scene_desc.filterShader = physics_system::FilterShader;
	scene_desc.simulationEventCallback = &m_callback;
	scene_desc.flags = PxSceneFlag::eENABLE_ACTIVETRANSFORMS | PxSceneFlag::eENABLE_KINEMATIC_STATIC_PAIRS | PxSceneFlag::eENABLE_KINEMATIC_PAIRS;

	if(!scene_desc.cpuDispatcher)
	{
		m_cpu_dispatcher = physx::PxDefaultCpuDispatcherCreate(1);
		CE_ASSERT(m_cpu_dispatcher != NULL, "Failed to create PhysX cpu dispatcher");

		scene_desc.cpuDispatcher = m_cpu_dispatcher;
	}

	CE_ASSERT(scene_desc.isValid(), "Scene is not valid");
	m_scene = physics_system::s_physics->createScene(scene_desc);
	
	// Create controller manager
	m_controller_manager = PxCreateControllerManager(*m_scene);
	CE_ASSERT(m_controller_manager != NULL, "Failed to create PhysX controller manager");
}

//-----------------------------------------------------------------------------
PhysicsWorld::~PhysicsWorld()
{
	m_cpu_dispatcher->release();
	m_controller_manager->release();
	m_scene->release();
}

//-----------------------------------------------------------------------------
ActorId	PhysicsWorld::create_actor(const PhysicsResource* res, const uint32_t index, SceneGraph& sg, int32_t node)
{
	PhysicsConfigResource* config = (PhysicsConfigResource*) device()->resource_manager()->lookup("physics_config", "global");
	Actor* actor = CE_NEW(m_actors_pool, Actor)(res, config, index, physics_system::s_physics, physics_system::s_cooking, m_scene, sg, node, Vector3::ZERO, Quaternion::IDENTITY);
	return m_actors.create(actor);
}

//-----------------------------------------------------------------------------
void PhysicsWorld::destroy_actor(ActorId id)
{
	CE_ASSERT(m_actors.has(id), "Actor does not exist");

	CE_DELETE(m_actors_pool, m_actors.lookup(id));
	m_actors.destroy(id);
}

//-----------------------------------------------------------------------------
ControllerId PhysicsWorld::create_controller(const PhysicsResource* pr, SceneGraph& sg, int32_t node)
{
	Controller* controller = CE_NEW(m_controllers_pool, Controller)(pr, sg, node, physics_system::s_physics, m_controller_manager);
	return m_controllers.create(controller);
}

//-----------------------------------------------------------------------------
void PhysicsWorld::destroy_controller(ControllerId id)
{
	CE_ASSERT(m_controllers.has(id), "Controller does not exist");

	CE_DELETE(m_controllers_pool, m_controllers.lookup(id));
	m_controllers.destroy(id);
}

//-----------------------------------------------------------------------------
TriggerId PhysicsWorld::create_trigger(const Vector3& half_extents, const Vector3& pos, const Quaternion& rot)
{
	Trigger* trigger = CE_NEW(m_triggers_pool, Trigger)(physics_system::s_physics, m_scene, half_extents, pos, rot);
	return m_triggers.create(trigger);
}

//-----------------------------------------------------------------------------
void PhysicsWorld::destroy_trigger(TriggerId id)
{
	CE_ASSERT(m_triggers.has(id), "Trigger does not exist");

	CE_DELETE(m_triggers_pool, m_triggers.lookup(id));
	m_triggers.destroy(id);
}

//-----------------------------------------------------------------------------
JointId	PhysicsWorld::create_joint(const PhysicsResource* pr, const uint32_t index, const Actor& actor_0, const Actor& actor_1)
{
	Joint* joint = CE_NEW(m_joints_pool, Joint)(physics_system::s_physics, pr, index, actor_0, actor_1);
	return m_joints.create(joint);
}

//-----------------------------------------------------------------------------
void PhysicsWorld::destroy_joint(JointId id)
{
	CE_ASSERT(m_joints.has(id), "Joint does not exist");

	CE_DELETE(m_joints_pool, m_joints.lookup(id));
	m_joints.destroy(id);
}

//-----------------------------------------------------------------------------
Actor* PhysicsWorld::lookup_actor(StringId32 name)
{
	for (uint32_t i = 0; i < m_actors.size(); i++)
	{
		Actor* actor = m_actors[i];

		if (actor->name() == name)
		{
			return actor;
		}
	}
}

//-----------------------------------------------------------------------------
Actor* PhysicsWorld::lookup_actor(ActorId id)
{
	CE_ASSERT(m_actors.has(id), "Actor does not exist");
	return m_actors.lookup(id);
}

//-----------------------------------------------------------------------------
Controller* PhysicsWorld::lookup_controller(ControllerId id)
{
	CE_ASSERT(m_controllers.has(id), "Controller does not exist");
	return m_controllers.lookup(id);
}

//-----------------------------------------------------------------------------
Trigger* PhysicsWorld::lookup_trigger(TriggerId id)
{
	CE_ASSERT(m_triggers.has(id), "Trigger does not exist");
	return m_triggers.lookup(id);
}
//-----------------------------------------------------------------------------
Vector3 PhysicsWorld::gravity() const
{
	PxVec3 g = m_scene->getGravity();
	return Vector3(g.x, g.y, g.z);
}

//-----------------------------------------------------------------------------
void PhysicsWorld::set_gravity(const Vector3& g)
{
	m_scene->setGravity(PxVec3(g.x, g.y, g.z));
}

//-----------------------------------------------------------------------------
void PhysicsWorld::set_kinematic(ActorId id)
{
	Actor* actor = lookup_actor(id);
	actor->set_kinematic();
}

//-----------------------------------------------------------------------------
void PhysicsWorld::clear_kinematic(ActorId id)
{
	Actor* actor = lookup_actor(id);
	actor->clear_kinematic();
}

//-----------------------------------------------------------------------------
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
	for (uint32_t i = 0; i < m_controllers.size(); i++)
	{
		m_controllers[i]->update();
	}
}

} // namespace crown