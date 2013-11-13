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

#include "SceneGraph.h"
#include "Quaternion.h"
#include "Allocator.h"

namespace crown
{

//-----------------------------------------------------------------------------
SceneGraph::SceneGraph()
	: m_world_poses(default_allocator())
	, m_local_poses(default_allocator())
	, m_parents(default_allocator())
{
}

//-----------------------------------------------------------------------------
int32_t SceneGraph::create_node(int32_t parent, const Vector3& pos, const Quaternion& rot)
{
	CE_ASSERT(parent >= -1, "Parent node must be >= -1");
	CE_ASSERT(parent < (int32_t) m_local_poses.size(), "Parent node must be < child node");

	Matrix4x4 pose(rot, pos);

	m_world_poses.push_back(pose);
	m_local_poses.push_back(pose);
	m_parents.push_back(parent);

	return m_local_poses.size() - 1;
}

//-----------------------------------------------------------------------------
void SceneGraph::link(int32_t child, int32_t parent)
{
	CE_ASSERT(child < (int32_t) m_parents.size(), "Child node does not exist");
	CE_ASSERT(parent < (int32_t) m_parents.size(), "Parent node does not exist");
	CE_ASSERT(parent < child, "Parent must be < child");

	m_world_poses[child] = Matrix4x4::IDENTITY;
	m_local_poses[child] = Matrix4x4::IDENTITY;
	m_parents[child] = parent;
}

//-----------------------------------------------------------------------------
void SceneGraph::unlink(int32_t child)
{
	CE_ASSERT(child < (int32_t) m_parents.size(), "Child node does not exist");

	if (m_parents[child] != -1)
	{
		// Copy world pose before unlinking from parent
		m_local_poses[child] = m_world_poses[child];
		m_parents[child] = -1;
	}
}

//-----------------------------------------------------------------------------
void SceneGraph::set_local_position(int32_t node, const Vector3& pos)
{
	CE_ASSERT(node < (int32_t) m_parents.size(), "Node does not exist");

	Matrix4x4& local_pose = m_local_poses[node];
	local_pose.set_translation(pos);
}

//-----------------------------------------------------------------------------
void SceneGraph::set_local_rotation(int32_t node, const Quaternion& rot)
{
	CE_ASSERT(node < (int32_t) m_parents.size(), "Node does not exist");

	Matrix4x4& local_pose = m_local_poses[node];

	Vector3 local_translation = local_pose.translation();
	local_pose = rot.to_matrix4x4();
	local_pose.set_translation(local_translation);
}

//-----------------------------------------------------------------------------
void SceneGraph::set_local_pose(int32_t node, const Matrix4x4& pose)
{
	CE_ASSERT(node < (int32_t) m_parents.size(), "Node does not exist");

	m_local_poses[node] = pose;
}

//-----------------------------------------------------------------------------
Vector3 SceneGraph::local_position(int32_t node) const
{
	CE_ASSERT(node < (int32_t) m_parents.size(), "Node does not exist");

	return m_local_poses[node].translation();
}

//-----------------------------------------------------------------------------
Quaternion SceneGraph::local_rotation(int32_t node) const
{
	CE_ASSERT(node < (int32_t) m_parents.size(), "Node does not exist");

	return m_local_poses[node].to_quaternion();
}

//-----------------------------------------------------------------------------
Matrix4x4 SceneGraph::local_pose(int32_t node) const
{
	CE_ASSERT(node < (int32_t) m_parents.size(), "Node does not exist");

	return m_local_poses[node];
}

//-----------------------------------------------------------------------------
Vector3 SceneGraph::world_position(int32_t node) const
{
	CE_ASSERT(node < (int32_t) m_parents.size(), "Node does not exist");

	return m_world_poses[node].translation();
}

//-----------------------------------------------------------------------------
Quaternion SceneGraph::world_rotation(int32_t node) const
{
	CE_ASSERT(node < (int32_t) m_parents.size(), "Node does not exist");

	return m_world_poses[node].to_quaternion();
}

//-----------------------------------------------------------------------------
Matrix4x4 SceneGraph::world_pose(int32_t node) const
{
	CE_ASSERT(node < (int32_t) m_parents.size(), "Node does not exist");

	return m_world_poses[node];
}

//-----------------------------------------------------------------------------
void SceneGraph::clear()
{
	m_world_poses.clear();
	m_local_poses.clear();
	m_parents.clear();
}

//-----------------------------------------------------------------------------
void SceneGraph::update()
{
	for (uint32_t i = 0; i < m_world_poses.size(); i++)
	{
		if (m_parents[i] == -1)
		{
			m_world_poses[i] = m_local_poses[i];
		}
		else
		{
			m_world_poses[i] = m_local_poses[m_parents[i]] * m_local_poses[i];
		}
	}
}

} // namespace crown
