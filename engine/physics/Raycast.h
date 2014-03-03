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

#include "Log.h"
#include "PhysicsCallback.h"
#include "PhysicsTypes.h"
#include "PxQueryFiltering.h"
#include "PxScene.h"
#include "PxVec3.h"

using physx::PxQueryFilterData;
using physx::PxQueryFlag;
using physx::PxHitFlag;
using physx::PxHitFlags;
using physx::PxRaycastBuffer;
using physx::PxScene;
using physx::PxVec3;

#define MAX_RAYCAST_INTERSECTIONS 32

namespace crown
{

struct Raycast
{
	//-------------------------------------------------------------------------
	Raycast(PxScene* scene, RaycastMode::Enum mode, RaycastFilter::Enum filter)
		: m_scene(scene)
		, m_buffer(m_hits, MAX_RAYCAST_INTERSECTIONS)
		, m_mode(mode)
		, m_filter(filter)
	{
		switch (m_filter)
		{
			case RaycastFilter::BOTH: break;
			case RaycastFilter::STATIC: m_fd.flags = PxQueryFlag::eSTATIC; break;
			case RaycastFilter::DYNAMIC: m_fd.flags = PxQueryFlag::eDYNAMIC; break;
		}

		switch (m_mode)
		{
			case RaycastMode::CLOSEST: break;
			case RaycastMode::ANY: m_fd.flags |= PxQueryFlag::eANY_HIT; break;
			case RaycastMode::ALL: break;
		}
	}

	//-------------------------------------------------------------------------
	bool cast(const Vector3& from, const Vector3& dir, const float length)
	{
		// Log::i("from: (%f, %f, %f)", from.x, from.y, from.z);
		// Log::i("dir: (%f, %f, %f)", dir.x, dir.y, dir.z);
		// Log::i("length: %f", length);

		bool status = m_scene->raycast(PxVec3(from.x, from.y, from.z)
									 , PxVec3(dir.x, dir.y, dir.z)
									 , length
									 , m_buffer
									 , PxHitFlags(PxHitFlag::eDEFAULT)
									 , m_fd);

		if (status)	Log::i("Raycast YES");
		return status;
	}

private:

	PxScene* 				m_scene;
	PxRaycastHit 			m_hits[MAX_RAYCAST_INTERSECTIONS];	
	PxRaycastBuffer			m_buffer;
	PxQueryFilterData 		m_fd;

	// PhysicsRaycastCallback	m_callback;

	RaycastMode::Enum		m_mode;
	RaycastFilter::Enum		m_filter;
};

} // namespace crown