/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2009 Erwin Coumans  http://bulletphysics.org

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/
#include "BulletCollision/CollisionShapes/btCollisionShape.h"
#include "LinearMath/btSerializer.h"

/*
  Make sure this dummy function never changes so that it
  can be used by probes that are checking whether the
  library is actually installed.
*/
extern "C"
{
	void btBulletCollisionProbe();

	void btBulletCollisionProbe() {}
}

void btCollisionShape::getBoundingSphere(btVector3& center, btScalar& radius) const
{
	btTransform tr;
	tr.setIdentity();
	btVector3 aabbMin, aabbMax;

	getAabb(tr, aabbMin, aabbMax);

	radius = (aabbMax - aabbMin).length() * btScalar(0.5);
	center = (aabbMin + aabbMax) * btScalar(0.5);
}

btScalar btCollisionShape::getContactBreakingThreshold(btScalar defaultContactThreshold) const
{
	return getAngularMotionDisc() * defaultContactThreshold;
}

btScalar btCollisionShape::getAngularMotionDisc() const
{
	///@todo cache this value, to improve performance
	btVector3 center;
	btScalar disc;
	getBoundingSphere(center, disc);
	disc += (center).length();
	return disc;
}

void btCollisionShape::calculateTemporalAabb(const btTransform& curTrans, const btVector3& linvel, const btVector3& angvel, btScalar timeStep, btVector3& temporalAabbMin, btVector3& temporalAabbMax) const
{
	//start with static aabb
	getAabb(curTrans, temporalAabbMin, temporalAabbMax);

	btScalar temporalAabbMaxx = temporalAabbMax.m_floats[0];
	btScalar temporalAabbMaxy = temporalAabbMax.m_floats[1];
	btScalar temporalAabbMaxz = temporalAabbMax.m_floats[2];
	btScalar temporalAabbMinx = temporalAabbMin.m_floats[0];
	btScalar temporalAabbMiny = temporalAabbMin.m_floats[1];
	btScalar temporalAabbMinz = temporalAabbMin.m_floats[2];

	// add linear motion
	btVector3 linMotion = linvel * timeStep;
	///@todo: simd would have a vector max/min operation, instead of per-element access
	if (linMotion.m_floats[0] > btScalar(0.))
		temporalAabbMaxx += linMotion.m_floats[0];
	else
		temporalAabbMinx += linMotion.m_floats[0];
	if (linMotion.m_floats[1] > btScalar(0.))
		temporalAabbMaxy += linMotion.m_floats[1];
	else
		temporalAabbMiny += linMotion.m_floats[1];
	if (linMotion.m_floats[2] > btScalar(0.))
		temporalAabbMaxz += linMotion.m_floats[2];
	else
		temporalAabbMinz += linMotion.m_floats[2];

	//add conservative angular motion
	btScalar angularMotion = angvel.length() * getAngularMotionDisc() * timeStep;
	btVector3 angularMotion3d(angularMotion, angularMotion, angularMotion);
	temporalAabbMin = btVector3(temporalAabbMinx, temporalAabbMiny, temporalAabbMinz);
	temporalAabbMax = btVector3(temporalAabbMaxx, temporalAabbMaxy, temporalAabbMaxz);

	temporalAabbMin -= angularMotion3d;
	temporalAabbMax += angularMotion3d;
}

///fills the dataBuffer and returns the struct name (and 0 on failure)
const char* btCollisionShape::serialize(void* dataBuffer, btSerializer* serializer) const
{
	btCollisionShapeData* shapeData = (btCollisionShapeData*)dataBuffer;
	char* name = (char*)serializer->findNameForPointer(this);
	shapeData->m_name = (char*)serializer->getUniquePointer(name);
	if (shapeData->m_name)
	{
		serializer->serializeName(name);
	}
	shapeData->m_shapeType = m_shapeType;

	// Fill padding with zeros to appease msan.
	memset(shapeData->m_padding, 0, sizeof(shapeData->m_padding));

	return "btCollisionShapeData";
}

void btCollisionShape::serializeSingleShape(btSerializer* serializer) const
{
	int len = calculateSerializeBufferSize();
	btChunk* chunk = serializer->allocate(len, 1);
	const char* structType = serialize(chunk->m_oldPtr, serializer);
	serializer->finalizeChunk(chunk, structType, BT_SHAPE_CODE, (void*)this);
}
