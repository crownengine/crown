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

#include "Device.h"
#include "Matrix4x4.h"
#include "Physics.h"
#include "PxPhysicsAPI.h"
#include "Quaternion.h"
#include "Trigger.h"
#include "Vector3.h"

using physx::PxRigidDynamicFlag;
using physx::PxMat44;
using physx::PxTransform;
using physx::PxActorFlag;
using physx::PxVec3;
using physx::PxReal;
using physx::PxRigidBody;
using physx::PxRigidDynamic;
using physx::PxPlaneGeometry;
using physx::PxSphereGeometry;
using physx::PxBoxGeometry;
using physx::PxRigidBodyExt;
using physx::PxShape;
using physx::PxShapeFlag;

namespace crown
{
	
//-----------------------------------------------------------------------------
Trigger::Trigger(PxScene* scene, const Vector3& half_extents, const Vector3& pos, const Quaternion& rot)
	: m_scene(scene)
{
	Matrix4x4 m(rot, pos);
	m.transpose();
	PxMat44 pose((PxReal*)(m.to_float_ptr()));

	m_actor = device()->physx()->createRigidStatic(PxTransform(pose));
	m_actor->userData = NULL;

	m_mat = device()->physx()->createMaterial(0.5f, 0.5f, 0.5f);
	PxShape* shape = m_actor->createShape(PxBoxGeometry(half_extents.x, half_extents.y, half_extents.z), *m_mat);
	shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
	shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);

	m_scene->addActor(*m_actor);
}

//-----------------------------------------------------------------------------
Trigger::~Trigger()
{
	if (m_actor)
	{
		m_scene->removeActor(*m_actor);
		m_actor->release();
	}
}

} // namespace crown
