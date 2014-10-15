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

#include "joint.h"
#include "actor.h"
#include "vector3.h"
#include "physics_resource.h"
#include "scene_graph.h"
#include "math_utils.h"

#include "PxVec3.h"
#include "PxMat44.h"
#include "PxRigidActor.h"
#include "PxRigidDynamic.h"
#include "PxRigidStatic.h"
#include "PxBoxGeometry.h"

#include "PxFixedJoint.h"
#include "PxSphericalJoint.h"
#include "PxRevoluteJoint.h"
#include "PxPrismaticJoint.h"
#include "PxDistanceJoint.h"
#include "PxJointLimit.h"
#include "PxTolerancesScale.h"
#include "actor.h"

using physx::PxPhysics;
using physx::PxScene;

using physx::PxTransform;
using physx::PxVec3;
using physx::PxMat44;
using physx::PxRigidActor;
using physx::PxBoxGeometry;
using physx::PxReal;
using physx::PxPi;
using physx::PxConstraintFlag;

using physx::PxJoint;
using physx::PxFixedJoint;
using physx::PxFixedJointCreate;

using physx::PxSphericalJoint;
using physx::PxSphericalJointCreate;
using physx::PxSphericalJointFlag;
using physx::PxJointLimitCone;

using physx::PxRevoluteJoint;
using physx::PxRevoluteJointCreate;
using physx::PxRevoluteJointFlag;

using physx::PxPrismaticJoint;
using physx::PxPrismaticJointCreate;
using physx::PxPrismaticJointFlag;

using physx::PxDistanceJoint;
using physx::PxDistanceJointCreate;
using physx::PxDistanceJointFlag;
using physx::PxJointLinearLimitPair;
using physx::PxTolerancesScale;
using physx::PxJointAngularLimitPair;

namespace crown
{

Joint::Joint(PxPhysics* physics, const PhysicsResource* pr, const uint32_t index, const Actor& actor_0, const Actor& actor_1)
	: m_resource(pr)
	, m_index(index)
{
	const PhysicsJoint* joint = physics_resource::joint(m_resource, m_index);

	PxVec3 anchor_0(joint->anchor_0.x, joint->anchor_0.y, joint->anchor_0.z);
	PxVec3 anchor_1(joint->anchor_1.x, joint->anchor_1.y, joint->anchor_1.z);

	switch(joint->type)
	{
		case JointType::FIXED:
		{
			m_joint = PxFixedJointCreate(*physics, actor_0.m_actor, PxTransform(anchor_0), actor_1.m_actor, PxTransform(anchor_1));

			break;
		}
		case JointType::SPHERICAL:
		{
			PxJointLimitCone limit_cone(joint->y_limit_angle, joint->z_limit_angle, joint->contact_dist);
			limit_cone.restitution = joint->restitution;
			limit_cone.damping = joint->damping;
			limit_cone.contactDistance = joint->distance;

			m_joint = PxSphericalJointCreate(*physics, actor_0.m_actor, PxTransform(anchor_0), actor_1.m_actor, PxTransform(anchor_1));
			static_cast<PxSphericalJoint*>(m_joint)->setLimitCone(limit_cone);
			static_cast<PxSphericalJoint*>(m_joint)->setSphericalJointFlag(PxSphericalJointFlag::eLIMIT_ENABLED, true);

			break;
		}
		case JointType::REVOLUTE:
		{
			PxJointAngularLimitPair limit_pair(joint->lower_limit, joint->upper_limit, joint->contact_dist);
			limit_pair.damping = joint->damping;

			m_joint = PxRevoluteJointCreate(*physics, actor_0.m_actor, PxTransform(anchor_0), actor_1.m_actor, PxTransform(anchor_1));
			static_cast<PxRevoluteJoint*>(m_joint)->setLimit(limit_pair);
			static_cast<PxRevoluteJoint*>(m_joint)->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, true);

			static_cast<PxRevoluteJoint*>(m_joint)->setDriveVelocity(10.0f);
			static_cast<PxRevoluteJoint*>(m_joint)->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_ENABLED, true);

			break;
		}
		case JointType::PRISMATIC:
		{
			PxJointLinearLimitPair limit_pair(PxTolerancesScale(), joint->lower_limit, joint->upper_limit, joint->contact_dist);
			limit_pair.damping = joint->damping;

			m_joint = PxPrismaticJointCreate(*physics, actor_0.m_actor, PxTransform(anchor_0), actor_1.m_actor, PxTransform(anchor_1));
			static_cast<PxPrismaticJoint*>(m_joint)->setLimit(limit_pair);
			static_cast<PxPrismaticJoint*>(m_joint)->setPrismaticJointFlag(PxPrismaticJointFlag::eLIMIT_ENABLED, true);

			break;
		}
		case JointType::DISTANCE:
		{
			m_joint = PxDistanceJointCreate(*physics, actor_0.m_actor, PxTransform(anchor_0), actor_1.m_actor, PxTransform(anchor_1));
			static_cast<PxDistanceJoint*>(m_joint)->setMaxDistance(joint->max_distance);
			static_cast<PxDistanceJoint*>(m_joint)->setDistanceJointFlag(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);

			break;
		}
		case JointType::D6:
		{
			// Must be implemented

			break;
		}
	}

	if (joint->breakable) m_joint->setBreakForce(joint->break_force, joint->break_torque);
}

Joint::~Joint()
{

}

} // namespace crown
