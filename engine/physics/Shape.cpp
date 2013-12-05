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

#include "Shape.h"

#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix4x4.h"

#include "PxShapeExt.h"
#include "PxTransform.h"
#include "PxMat44.h"

namespace crown
{

//-----------------------------------------------------------------------------
Shape::Shape(physx::PxShape* shape)
	: m_shape(shape)
{
}

//-----------------------------------------------------------------------------
Shape::~Shape()
{
	//m_shape->release();
}

//-----------------------------------------------------------------------------
Vector3	Shape::local_position() const
{
	physx::PxTransform t = m_shape->getLocalPose();

	Vector3 pos(t.p.x, t.p.y, t.p.z);

	return pos;
}

//-----------------------------------------------------------------------------
Quaternion Shape::local_rotation() const
{
	physx::PxTransform t = m_shape->getLocalPose();

	Vector3 pos(t.p.x, t.p.y, t.p.z);

	Quaternion rot(t.q.x, t.q.y, t.q.z, t.q.w);

	return rot;
}

//-----------------------------------------------------------------------------
Matrix4x4 Shape::local_pose() const
{
	physx::PxTransform t = m_shape->getLocalPose();

	Vector3 pos(t.p.x, t.p.y, t.p.z);
	Quaternion rot(t.q.x, t.q.y, t.q.z, t.q.w);

	Matrix4x4 m(rot, pos);
	return m;
}

//-----------------------------------------------------------------------------
Vector3	Shape::world_position() const
{
	physx::PxTransform t = physx::PxShapeExt::getGlobalPose(*m_shape);

	Vector3 pos(t.p.x, t.p.y, t.p.z);

	return pos;
}

//-----------------------------------------------------------------------------
Quaternion Shape::world_rotation() const
{
	physx::PxTransform t = physx::PxShapeExt::getGlobalPose(*m_shape);

	Quaternion rot(t.q.x, t.q.y, t.q.z, t.q.w);

	return rot;
}

//-----------------------------------------------------------------------------
Matrix4x4 Shape::world_pose() const
{
	physx::PxTransform t = physx::PxShapeExt::getGlobalPose(*m_shape);

	Vector3 pos(t.p.x, t.p.y, t.p.z);
	Quaternion rot(t.q.x, t.q.y, t.q.z, t.q.w);

	Matrix4x4 m(rot, pos);
	return m;
}

//-----------------------------------------------------------------------------
void Shape::set_local_pose(const Matrix4x4& pose)
{
	physx::PxMat44 m(const_cast<physx::PxReal*>(pose.to_float_ptr()));

	physx::PxTransform t(m);
	m_shape->setLocalPose(t);
}

} // namespace crown