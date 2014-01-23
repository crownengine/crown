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

#include "EventStream.h"
#include "PhysicsTypes.h"
#include "PxActor.h"
#include "PxController.h"
#include "PxSimulationEventCallback.h"

using physx::PxSimulationEventCallback;
using physx::PxContactPairHeader;
using physx::PxContactPair;
using physx::PxConstraintInfo;
using physx::PxTriggerPair;
using physx::PxActor;
using physx::PxU32;

using physx::PxUserControllerHitReport;
using physx::PxControllerShapeHit;
using physx::PxControllersHit;
using physx::PxControllerObstacleHit;
using physx::PxContactPairHeader;
using physx::PxContactPairHeaderFlag;
using physx::PxContactPairPoint;
using physx::PxVec3;

namespace crown
{

class PhysicsWorld;

//-----------------------------------------------------------------------------
class PhysicsSimulationCallback : public PxSimulationEventCallback
{
public:

	//-----------------------------------------------------------------------------
	PhysicsSimulationCallback(EventStream& stream)
		: m_events(stream)
	{
	}

	//-----------------------------------------------------------------------------
	void onConstraintBreak(PxConstraintInfo* /*constraints*/, PxU32 /*count*/)
	{
		// printf("COSTRAINTBREAK\n");
	}

	//-----------------------------------------------------------------------------
	void onContact(const PxContactPairHeader& pair_header, const PxContactPair* pairs, PxU32 num_pairs)
	{
		// printf("CONTACT\n");

		// Do not report contact if either actor0 or actor1 has been deleted
		if (pair_header.flags & PxContactPairHeaderFlag::eDELETED_ACTOR_0 || pair_header.flags & PxContactPairHeaderFlag::eDELETED_ACTOR_1) return;

		// printf("ACTOR0 = %.4X\n", pair_header.actors[0]->userData);
		// printf("ACTOR1 = %.4X\n", pair_header.actors[1]->userData);

		PxVec3 where;

		// printf("Num pairs = %d\n", num_pairs);
		for (PxU32 pp = 0; pp < num_pairs; pp++)
		{
			PxContactPairPoint points[8];
			const PxU32 num_points = pairs[pp].extractContacts(points, 8);
			// printf("Num points = %d\n", num_points);

			for (PxU32 i = 0; i < num_points; i++)
			{
				where = points[i].position;
				// printf("where = %.2f %.2f %.2f\n", where.x, where.y, where.z);
			}
		}

		physics_world::CollisionEvent ev;
		ev.actors[0] = (Actor*) pair_header.actors[0]->userData;
		ev.actors[1] = (Actor*) pair_header.actors[1]->userData;
		ev.where = Vector3(where.x, where.y, where.z);
		event_stream::write(m_events, physics_world::EventType::COLLISION, ev);
	}

	//-----------------------------------------------------------------------------
	void onTrigger(PxTriggerPair* /*pairs*/, PxU32 /*count*/)
	{
		// printf("TRIGGER\n");
	}

	//-----------------------------------------------------------------------------
	void onWake(PxActor** /*actors*/, PxU32 /*count*/)
	{
		// printf("WAKE\n");
	}

	//-----------------------------------------------------------------------------
	void onSleep(PxActor** /*actors*/, PxU32 /*count*/)
	{
		// printf("SLEEP\n");
	}

private:

	EventStream& m_events;
};

//-----------------------------------------------------------------------------
class PhysicsControllerCallback : public PxUserControllerHitReport
{
	//-----------------------------------------------------------------------------
	void onShapeHit(const PxControllerShapeHit& hit)
	{
		// printf("SHAPE HIT\n");
	}

	//-----------------------------------------------------------------------------
	void onControllerHit(const PxControllersHit& hit)
	{
		// printf("CONTROLLER HIT\n");
	}

	//-----------------------------------------------------------------------------
	void onObstacleHit(const PxControllerObstacleHit& hit)
	{
		// printf("OBSTACLE HIT\n");
	}
};

} // namespace crown