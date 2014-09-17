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

#include "event_stream.h"
#include "physics_types.h"
#include "PxActor.h"
#include "PxRigidActor.h"
#include "PxController.h"
#include "PxSimulationEventCallback.h"
#include "PxQueryReport.h"
#include "PxQueryFiltering.h"


using physx::PxActor;
using physx::PxConstraintInfo;
using physx::PxContactPair;
using physx::PxContactPairHeader;
using physx::PxContactPairHeader;
using physx::PxContactPairHeaderFlag;
using physx::PxContactPairPoint;
using physx::PxControllerObstacleHit;
using physx::PxControllerShapeHit;
using physx::PxControllersHit;
using physx::PxSimulationEventCallback;
using physx::PxTriggerPair;
using physx::PxTriggerPairFlag;
using physx::PxU32;
using physx::PxUserControllerHitReport;
using physx::PxVec3;
using physx::PxRaycastCallback;
using physx::PxAgain;
using physx::PxRaycastHit;
using physx::PxPairFlag;
using physx::PxContactPairFlag;

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
		// Do not report contact if either actor0 or actor1 or both have been deleted
		if (pair_header.flags & PxContactPairHeaderFlag::eDELETED_ACTOR_0 ||
			pair_header.flags & PxContactPairHeaderFlag::eDELETED_ACTOR_1) return;

		for (PxU32 pp = 0; pp < num_pairs; pp++)
		{
			const PxContactPair& cp = pairs[pp];

			// We are only interested in touch found or lost
			if ((cp.events & PxPairFlag::eNOTIFY_TOUCH_FOUND) == (PxPairFlag::Enum)0 &&
				(cp.events & PxPairFlag::eNOTIFY_TOUCH_LOST) == (PxPairFlag::Enum)0) continue;

			// Skip if either shape0 or shape1 or both have been deleted
			if (cp.flags & PxContactPairFlag::eDELETED_SHAPE_0 ||
				cp.flags & PxContactPairFlag::eDELETED_SHAPE_1) continue;

			PxContactPairPoint points[8];
			const PxU32 num_points = cp.extractContacts(points, 8);

			PxVec3 where(0, 0, 0);
			PxVec3 normal(0, 0, 0);
			for (PxU32 i = 0; i < num_points; i++)
			{
				where = points[i].position;
				normal = points[i].normal;
			}

			post_collision_event((Actor*) pair_header.actors[0]->userData,
				(Actor*) pair_header.actors[1]->userData,
				Vector3(where.x, where.y, where.z),
				Vector3(normal.x, normal.y, normal.z),
				(cp.events & PxPairFlag::eNOTIFY_TOUCH_FOUND) ?
					physics_world::CollisionEvent::BEGIN_TOUCH :
					physics_world::CollisionEvent::END_TOUCH);
		}
	}

	//-----------------------------------------------------------------------------
	void onTrigger(PxTriggerPair* pairs, PxU32 count)
	{
		for (PxU32 pp = 0; pp < count; pp++)
		{
			const PxTriggerPair& tp = pairs[pp];

			// Do not report event if either trigger ot other shape or both have been deleted
			if (tp.flags & PxTriggerPairFlag::eDELETED_SHAPE_TRIGGER ||
				tp.flags & PxTriggerPairFlag::eDELETED_SHAPE_OTHER) continue;

			post_trigger_event((Actor*)tp.triggerActor->userData,
				(Actor*)tp.otherActor->userData,
				(tp.status & PxPairFlag::eNOTIFY_TOUCH_FOUND ?
				physics_world::TriggerEvent::BEGIN_TOUCH : physics_world::TriggerEvent::END_TOUCH));
		}
	}

	//-----------------------------------------------------------------------------
	void onWake(PxActor** /*actors*/, PxU32 /*count*/)
	{
	}

	//-----------------------------------------------------------------------------
	void onSleep(PxActor** /*actors*/, PxU32 /*count*/)
	{
	}

private:

	void post_collision_event(Actor* actor0, Actor* actor1, const Vector3& where, const Vector3& normal, physics_world::CollisionEvent::Type type)
	{
		physics_world::CollisionEvent ev;
		ev.type = type;
		ev.actors[0] = actor0;
		ev.actors[1] = actor1;
		ev.where = where;
		ev.normal = normal;
		event_stream::write(m_events, physics_world::EventType::COLLISION, ev);
	}

	void post_trigger_event(Actor* trigger, Actor* other, physics_world::TriggerEvent::Type type)
	{
		physics_world::TriggerEvent ev;
		ev.type = type;
		ev.trigger = trigger;
		ev.other = other;
		event_stream::write(m_events, physics_world::EventType::TRIGGER, ev);
	}

private:

	EventStream& m_events;
};

//-----------------------------------------------------------------------------
class PhysicsControllerCallback : public PxUserControllerHitReport
{
	//-----------------------------------------------------------------------------
	void onShapeHit(const PxControllerShapeHit& /*hit*/)
	{
	}

	//-----------------------------------------------------------------------------
	void onControllerHit(const PxControllersHit& /*hit*/)
	{
	}

	//-----------------------------------------------------------------------------
	void onObstacleHit(const PxControllerObstacleHit& /*hit*/)
	{
	}
};

} // namespace crown
