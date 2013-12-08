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

#define MAX_ACTORS 1024

namespace crown
{

typedef Id ActorId;

class Vector3;
class Actor;
class SceneGraph;

//-----------------------------------------------------------------------------
class PhysicsWorld
{
public:

				PhysicsWorld();
				~PhysicsWorld();

	ActorId		create_actor(SceneGraph& sg, int32_t node, ActorType::Enum type);
	void		destroy_actor(ActorId id);

	Actor*		lookup_actor(ActorId id);

	Vector3		gravity() const;
	void		set_gravity(const Vector3& g);

	void		update();

public:

	physx::PxScene* m_scene;
	physx::PxDefaultCpuDispatcher* m_cpu_dispatcher;

	PoolAllocator m_actor_pool;
	IdArray<MAX_ACTORS, Actor*> m_actor;
};

} // namespace crown
