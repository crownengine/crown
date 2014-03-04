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

#include "PxRigidActor.h"
#include "Raycast.h"
#include "Actor.h"
#include "StringUtils.h"

using physx::PxRigidActor;

namespace crown
{

//-------------------------------------------------------------------------
Raycast::Raycast(PxScene* scene, EventStream& events, const char* callback, RaycastMode::Enum mode, RaycastFilter::Enum filter)
	: m_scene(scene)
	, m_buffer(m_hits, MAX_RAYCAST_INTERSECTIONS)
	, m_events(events)
	, m_callback(callback)
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
void Raycast::cast(const Vector3& from, const Vector3& dir, const float length)
{
	bool hit = m_scene->raycast(PxVec3(from.x, from.y, from.z), PxVec3(dir.x, dir.y, dir.z), length,
								m_buffer, PxHitFlags(PxHitFlag::eDEFAULT), m_fd);

	for (uint32_t i = 0; i < m_buffer.getNbAnyHits(); i++)
	{
		PxRaycastHit rh = m_buffer.getAnyHit(i);

		physics_world::RaycastEvent ev;
		
		ev.hit = hit;
		string::strncpy(ev.callback, m_callback, string::strlen(m_callback)+1);
		ev.mode = m_mode;
		ev.position.x = rh.position.x;
		ev.position.y = rh.position.y;
		ev.position.z = rh.position.z;
		ev.distance = rh.distance;
		ev.normal.x = rh.normal.x;
		ev.normal.y = rh.normal.y;
		ev.normal.z = rh.normal.z;
		ev.actor = (Actor*)(rh.actor->userData);

		Log::i("callback: %s", ev.callback);
		Log::i("position: (%f, %f, %f)", ev.position.x, ev.position.y, ev.position.z);
		Log::i("normal: (%f, %f, %f)", ev.normal.x, ev.normal.y, ev.normal.z);
		Log::i("distance: %f", ev.distance);

		event_stream::write(m_events, physics_world::EventType::RAYCAST, ev);
	}
}

} // namespace crown