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

#include "PxSimulationEventCallback.h"
#include "PxController.h"

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

namespace crown
{

//-----------------------------------------------------------------------------
class PhysicsSimulationCallback : public PxSimulationEventCallback
{
public:

	//-----------------------------------------------------------------------------
	PhysicsSimulationCallback() {}

	//-----------------------------------------------------------------------------
	void onConstraintBreak(PxConstraintInfo* /*constraints*/, PxU32 /*count*/)
	{
		// Log::i("COSTRAINTBREAK");
	}

	//-----------------------------------------------------------------------------
	void onContact(const PxContactPairHeader& /*pairHeader*/, const PxContactPair* /*pairs*/, PxU32 /*nbPairs*/)
	{
		// Log::i("CONTACT");
	}

	//-----------------------------------------------------------------------------
	void onTrigger(PxTriggerPair* /*pairs*/, PxU32 /*count*/)
	{
		// Log::i("TRIGGER");
	}

	//-----------------------------------------------------------------------------
	void onWake(PxActor** /*actors*/, PxU32 /*count*/)
	{
		// Log::i("WAKE");
	}

	//-----------------------------------------------------------------------------
	void onSleep(PxActor** /*actors*/, PxU32 /*count*/)
	{
		// Log::i("SLEEP");
	}
};

//-----------------------------------------------------------------------------
class PhysicsControllerCallback : public PxUserControllerHitReport
{
	//-----------------------------------------------------------------------------
	void onShapeHit(const PxControllerShapeHit& hit)
	{
		// Log::i("SHAPE HIT");
	}

	//-----------------------------------------------------------------------------
	void onControllerHit(const PxControllersHit& hit)
	{
		// Log::i("CONTROLLER HIT");
	}

	//-----------------------------------------------------------------------------
	void onObstacleHit(const PxControllerObstacleHit& hit)
	{
		// Log::i("OBSTACLE HIT");
	}
};

} // namespace crown