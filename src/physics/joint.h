/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "resource_types.h"

#include "PxPhysics.h"
#include "PxJoint.h"
#include "PxMaterial.h"

using physx::PxPhysics;
using physx::PxJoint;
using physx::PxMaterial;

namespace crown
{

struct Actor;

///
/// @ingroup Physics
struct Joint
{
	Joint(PxPhysics* physics, const JointResource* jr, const Actor& actor_0, const Actor& actor_1);

public:

	const JointResource* 	m_resource;

	PxJoint* 			m_joint;
	PxMaterial* 		m_mat;
};

} // namespace crown
