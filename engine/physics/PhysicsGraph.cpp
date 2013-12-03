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

#include "PhysicsGraph.h"
#include "Vector3.h"
#include "Quaternion.h"

namespace crown
{

//-----------------------------------------------------------------------------
PhysicsGraph::PhysicsGraph(int32_t index)
	: m_index(index)
	, m_local_poses(default_allocator()) 
	, m_world_poses(default_allocator())
	, m_sg_nodes(default_allocator())
{
}

//-----------------------------------------------------------------------------
int32_t	PhysicsGraph::create_node(int32_t node, const Vector3& pos, const Quaternion& rot)
{
	Matrix4x4 pose(rot, pos);
	return create_node(node, pose);	
}

//-----------------------------------------------------------------------------
int32_t PhysicsGraph::create_node(int32_t node, const Matrix4x4& pose)
{
	CE_ASSERT(node >= -1, "node must be >= -1");

	m_world_poses.push_back(pose);
	m_sg_nodes.push_back(node);

	return m_world_poses.size() - 1;
}

//-----------------------------------------------------------------------------
Vector3	PhysicsGraph::local_position(int32_t node) const
{
	return m_local_poses[node].translation();
}

//-----------------------------------------------------------------------------
Quaternion PhysicsGraph::local_rotation(int32_t node) const
{
	return m_local_poses[node].to_quaternion();

}

//-----------------------------------------------------------------------------
Matrix4x4 PhysicsGraph::local_pose(int32_t node) const
{
	return m_local_poses[node];
}


//-----------------------------------------------------------------------------
void PhysicsGraph::set_local_position(int32_t node, const Vector3& pos)
{
	Matrix4x4& local_pose = m_local_poses[node];
	local_pose.set_translation(pos);
}

//-----------------------------------------------------------------------------
void PhysicsGraph::set_local_rotation(int32_t node, const Quaternion& rot)
{
	Matrix4x4& local_pose = m_local_poses[node];

	Vector3 local_translation = local_pose.translation();
	local_pose = rot.to_matrix4x4();
	local_pose.set_translation(local_translation);
}

//-----------------------------------------------------------------------------
void PhysicsGraph::set_local_pose(int32_t node, const Matrix4x4& pose)
{
	m_local_poses[node] = pose;
}

//-----------------------------------------------------------------------------
Vector3	PhysicsGraph::world_position(int32_t node) const
{
	return m_world_poses[node].translation();
}

//-----------------------------------------------------------------------------
Quaternion PhysicsGraph::world_rotation(int32_t node) const
{
	return m_world_poses[node].to_quaternion();
}

//-----------------------------------------------------------------------------
Matrix4x4 PhysicsGraph::world_pose(int32_t node) const
{
	return m_world_poses[node];
}

//-----------------------------------------------------------------------------
void PhysicsGraph::clear()
{
	m_world_poses.clear();
	m_sg_nodes.clear();
}

//-----------------------------------------------------------------------------
void PhysicsGraph::update()
{
	// TODO
}

} // namespace crown