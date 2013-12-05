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

#include "PxPhysicsAPI.h"

namespace crown
{

static physx::PxSimulationFilterShader g_default_filter_shader = physx::PxDefaultSimulationFilterShader;

//-----------------------------------------------------------------------------
PhysicsWorld::PhysicsWorld()
	: m_scene(NULL)
	, m_actor_pool(default_allocator(), MAX_ACTORS, sizeof(Actor), CE_ALIGNOF(Actor))
{
	physx::PxSceneDesc scene_desc(device()->physx()->getTolerancesScale());
	scene_desc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);

	if(!scene_desc.cpuDispatcher)
	{
		m_cpu_dispatcher = physx::PxDefaultCpuDispatcherCreate(1);
		if(!m_cpu_dispatcher)
			CE_FATAL("Asd");
		scene_desc.cpuDispatcher = m_cpu_dispatcher;
	}
	if(!scene_desc.filterShader)
		scene_desc.filterShader = g_default_filter_shader;
	
	m_scene = device()->physx()->createScene(scene_desc);

/*	m_scene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE,				 1.0);
	m_scene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_SHAPES,	1.0f);*/
}

//-----------------------------------------------------------------------------
PhysicsWorld::~PhysicsWorld()
{
	m_scene->release();
}

//-----------------------------------------------------------------------------
ActorId	PhysicsWorld::create_actor(int32_t sg_node, ActorType::Enum type)
{
	physx::PxReal d = 0.0f;
	physx::PxTransform pose = physx::PxTransform(physx::PxVec3(0.0f, -5, 0.0f),physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0.0f, 0.0f, 1.0f)));

/*	physx::PxMaterial* mat = device()->physx()->createMaterial(0.5f, 0.5f, 1.0f);
	physx::PxRigidStatic* plane = device()->physx()->createRigidStatic(pose);
	physx::PxShape* shape = plane->createShape(physx::PxPlaneGeometry(), *mat);
	m_scene->addActor(*plane);*/

	PhysicsGraph* pg = m_graph_manager.create_physics_graph();

	Actor* actor = CE_NEW(m_actor_pool, Actor)(*pg, sg_node, type, Vector3::ZERO, Quaternion::IDENTITY);
	m_scene->addActor(*actor->m_actor);

	return m_actor.create(actor);
}

//-----------------------------------------------------------------------------
void PhysicsWorld::destroy_actor(ActorId id)
{
	CE_ASSERT(m_actor.has(id), "Actor does not exist");

	Actor* actor = m_actor.lookup(id);
	CE_DELETE(m_actor_pool, actor);

	m_scene->removeActor(*actor->m_actor);

	m_actor.destroy(id);
}

//-----------------------------------------------------------------------------
Actor* PhysicsWorld::lookup_actor(ActorId id)
{
	CE_ASSERT(m_actor.has(id), "Actor does not exist");

	return m_actor.lookup(id);
}


//-----------------------------------------------------------------------------
void PhysicsWorld::update(float dt)
{
	Log::d("simulating..");
	m_scene->simulate(0.015f);

	while (!m_scene->fetchResults());

	m_graph_manager.update();
}

} // namespace crown