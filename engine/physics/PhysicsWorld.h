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

#include "PxScene.h"
#include "PxDefaultCpuDispatcher.h"
#include "PxControllerManager.h"

#include "Log.h"

using physx::PxControllerManager;
using physx::PxScene;
using physx::PxDefaultCpuDispatcher;
using physx::PxActor;

#define MAX_ACTORS 1024
#define MAX_CONTROLLERS 1024
#define MAX_TRIGGERS 1024
#define MAX_JOINTS 512

namespace crown
{

/// Global physics-related functions
namespace physics_system
{
	/// Initializes the physics system.
	/// This is the place where to create and initialize per-application objects.
	void init();

	/// It should reverse the actions performed by audio_system::init().
	void shutdown();
} // namespace physics_system

//-----------------------------------------------------------------------------
struct PhysicsResource;
struct PhysicsActor;
struct Controller;
struct Vector3;
struct Actor;
struct Trigger;
struct Joint;
struct Quaternion;
class SceneGraph;
struct Actor;

//-----------------------------------------------------------------------------
class PhysicsWorld
{
public:

								PhysicsWorld();
								~PhysicsWorld();

	ActorId						create_actor(const PhysicsResource* res, const uint32_t index, SceneGraph& sg, int32_t node);
	void						destroy_actor(ActorId id);

	ControllerId				create_controller(const PhysicsResource* pr, SceneGraph& sg, int32_t node);
	void						destroy_controller(ControllerId id);

	TriggerId					create_trigger(const Vector3& half_extents, const Vector3& pos, const Quaternion& rot);
	void						destroy_trigger(TriggerId id);

	JointId						create_joint(const PhysicsResource* pr, const uint32_t index, const Actor& actor_0, const Actor& actor_1);
	void						destroy_joint(JointId id);

	Actor*						lookup_actor(StringId32 name);
	Actor*						lookup_actor(ActorId id);
	Controller*					lookup_controller(ControllerId id);
	Trigger*					lookup_trigger(TriggerId id);

	Vector3						gravity() const;
	void						set_gravity(const Vector3& g);

	void						set_kinematic(ActorId id);
	void						clear_kinematic(ActorId id);

	void						update(float dt);

public:

	PxControllerManager*		m_controller_manager;
	PxScene*					m_scene;
	PxDefaultCpuDispatcher*		m_cpu_dispatcher;

	PoolAllocator				m_actors_pool;
	PoolAllocator				m_controllers_pool;
	PoolAllocator				m_triggers_pool;
	PoolAllocator				m_joints_pool;

	IdArray<MAX_ACTORS, Actor*>	m_actors;
	IdArray<MAX_CONTROLLERS, Controller*> m_controllers;
	IdArray<MAX_TRIGGERS, Trigger*> m_triggers;
	IdArray<MAX_JOINTS, Joint*> m_joints;

	// Events management
	EventStream m_events;
	PhysicsSimulationCallback m_callback;

public:

	friend class PhysicsSimulationCallback;
};

} // namespace crown
