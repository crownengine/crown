/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2006 Erwin Coumans  https://bulletphysics.org

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it freely,
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#include "btSphereBoxCollisionAlgorithm.h"
#include "BulletCollision/CollisionDispatch/btCollisionDispatcher.h"
#include "BulletCollision/CollisionShapes/btSphereShape.h"
#include "BulletCollision/CollisionShapes/btBoxShape.h"
#include "BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "BulletCollision/CollisionDispatch/btCollisionObjectWrapper.h"
//#include <stdio.h>

btSphereBoxCollisionAlgorithm::btSphereBoxCollisionAlgorithm(btPersistentManifold* mf, const btCollisionAlgorithmConstructionInfo& ci, const btCollisionObjectWrapper* col0Wrap, const btCollisionObjectWrapper* col1Wrap, bool isSwapped)
	: btActivatingCollisionAlgorithm(ci, col0Wrap, col1Wrap),
	  m_ownManifold(false),
	  m_manifoldPtr(mf),
	  m_isSwapped(isSwapped)
{
	const btCollisionObjectWrapper* sphereObjWrap = m_isSwapped ? col1Wrap : col0Wrap;
	const btCollisionObjectWrapper* boxObjWrap = m_isSwapped ? col0Wrap : col1Wrap;

	if (!m_manifoldPtr && m_dispatcher->needsCollision(sphereObjWrap->m_collisionObject, boxObjWrap->m_collisionObject))
	{
		m_manifoldPtr = m_dispatcher->getNewManifold(sphereObjWrap->m_collisionObject, boxObjWrap->m_collisionObject);
		m_ownManifold = true;
	}
}

btSphereBoxCollisionAlgorithm::~btSphereBoxCollisionAlgorithm()
{
	if (m_ownManifold)
	{
		if (m_manifoldPtr)
			m_dispatcher->releaseManifold(m_manifoldPtr);
	}
}

void btSphereBoxCollisionAlgorithm::processCollision(const btCollisionObjectWrapper* body0Wrap, const btCollisionObjectWrapper* body1Wrap, const btDispatcherInfo& dispatchInfo, btManifoldResult* resultOut)
{
	(void)dispatchInfo;
	(void)resultOut;
	if (!m_manifoldPtr)
		return;

	const btCollisionObjectWrapper* sphereObjWrap = m_isSwapped ? body1Wrap : body0Wrap;
	const btCollisionObjectWrapper* boxObjWrap = m_isSwapped ? body0Wrap : body1Wrap;

	btVector3 pOnBox;

	btVector3 normalOnSurfaceB;
	btScalar penetrationDepth;
	btVector3 sphereCenter = sphereObjWrap->m_worldTransform.m_origin;
	const btSphereShape* sphere0 = (const btSphereShape*)sphereObjWrap->m_collisionShape;
	btScalar radius = sphere0->getRadius();
	btScalar maxContactDistance = m_manifoldPtr->getContactBreakingThreshold();

	resultOut->setPersistentManifold(m_manifoldPtr);

	if (getSphereDistance(boxObjWrap, pOnBox, normalOnSurfaceB, penetrationDepth, sphereCenter, radius, maxContactDistance))
	{
		/// report a contact. internally this will be kept persistent, and contact reduction is done
		resultOut->addContactPoint(normalOnSurfaceB, pOnBox, penetrationDepth);
	}

	if (m_ownManifold)
	{
		if (m_manifoldPtr->getNumContacts())
		{
			resultOut->refreshContactPoints();
		}
	}
}

btScalar btSphereBoxCollisionAlgorithm::calculateTimeOfImpact(btCollisionObject* col0, btCollisionObject* col1, const btDispatcherInfo& dispatchInfo, btManifoldResult* resultOut)
{
	(void)resultOut;
	(void)dispatchInfo;
	(void)col0;
	(void)col1;

	//not yet
	return btScalar(1.);
}

