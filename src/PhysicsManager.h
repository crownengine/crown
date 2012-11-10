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

#pragma once

//#include "Config.h"
//#include "PhysicNode.h"
//#include "Dictionary.h"

//namespace crown
//{

//class Scene;
//class SceneNode;

//struct CollisionGroupPair
//{
//	CollisionGroupPair():
//		collisionGroup1Id(0), collisionGroup2Id(0)
//	{
//	}

//	CollisionGroupPair(int Id1, int Id2):
//		collisionGroup1Id(Id1), collisionGroup2Id(Id2)
//	{
//	}

//	CollisionGroupPair(const CollisionGroupPair& other)
//	{
//		collisionGroup1Id = other.collisionGroup1Id;
//		collisionGroup2Id = other.collisionGroup2Id;
//	}

//	int collisionGroup1Id;
//	int collisionGroup2Id;

//	bool operator==(const CollisionGroupPair& other) const
//	{
//		return (collisionGroup1Id == other.collisionGroup1Id && collisionGroup2Id == other.collisionGroup2Id);
//	}
//};

//class PhysicsManager
//{
//private:
//	typedef Dictionary<int, List<PhysicNode*>* > CollisionGroupDictionary;

//public:
//	//! Constructor
//	PhysicsManager();

//	//! Destructor
//	~PhysicsManager();

//	//! Registers the node for collision. The node shape will be used for collision checking and collisions will be reported
//	void Register(PhysicNode* node, int collisionGroupId);

//	//! Removes the node from the physic system
//	void Unregister(PhysicNode* node);

//	//! Register a group pair to perform collision detection
//	void AddGroupPairCollisionCheck(int groupId1, int groupId2);

//	//! Updates the physic system
//	void Update(real dt);

//private:
//	List<PhysicNode*> mPhysicNodes;
//	CollisionGroupDictionary mCollisionGroups;
//	List<CollisionGroupPair> mGroupIdPairsToCollide;

//	void CollideGroups(real dt, int groupId1, int groupId2);

//	// Disable copying
//	PhysicsManager(const PhysicsManager&);
//	PhysicsManager& operator=(const PhysicsManager&);
//};

//}
