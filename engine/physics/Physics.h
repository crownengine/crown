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

#include "PxFoundation.h"
#include "PxPhysics.h"
#include "PxCooking.h"
#include "PxDefaultAllocator.h"
#include "PxDefaultErrorCallback.h"
#include "PxExtensionsAPI.h"

namespace crown
{

static physx::PxDefaultErrorCallback 	g_physx_error_callback;
static physx::PxDefaultAllocator 		g_physx_allocator_callback;

struct Physics
{
	Physics();
	~Physics();

public:

	physx::PxFoundation* m_foundation;
	physx::PxPhysics* m_physics;
	physx::PxCooking* m_cooking;
};

//-----------------------------------------------------------------------------
inline Physics::Physics()
	: m_foundation(NULL)
	, m_physics(NULL)
	, m_cooking(NULL)
{
	m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, g_physx_allocator_callback, g_physx_error_callback);
	CE_ASSERT(m_foundation, "Unable to create PhysX Foundation");

	m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation, physx::PxTolerancesScale());
	CE_ASSERT(m_physics, "Unable to create PhysX Physics");

	bool extension = PxInitExtensions(*m_physics);
	CE_ASSERT(extension, "Unable to initialize PhysX Extensions");

	m_cooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_foundation, physx::PxCookingParams());
	CE_ASSERT(m_cooking, "Unable to create PhysX Cooking");
}

//-----------------------------------------------------------------------------
inline Physics::~Physics()
{
	m_cooking->release();
	PxCloseExtensions();
	m_physics->release();
	m_foundation->release();
}

} // namespace crown