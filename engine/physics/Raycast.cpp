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
Raycast::Raycast(PxScene* scene, EventStream& events, const char* callback, CollisionMode::Enum mode, CollisionType::Enum filter)
	: m_scene(scene)
	, m_buffer(m_hits, CE_MAX_RAY_INTERSECTIONS)
	, m_events(events)
	, m_callback(callback)
	, m_mode(mode)
	, m_filter(filter)
{
	switch (m_filter)
	{
		case CollisionType::BOTH: break;
		case CollisionType::STATIC: m_fd.flags = PxQueryFlag::eSTATIC; break;
		case CollisionType::DYNAMIC: m_fd.flags = PxQueryFlag::eDYNAMIC; break;
	}

	switch (m_mode)
	{
		case CollisionMode::CLOSEST: break;
		case CollisionMode::ANY: m_fd.flags |= PxQueryFlag::eANY_HIT; break;
		case CollisionMode::ALL: break;
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

		physics_world::SceneQueryEvent ev;

		ev.type = SceneQueryType::RAYCAST;
		ev.mode = m_mode;	
		ev.hit = hit;
		ev.callback = m_callback;
		ev.position.x = rh.position.x;
		ev.position.y = rh.position.y;
		ev.position.z = rh.position.z;
		ev.distance = rh.distance;
		ev.normal.x = rh.normal.x;
		ev.normal.y = rh.normal.y;
		ev.normal.z = rh.normal.z;
		ev.actor = (Actor*)(rh.actor->userData);

		event_stream::write(m_events, physics_world::EventType::SCENE_QUERY, ev);

		Log::i("callback: %s", ev.callback);
		Log::i("position: (%f, %f, %f)", ev.position.x, ev.position.y, ev.position.z);
		Log::i("normal: (%f, %f, %f)", ev.normal.x, ev.normal.y, ev.normal.z);
		Log::i("distance: %f", ev.distance);
	}
}

//-------------------------------------------------------------------------
Actor* Raycast::sync_cast(const Vector3& from, const Vector3& dir, const float length)
{
	bool hit = m_scene->raycast(PxVec3(from.x, from.y, from.z), PxVec3(dir.x, dir.y, dir.z), length,
								m_buffer, PxHitFlags(PxHitFlag::eDEFAULT), m_fd);

	if (hit)
	{
		PxRaycastHit rh = m_buffer.getAnyHit(0);

		Log::i("callback: %s", m_callback);
		Log::i("position: (%f, %f, %f)", rh.position.x, rh.position.y, rh.position.z);
		Log::i("distance: %f", rh.distance);

		return (Actor*)(rh.actor->userData);
	}
	else return NULL;
}


} // namespace crown