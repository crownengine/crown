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

#include "PxScene.h"
#include "PxDefaultCpuDispatcher.h"
#include "PxControllerManager.h"

using physx::PxControllerManager;
using physx::PxScene;
using physx::PxDefaultCpuDispatcher;

#define MAX_ACTORS 1024
#define MAX_CONTROLLERS 1024

namespace crown
{

struct PhysicsResource;
struct Controller;
struct Vector3;
struct Actor;
class SceneGraph;

//-----------------------------------------------------------------------------
class PhysicsWorld
{
public:

								PhysicsWorld();
								~PhysicsWorld();

	ActorId						create_actor(SceneGraph& sg, int32_t node, ActorType::Enum type);
	void						destroy_actor(ActorId id);

	ControllerId				create_controller(const PhysicsResource* pr, SceneGraph& sg, int32_t node);
	void						destroy_controller(ControllerId id);

	Actor*						lookup_actor(ActorId id);
	Controller*					lookup_controller(ControllerId id);

	Vector3						gravity() const;
	void						set_gravity(const Vector3& g);

	void						update(float dt);

public:

	PxControllerManager*		m_controller_manager;
	PxScene*					m_scene;
	PxDefaultCpuDispatcher*		m_cpu_dispatcher;
	
	PoolAllocator				m_actors_pool;
	IdArray<MAX_ACTORS, Actor*>	m_actors;

	PoolAllocator				m_controllers_pool;
	IdArray<MAX_CONTROLLERS, Controller*> m_controllers;
};

} // namespace crown
