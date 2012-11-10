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

#include "Shape.h"
#include "Vec3.h"
#include "Angles.h"

namespace crown
{

class PhysicNode
{
public:
	//! Constructor
	PhysicNode(const Vec3& position, const Angles& axis, bool visible, int collisionGroupId);

	//! Destructor
	virtual ~PhysicNode();

	//! Gets the velocity
	const Vec3& GetVelocity() const;

	//! Sets the velocity
	void SetVelocity(const Vec3 velocity);

	//! Gets the inverse mass
	real GetInverseMass() const;

	//! Sets the inverse mass
	void SetInverseMass(real inverseMass);

	//! Gets the shape
	Shape* GetShape() const;

	//! Sets the shape
	void SetShape(Shape* shape);

	//! Update that applies the velocity
	virtual void Update(real dt);

	//! Called when a collision occours
	virtual void Collision(PhysicNode* other);

	//! Returns the collision group id
	int GetCollisionGroupId() const;

private:

	Vec3 mPosition;
	Vec3 mVelocity;
	real mInverseMass;
	Shape* mShape;
	int mCollisionGroupId;
};

}