bool btSphereBoxCollisionAlgorithm::getSphereDistance(const btCollisionObjectWrapper* boxObjWrap, btVector3& pointOnBox, btVector3& normal, btScalar& penetrationDepth, const btVector3& sphereCenter, btScalar fRadius, btScalar maxContactDistance)
{
	const btBoxShape* boxShape = (const btBoxShape*)boxObjWrap->m_collisionShape;
	btVector3 const& boxHalfExtent = boxShape->getHalfExtentsWithoutMargin();
	btScalar boxMargin = boxShape->getMargin();
	penetrationDepth = 1.0f;

	// convert the sphere position to the box's local space
	btTransform const& m44T = boxObjWrap->m_worldTransform;
	btVector3 sphereRelPos = m44T.invXform(sphereCenter);

	// Determine the closest point to the sphere center in the box
	btVector3 closestPoint = sphereRelPos;
	closestPoint.m_floats[0] = (btMin(boxHalfExtent.m_floats[0], closestPoint.m_floats[0]));
	closestPoint.m_floats[0] = (btMax(-boxHalfExtent.m_floats[0], closestPoint.m_floats[0]));
	closestPoint.m_floats[1] = (btMin(boxHalfExtent.m_floats[1], closestPoint.m_floats[1]));
	closestPoint.m_floats[1] = (btMax(-boxHalfExtent.m_floats[1], closestPoint.m_floats[1]));
	closestPoint.m_floats[2] = (btMin(boxHalfExtent.m_floats[2], closestPoint.m_floats[2]));
	closestPoint.m_floats[2] = (btMax(-boxHalfExtent.m_floats[2], closestPoint.m_floats[2]));

	btScalar intersectionDist = fRadius + boxMargin;
	btScalar contactDist = intersectionDist + maxContactDistance;
	normal = sphereRelPos - closestPoint;

	//if there is no penetration, we are done
	btScalar dist2 = normal.length2();
	if (dist2 > contactDist * contactDist)
	{
		return false;
	}

	btScalar distance;

	//special case if the sphere center is inside the box
	if (dist2 <= SIMD_EPSILON)
	{
		distance = -getSpherePenetration(boxHalfExtent, sphereRelPos, closestPoint, normal);
	}
	else  //compute the penetration details
	{
		distance = normal.length();
		normal /= distance;
	}

	pointOnBox = closestPoint + normal * boxMargin;
	//	v3PointOnSphere = sphereRelPos - (normal * fRadius);
	penetrationDepth = distance - intersectionDist;

	// transform back in world space
	btVector3 tmp = m44T(pointOnBox);
	pointOnBox = tmp;
	//	tmp = m44T(v3PointOnSphere);
	//	v3PointOnSphere = tmp;
	tmp = m44T.m_basis * normal;
	normal = tmp;

	return true;
}

btScalar btSphereBoxCollisionAlgorithm::getSpherePenetration(btVector3 const& boxHalfExtent, btVector3 const& sphereRelPos, btVector3& closestPoint, btVector3& normal)
{
	//project the center of the sphere on the closest face of the box
	btScalar faceDist = boxHalfExtent.m_floats[0] - sphereRelPos.m_floats[0];
	btScalar minDist = faceDist;
	closestPoint.m_floats[0] = (boxHalfExtent.m_floats[0]);
	normal.setValue(btScalar(1.0f), btScalar(0.0f), btScalar(0.0f));

	faceDist = boxHalfExtent.m_floats[0] + sphereRelPos.m_floats[0];
	if (faceDist < minDist)
	{
		minDist = faceDist;
		closestPoint = sphereRelPos;
		closestPoint.m_floats[0] = (-boxHalfExtent.m_floats[0]);
		normal.setValue(btScalar(-1.0f), btScalar(0.0f), btScalar(0.0f));
	}

	faceDist = boxHalfExtent.m_floats[1] - sphereRelPos.m_floats[1];
	if (faceDist < minDist)
	{
		minDist = faceDist;
		closestPoint = sphereRelPos;
		closestPoint.m_floats[1] = (boxHalfExtent.m_floats[1]);
		normal.setValue(btScalar(0.0f), btScalar(1.0f), btScalar(0.0f));
	}

	faceDist = boxHalfExtent.m_floats[1] + sphereRelPos.m_floats[1];
	if (faceDist < minDist)
	{
		minDist = faceDist;
		closestPoint = sphereRelPos;
		closestPoint.m_floats[1] = (-boxHalfExtent.m_floats[1]);
		normal.setValue(btScalar(0.0f), btScalar(-1.0f), btScalar(0.0f));
	}

	faceDist = boxHalfExtent.m_floats[2] - sphereRelPos.m_floats[2];
	if (faceDist < minDist)
	{
		minDist = faceDist;
		closestPoint = sphereRelPos;
		closestPoint.m_floats[2] = (boxHalfExtent.m_floats[2]);
		normal.setValue(btScalar(0.0f), btScalar(0.0f), btScalar(1.0f));
	}

	faceDist = boxHalfExtent.m_floats[2] + sphereRelPos.m_floats[2];
	if (faceDist < minDist)
	{
		minDist = faceDist;
		closestPoint = sphereRelPos;
		closestPoint.m_floats[2] = (-boxHalfExtent.m_floats[2]);
		normal.setValue(btScalar(0.0f), btScalar(0.0f), btScalar(-1.0f));
	}

	return minDist;
}
