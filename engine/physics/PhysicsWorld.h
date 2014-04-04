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

#pragma once

#include "IdArray.h"
#include "PoolAllocator.h"
#include "PhysicsTypes.h"
#include "PhysicsCallback.h"
#include "EventStream.h"
#include "PhysicsTypes.h"
#include "MathTypes.h"
#include "WorldTypes.h"
#include "PxScene.h"
#include "PxCooking.h"
#include "PxDefaultCpuDispatcher.h"
#include "PxControllerManager.h"

#include "Log.h"

using physx::PxControllerManager;
using physx::PxScene;
using physx::PxDefaultCpuDispatcher;
using physx::PxActor;
using physx::PxOverlapHit;
using physx::PxOverlapBuffer;
using physx::PxMaterial;
using physx::PxShape;
using physx::PxPhysics;
using physx::PxCooking;

namespace crown
{

/// @defgroup Physics Physics

/// Global physics-related functions
namespace physics_system
{
	/// Initializes the physics system.
	/// This is the place where to create and initialize per-application objects.
	void init();

	/// It should reverse the actions performed by physics_system::init().
	void shutdown();
} // namespace physics_system

//-----------------------------------------------------------------------------
class SceneGraph;
class World;
struct PhysicsResource;
struct PhysicsConfigResource;
struct Unit;

/// Manages physics objects in a World.
///
/// @ingroup Physics
class PhysicsWorld
{
public:

	PhysicsWorld(World& world);
	~PhysicsWorld();

	ActorId create_actor(const PhysicsResource* res, const uint32_t index, SceneGraph& sg, int32_t node, UnitId unit_id);
	void destroy_actor(ActorId id);

	ControllerId create_controller(const PhysicsResource* pr, SceneGraph& sg, int32_t node);
	void destroy_controller(ControllerId id);

	JointId create_joint(const PhysicsResource* pr, const uint32_t index, const Actor& actor_0, const Actor& actor_1);
	void destroy_joint(JointId id);

	RaycastId create_raycast(const char* callback, CollisionMode::Enum mode, CollisionType::Enum filter);
	void destroy_raycast(RaycastId id);


	Vector3 gravity() const;
	void set_gravity(const Vector3& g);

	void set_kinematic(ActorId id);
	void clear_kinematic(ActorId id);

	/// Finds all actors in the physics world that are in a particular shape (supported: spheres, capsules and boxes)
	void overlap_test(const char* callback, CollisionType::Enum filter, ShapeType::Enum type,
											const Vector3& pos, const Quaternion& rot, const Vector3& size, Array<Actor*>& actors);

	void update(float dt);

	Actor* lookup_actor(StringId32 name);
	Actor* lookup_actor(ActorId id);
	Controller* lookup_controller(ControllerId id);
	Raycast* lookup_raycast(RaycastId id);

	World& world() { return m_world; }

public:

	PxPhysics* physx_physics();
	PxCooking* physx_cooking();
	PxScene* physx_scene();
	const PhysicsConfigResource* resource() { return m_resource; }

private:

	World&						m_world;
	PxControllerManager*		m_controller_manager;
	PxScene*					m_scene;
	PxDefaultCpuDispatcher*		m_cpu_dispatcher;

	PxOverlapHit 				m_hits[64]; // hardcoded
	PxOverlapBuffer 			m_buffer;

	PoolAllocator				m_actors_pool;
	PoolAllocator				m_controllers_pool;
	PoolAllocator				m_joints_pool;
	PoolAllocator				m_raycasts_pool;

	IdArray<CE_MAX_ACTORS, Actor*>	m_actors;
	IdArray<CE_MAX_CONTROLLERS, Controller*> m_controllers;
	IdArray<CE_MAX_JOINTS, Joint*> m_joints;
	IdArray<CE_MAX_RAYCASTS, Raycast*> m_raycasts;

	// Events management
	EventStream m_events;
	PhysicsSimulationCallback m_callback;

	const PhysicsConfigResource* m_resource;
};

} // namespace crown
