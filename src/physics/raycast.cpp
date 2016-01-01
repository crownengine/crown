/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "raycast.h"
#include "actor.h"
#include "array.h"
#include "string_utils.h"
#include "PxRigidActor.h"

using physx::PxRigidActor;

namespace crown
{

Raycast::Raycast(PxScene* scene, CollisionMode::Enum mode, CollisionType::Enum type)
	: m_scene(scene)
	, m_buffer(m_hits, CE_MAX_RAY_INTERSECTIONS)
	, m_mode(mode)
	, m_type(type)
{
	switch (m_type)
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

void Raycast::cast(const Vector3& from, const Vector3& dir, const float length, Array<RaycastHit>& hits)
{
	m_scene->raycast(PxVec3(from.x, from.y, from.z), PxVec3(dir.x, dir.y, dir.z), length, m_buffer, PxHitFlags(PxHitFlag::eDEFAULT), m_fd);

	for (uint32_t i = 0; i < m_buffer.getNbAnyHits(); i++)
	{
		PxRaycastHit rh = m_buffer.getAnyHit(i);

		RaycastHit hit;

		hit.position.x = rh.position.x;
		hit.position.y = rh.position.y;
		hit.position.z = rh.position.z;
		hit.distance = rh.distance;
		hit.normal.x = rh.normal.x;
		hit.normal.y = rh.normal.y;
		hit.normal.z = rh.normal.z;
		hit.actor = (Actor*)(rh.actor->userData);

		array::push_back(hits, hit);
	}
}

CollisionMode::Enum Raycast::mode() const
{
	return m_mode;
}

CollisionType::Enum Raycast::type() const
{
	return m_type;
}

} // namespace crown
