/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "resource_types.h"

#include "PxPhysics.h"
#include "PxScene.h"
#include "PxJoint.h"
#include "PxMaterial.h"

using physx::PxPhysics;
using physx::PxJoint;
using physx::PxScene;
using physx::PxMaterial;

namespace crown
{

struct Actor;

///
/// @ingroup Physics
struct Joint
{
	Joint(PxPhysics* physics, const PhysicsResource* pr, const uint32_t index, const Actor& actor_0, const Actor& actor_1);
	~Joint();

public:

	const PhysicsResource* 	m_resource;
	const uint32_t 			m_index;

	PxJoint* 			m_joint;
	PxMaterial* 		m_mat;
};

} // namespace crown
