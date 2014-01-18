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
#include "Physics.h"
#include "Quaternion.h"
#include "SceneGraph.h"
#include "Controller.h"
#include "Trigger.h"
#include "PhysicsCallback.h"

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

namespace crown
{

PxFilterFlags PhysicsFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0, 
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
		pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
	}

	return PxFilterFlag::eDEFAULT;
}

static physx::PxSimulationFilterShader g_default_filter_shader = PhysicsFilterShader;

//-----------------------------------------------------------------------------
PhysicsWorld::PhysicsWorld()
	: m_scene(NULL)
	, m_actors_pool(default_allocator(), MAX_ACTORS, sizeof(Actor), CE_ALIGNOF(Actor))
	, m_controllers_pool(default_allocator(), MAX_CONTROLLERS, sizeof(Controller), CE_ALIGNOF(Controller))
	, m_triggers_pool(default_allocator(), MAX_TRIGGERS, sizeof(Trigger), CE_ALIGNOF(Trigger))
{
	PxSceneDesc scene_desc(device()->physx()->getTolerancesScale());
	scene_desc.gravity = PxVec3(0.0f, -9.81f, 0.0f);

	if(!scene_desc.cpuDispatcher)
	{
		m_cpu_dispatcher = physx::PxDefaultCpuDispatcherCreate(1);
		CE_ASSERT(m_cpu_dispatcher != NULL, "Failed to create PhysX cpu dispatcher");

		scene_desc.cpuDispatcher = m_cpu_dispatcher;
	}

	// Set filter shader
	scene_desc.filterShader = g_default_filter_shader;
	// Enable active transformation
	scene_desc.flags = PxSceneFlag::eENABLE_ACTIVETRANSFORMS;
	// Enable collision detection
	// scene_desc.flags |= PxSceneFlag::eENABLE_CCD;

	// Set simulation event callback
	m_callback = CE_NEW(default_allocator(), PhysicsCallback)();
	scene_desc.simulationEventCallback = m_callback;
	
	// Create scene
	m_scene = device()->physx()->createScene(scene_desc);

	m_scene->setFlag(PxSceneFlag::eENABLE_KINEMATIC_STATIC_PAIRS, true);

	// Create controller manager
	m_controller_manager = PxCreateControllerManager(device()->physx()->getFoundation());
	CE_ASSERT(m_controller_manager != NULL, "Failed to create PhysX controller manager");

	// FIXME FIXME FIXME
	//create_trigger(Vector3(.5, .5, .5), Vector3(5.0, -3.0, 3), Quaternion::IDENTITY);
}

//-----------------------------------------------------------------------------
PhysicsWorld::~PhysicsWorld()
{
	CE_DELETE(default_allocator(), m_callback);
	m_cpu_dispatcher->release();
	m_controller_manager->release();
	m_scene->release();
}

//-----------------------------------------------------------------------------
ActorId	PhysicsWorld::create_actor(const PhysicsResource* res, const uint32_t index, SceneGraph& sg, int32_t node)
{
	Actor* actor = CE_NEW(m_actors_pool, Actor)(res, index, m_scene, sg, node, Vector3::ZERO, Quaternion::IDENTITY);
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
	Controller* controller = CE_NEW(m_controllers_pool, Controller)(pr, sg, node, m_scene, m_controller_manager);
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
	Trigger* trigger = CE_NEW(m_triggers_pool, Trigger)(m_scene, half_extents, pos, rot);
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
void PhysicsWorld::update(float dt)
{
	// Update world pose of the actors
	for (Actor** aa = m_actors.begin(); aa != m_actors.end(); aa++)
	{
		(*aa)->update_pose();
	}

	// Run with fixed timestep
	m_scene->simulate(1.0 / 60.0);

	while (!m_scene->fetchResults());

	// Update transforms
	PxU32 num_active_transforms;
	PxActiveTransform* active_transforms = m_scene->getActiveTransforms(num_active_transforms);

	// Update each actor with its new transform
	for (PxU32 i = 0; i < num_active_transforms; i++)
	{
		const PxTransform tr = active_transforms[i].actor2World;
		const Vector3 pos(tr.p.x, tr.p.y, tr.p.z);
		const Quaternion rot(tr.q.x, tr.q.y, tr.q.z, tr.q.w);

		Actor* actor = static_cast<Actor*>(active_transforms[i].userData);
		if (actor != NULL)
		{
			actor->update(Matrix4x4(rot, pos));
		}
	}

	// Update controllers
	for (Controller** cc = m_controllers.begin(); cc != m_controllers.end(); cc++)
	{
		(*cc)->update();
	}	
}

} // namespace crown