/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "config.h"
#include "math_types.h"
#include "container_types.h"
#include "physics_types.h"
#include "PxQueryFiltering.h"
#include "PxScene.h"
#include "PxVec3.h"

using physx::PxQueryFilterData;
using physx::PxQueryFlag;
using physx::PxHitFlag;
using physx::PxHitFlags;
using physx::PxRaycastHit;
using physx::PxRaycastBuffer;
using physx::PxScene;
using physx::PxVec3;

namespace crown
{

struct Actor;

///
/// @ingroup Physics
struct RaycastHit
{
	Vector3 position;
	float distance;
	Vector3 normal;
	Actor* actor;
};


///
/// @ingroup Physics
struct Raycast
{
	/// Constructor
	Raycast(PxScene* scene, CollisionMode::Enum mode, CollisionType::Enum type);

	/// Performs a raycast against objects in the scene. The ray is casted from position @a from, has direction @a dir and is long @a length
	/// If any actor is hit along the ray, @a EventStream is filled according to @a mode previously specified and callback will be called for processing.
	/// @a CollisionMode::ANY: the callback is called with just true or false depending on whether the ray hit anything or not.
	/// @a CollisionMode::CLOSEST: the first argument will tell if there was a hit or not, as before.
	/// If there was a hit, the callback will also be called with the position of the hit, the distance from the origin, the normal of the surface that
	/// was hit and the actor that was hit.
	/// @a CollisionMode::ALL: as @a CollisionMode::CLOSEST, with more tuples
	void					cast(const Vector3& from, const Vector3& dir, const float length, Array<RaycastHit>& hits);

	CollisionMode::Enum 	mode() const;

	CollisionType::Enum 	type() const;

private:

	PxScene* 				m_scene;
	PxRaycastHit 			m_hits[CE_MAX_RAY_INTERSECTIONS];
	PxRaycastBuffer			m_buffer;
	PxQueryFilterData 		m_fd;
	CollisionMode::Enum		m_mode;
	CollisionType::Enum		m_type;
};

} // namespace crown
