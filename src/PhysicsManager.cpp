/*
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

#include "PhysicsManager.h"

namespace crown
{

//PhysicsManager::PhysicsManager()
//{

//}

//PhysicsManager::~PhysicsManager()
//{
//	CollisionGroupDictionary::Enumerator e = mCollisionGroups.getBegin();
//	while (e.next())
//	{
//		delete e.current().value;
//	}
//}

//void PhysicsManager::Register(PhysicNode* node, int collisionGroupId)
//{
//	if (mPhysicNodes.Find(node) == -1)
//	{
//		mPhysicNodes.Append(node);
//		List<PhysicNode*>* list;
//		if (mCollisionGroups.Contains(collisionGroupId))
//		{
//			list = mCollisionGroups[collisionGroupId];
//		}
//		else
//		{
//			list = new List<PhysicNode*>();
//			mCollisionGroups[collisionGroupId] = list;
//		}
//		list->Append(node);
//	}
//}

//void PhysicsManager::Unregister(PhysicNode* node)
//{
//	int nodeIndex = mPhysicNodes.Find(node);
//	if (nodeIndex != -1)
//	{
//		mPhysicNodes.Remove(nodeIndex);
//		List<PhysicNode*>* list;
//		list = mCollisionGroups[node->GetCollisionGroupId()];
//		list->Remove(list->Find(node));
//	}
//}

//void PhysicsManager::AddGroupPairCollisionCheck(int groupId1, int groupId2)
//{
//	if (mGroupIdPairsToCollide.Find(CollisionGroupPair(groupId1, groupId2)) == -1)
//	{
//		mGroupIdPairsToCollide.Append(CollisionGroupPair(groupId1, groupId2));
//	}
//}

//void PhysicsManager::Update(real dt)
//{
//	for(int i = 0; i < mGroupIdPairsToCollide.GetSize(); i++)
//	{
//		CollideGroups(dt, mGroupIdPairsToCollide[i].collisionGroup1Id, mGroupIdPairsToCollide[i].collisionGroup2Id);
//	}
//}

//void PhysicsManager::CollideGroups(real dt, int groupId1, int groupId2)
//{
//	if (!mCollisionGroups.Contains(groupId1) || !mCollisionGroups.Contains(groupId2))
//	{
//		return;
//	}

//	List<PhysicNode*>* listGroup1 = mCollisionGroups[groupId1];
//	List<PhysicNode*>* listGroup2 = mCollisionGroups[groupId2];

//	for(int i = 0; i < listGroup1->GetSize(); i++)
//	{
//		PhysicNode* node1 = listGroup1->GetElement(i);
//		Shape* shapeI = node1->GetShape();
//		if (shapeI != NULL)
//		{
//			int j = 0;
//			//Initilization of the j loop variable separated, because it has to be i+1 if the group is the same, to avoid double checks
//			if (groupId1 == groupId2)
//			{
//				j = i + 1;
//			}
//			for(; j < listGroup2->GetSize(); j++)
//			{
//				PhysicNode* node2 = listGroup2->GetElement(j);
//				Shape* shapeJ = node2->GetShape();
//				Vec2 penetration(0.0, 0.0);
//				if (shapeJ != NULL && shapeI->Overlaps(shapeJ, penetration))
//				{
//					real imI = node1->GetInverseMass();
//					real imJ = node2->GetInverseMass();
//					Vec2 imp = penetration / (imI + imJ);
//					Vec3 imp3 = Vec3(imp.x, imp.y, 0.0);
//					node1->SetPosition(node1->GetPosition() + imp3 * imI);
//					node2->SetPosition(node2->GetPosition() - imp3 * imJ);
//					
//					node1->Collision(node2);
//					node2->Collision(node1);
//				}
//			}
//		}
//	}
//}

}
