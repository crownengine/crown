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

#include "PhysicNode.h"
#include "PhysicsManager.h"

namespace crown
{

PhysicNode::PhysicNode(const Vec3& position, const Angles& axis, bool visible, int collisionGroupId) :
	mVelocity(0.0, 0.0, 0.0),
	mInverseMass(1.0),
	mShape(NULL),
	mCollisionGroupId(collisionGroupId)
{
	//creator->GetPhysicsManager()->Register(this, mCollisionGroupId);
}

PhysicNode::~PhysicNode()
{
	//GetCreator()->GetPhysicsManager()->Unregister(this);
	if (mShape)
	{
		delete mShape;
	}
}

const Vec3& PhysicNode::GetVelocity() const
{
	return mVelocity;
}

void PhysicNode::SetVelocity(const Vec3 velocity)
{
	mVelocity = velocity;
}

real PhysicNode::GetInverseMass() const
{
	return mInverseMass;
}

void PhysicNode::SetInverseMass(real inverseMass)
{
	mInverseMass = inverseMass;
}

Shape* PhysicNode::GetShape() const
{
	return mShape;
}

void PhysicNode::SetShape(Shape* shape)
{
	if (mShape)
	{
		delete mShape;
	}
	mShape = shape;
	if (mShape)
	{
		mShape->SetPosition(mPosition.ToVec2());
	}
}

void PhysicNode::Update(real dt)
{
	//Integrates the velocity
	mPosition += mVelocity * dt;
	if (mShape)
	{
		mShape->SetPosition(mPosition.ToVec2());
	}
}

void PhysicNode::Collision(PhysicNode* other)
{

}

int PhysicNode::GetCollisionGroupId() const
{
	return mCollisionGroupId;
}

}
