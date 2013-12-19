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

#include "ProxyAllocator.h"

#include "PxFoundation.h"
#include "PxPhysics.h"
#include "PxCooking.h"
#include "PxDefaultAllocator.h"
#include "PxDefaultErrorCallback.h"
#include "PxExtensionsAPI.h"

#include "Log.h"

using physx::PxAllocatorCallback;
using physx::PxErrorCallback;
using physx::PxErrorCode;

namespace crown
{

class PhysXAllocator : public PxAllocatorCallback
{
public:

	PhysXAllocator(Allocator& a)
		: m_backing(a)
	{
	}

	void* allocate(size_t size, const char*, const char*, int)
	{
		return m_backing.allocate(size, 16);
	}

	void deallocate(void* p)
	{
		m_backing.deallocate(p);
	}

private:

	Allocator& m_backing;
};

class PhysXError : public PxErrorCallback
{
public:

	void reportError(PxErrorCode::Enum code, const char* message, const char* file, int line)
	{
		switch (code)
		{
			case PxErrorCode::eDEBUG_INFO:
			{
				Log::i("In %s:%d: %s", file, line, message);
				break;
			}
			case PxErrorCode::eDEBUG_WARNING: 
			case PxErrorCode::ePERF_WARNING:
			{
				Log::w("In %s:%d: %s", file, line, message);
				break;
			}
			case PxErrorCode::eINVALID_PARAMETER:
			case PxErrorCode::eINVALID_OPERATION:
			case PxErrorCode::eOUT_OF_MEMORY:
			case PxErrorCode::eINTERNAL_ERROR:
			case PxErrorCode::eABORT:
			{
				Log::e("In %s:%d: %s", file, line, message);
				break;
			}
			default:
			{
				break;
			}
		}
	}
};

struct Physics
{
	Physics();
	~Physics();

public:

	ProxyAllocator m_allocator;
	PhysXAllocator m_px_allocator;
	PhysXError m_error;
	physx::PxFoundation* m_foundation;
	physx::PxPhysics* m_physics;
	physx::PxCooking* m_cooking;
};

//-----------------------------------------------------------------------------
inline Physics::Physics()
	: m_allocator("physics", default_allocator())
	, m_px_allocator(m_allocator)
	, m_foundation(NULL)
	, m_physics(NULL)
	, m_cooking(NULL)
{
	m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_px_allocator, m_error);
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