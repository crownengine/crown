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

namespace crown
{

static physx::PxSimulationFilterShader g_default_filter_shader = physx::PxDefaultSimulationFilterShader;

//-----------------------------------------------------------------------------
PhysicsWorld::PhysicsWorld()
	: m_scene(NULL)
	, m_actors_pool(default_allocator(), MAX_ACTORS, sizeof(Actor), CE_ALIGNOF(Actor))
	, m_controllers_pool(default_allocator(), MAX_CONTROLLERS, sizeof(Controller), CE_ALIGNOF(Controller))
{
	m_controller_manager = PxCreateControllerManager(device()->physx()->getFoundation());

	PxSceneDesc scene_desc(device()->physx()->getTolerancesScale());
	scene_desc.gravity = PxVec3(0.0f, -9.81f, 0.0f);

	if(!scene_desc.cpuDispatcher)
	{
		m_cpu_dispatcher = physx::PxDefaultCpuDispatcherCreate(1);
		CE_ASSERT(m_cpu_dispatcher != NULL, "Failed to create PhysX cpu dispatcher");

		scene_desc.cpuDispatcher = m_cpu_dispatcher;
	}

	if(!scene_desc.filterShader)
		scene_desc.filterShader = g_default_filter_shader;
	
	m_scene = device()->physx()->createScene(scene_desc);
	m_scene->setFlag(PxSceneFlag::eENABLE_ACTIVETRANSFORMS, true);

/*	m_scene->setVisualizationParameter(PxVisualizationParameter::eSCALE,				 1.0);
	m_scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES,	1.0f);*/

	PxTransform pose = PxTransform(PxVec3(0.0f, -3.75, 0.0f), PxQuat(PxHalfPi, PxVec3(0.0f, 0.0f, 1.0f)));
	PxMaterial* mat = device()->physx()->createMaterial(0.5f, 0.5f, 1.0f);
	PxRigidStatic* plane = device()->physx()->createRigidStatic(pose);
	PxShape* shape = plane->createShape(PxPlaneGeometry(), *mat);
	m_scene->addActor(*plane);
}

//-----------------------------------------------------------------------------
PhysicsWorld::~PhysicsWorld()
{
	m_scene->release();
}

//-----------------------------------------------------------------------------
ActorId	PhysicsWorld::create_actor(SceneGraph& sg, int32_t node, ActorType::Enum type)
{
	Actor* actor = CE_NEW(m_actors_pool, Actor)(sg, node, type, Vector3::ZERO, Quaternion::IDENTITY);
	m_scene->addActor(*actor->m_actor);

	return m_actors.create(actor);
}

//-----------------------------------------------------------------------------
void PhysicsWorld::destroy_actor(ActorId id)
{
	CE_ASSERT(m_actors.has(id), "Actor does not exist");

	Actor* actor = m_actors.lookup(id);
	CE_DELETE(m_actors_pool, actor);

	m_scene->removeActor(*actor->m_actor);

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
void PhysicsWorld::update()
{
	// Apply gravity to controllers
	for (Controller** cc = m_controllers.begin(); cc != m_controllers.end(); cc++)
	{
		(*cc)->move(Vector3(0, -9.81, 0));
	}

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