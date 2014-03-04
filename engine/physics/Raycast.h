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
#include "PhysicsTypes.h"
#include "PxQueryFiltering.h"
#include "PxScene.h"
#include "PxVec3.h"
#include "EventStream.h"

using physx::PxQueryFilterData;
using physx::PxQueryFlag;
using physx::PxHitFlag;
using physx::PxHitFlags;
using physx::PxRaycastHit;
using physx::PxRaycastBuffer;
using physx::PxScene;
using physx::PxVec3;

#define MAX_RAYCAST_INTERSECTIONS 32

namespace crown
{

struct Vector3;

struct Raycast
{
			Raycast(PxScene* scene, EventStream& events, const char* callback, RaycastMode::Enum mode, RaycastFilter::Enum filter);

	void	cast(const Vector3& from, const Vector3& dir, const float length);

private:

	PxScene* 				m_scene;
	PxRaycastHit 			m_hits[MAX_RAYCAST_INTERSECTIONS];	
	PxRaycastBuffer			m_buffer;
	PxQueryFilterData 		m_fd;

	EventStream&			m_events;
	const char*				m_callback;

	RaycastMode::Enum		m_mode;
	RaycastFilter::Enum		m_filter;
};

} // namespace crown