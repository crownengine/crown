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

#include "Mesh.h"
#include "MeshResource.h"
#include "Vector3.h"
#include "Matrix4x4.h"
#include "Quaternion.h"

namespace crown
{

//-----------------------------------------------------------------------------
void Mesh::create(const MeshResource* mr, int32_t node, const Vector3& pos, const Quaternion& rot)
{
	m_vbuffer = mr->m_vbuffer;
	m_ibuffer = mr->m_ibuffer;

	set_local_position(pos);
	set_local_rotation(rot);

	m_node = node;
}

//-----------------------------------------------------------------------------
Vector3 Mesh::local_position() const
{
	return m_local_pose.translation();
}

//-----------------------------------------------------------------------------
Quaternion Mesh::local_rotation() const
{
	return m_local_pose.to_quaternion();
}

//-----------------------------------------------------------------------------
Matrix4x4 Mesh::local_pose() const
{
	return m_local_pose;
}

//-----------------------------------------------------------------------------
Vector3 Mesh::world_position() const
{
	return m_world_pose.translation();
}

//-----------------------------------------------------------------------------
Quaternion Mesh::world_rotation() const
{
	return m_world_pose.to_quaternion();
}

//-----------------------------------------------------------------------------
Matrix4x4 Mesh::world_pose() const
{
	return m_world_pose;
}

//-----------------------------------------------------------------------------
void Mesh::set_local_position(const Vector3& pos)
{
	m_local_pose.set_translation(pos);
}

//-----------------------------------------------------------------------------
void Mesh::set_local_rotation(const Quaternion& rot)
{
	Matrix4x4& local_pose = m_local_pose;

	Vector3 local_translation = local_pose.translation();
	local_pose = rot.to_matrix4x4();
	local_pose.set_translation(local_translation);
}

//-----------------------------------------------------------------------------
void Mesh::set_local_pose(const Matrix4x4& pose)
{
	m_local_pose = pose;
}

} // namespace crown
