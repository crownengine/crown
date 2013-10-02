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
#include "Device.h"
#include "DebugRenderer.h"
#include "Quat.h"

namespace crown
{

//-----------------------------------------------------------------------------
SceneGraph::SceneGraph()
	: m_world_poses(m_allocator)
	, m_local_poses(m_allocator)
	, m_parents(m_allocator)
{
}

//-----------------------------------------------------------------------------
NodeId SceneGraph::create_node(const Vec3& pos, const Quat& rot)
{
	const NodeId node = m_nodes.create();

	Mat4 pose(rot, pos);

	if (m_world_poses.size() <= node.index)
	{
		m_world_poses.push_back(pose);
		m_local_poses.push_back(pose);
		m_parents.push_back(-1);
		m_sparse_to_packed[node.index] = m_world_poses.size() - 1;
	}
	else
	{
		m_world_poses[node.index] = pose;
		m_local_poses[node.index] = pose;
		m_parents[node.index] = -1;
		m_sparse_to_packed[node.index] = node.index;
	}

	return node;
}

//-----------------------------------------------------------------------------
void SceneGraph::destroy_node(NodeId id)
{
	(void)id;
}

//-----------------------------------------------------------------------------
void SceneGraph::link(NodeId child, NodeId parent)
{
	CE_ASSERT(m_nodes.has(child), "Child node does not exist");
	CE_ASSERT(m_nodes.has(parent), "Parent node does not exist");

	m_parents[m_sparse_to_packed[child.index]] = m_sparse_to_packed[parent.index];
}

//-----------------------------------------------------------------------------
void SceneGraph::unlink(NodeId child)
{
	CE_ASSERT(m_nodes.has(child), "Node does not exist");

	m_local_poses[m_sparse_to_packed[child.index]] = m_world_poses[m_sparse_to_packed[child.index]];
	m_parents[m_sparse_to_packed[child.index]] = -1;
}

//-----------------------------------------------------------------------------
void SceneGraph::set_local_position(NodeId node, const Vec3& pos)
{
	CE_ASSERT(m_nodes.has(node), "Node does not exist");

	Mat4& local_pose = m_local_poses[m_sparse_to_packed[node.index]];
	local_pose.set_translation(pos);
}

//-----------------------------------------------------------------------------
void SceneGraph::set_local_rotation(NodeId node, const Quat& rot)
{
	CE_ASSERT(m_nodes.has(node), "Node does not exist");

	Mat4& local_pose = m_local_poses[m_sparse_to_packed[node.index]];

	Vec3 local_translation = local_pose.translation();
	local_pose = rot.to_mat4();
	local_pose.set_translation(local_translation);
}

//-----------------------------------------------------------------------------
void SceneGraph::set_local_pose(NodeId node, const Mat4& pose)
{
	CE_ASSERT(m_nodes.has(node), "Node does not exist");

	m_local_poses[m_sparse_to_packed[node.index]] = pose;
}

//-----------------------------------------------------------------------------
Vec3 SceneGraph::local_position(NodeId node) const
{
	CE_ASSERT(m_nodes.has(node), "Node does not exist");

	return m_local_poses[m_sparse_to_packed[node.index]].translation();
}

//-----------------------------------------------------------------------------
Quat SceneGraph::local_rotation(NodeId node) const
{
	CE_ASSERT(m_nodes.has(node), "Node does not exist");

	return Quat(Vec3(1, 0, 0), 0.0f);
}

//-----------------------------------------------------------------------------
Mat4 SceneGraph::local_pose(NodeId node) const
{
	CE_ASSERT(m_nodes.has(node), "Node does not exist");

	return m_local_poses[m_sparse_to_packed[node.index]];
}

//-----------------------------------------------------------------------------
Vec3 SceneGraph::world_position(NodeId node) const
{
	CE_ASSERT(m_nodes.has(node), "Node does not exist");

	return m_world_poses[m_sparse_to_packed[node.index]].translation();
}

//-----------------------------------------------------------------------------
Quat SceneGraph::world_rotation(NodeId node) const
{
	CE_ASSERT(m_nodes.has(node), "Node does not exist");

	return Quat(Vec3(1, 0, 0), 0.0f);
}

//-----------------------------------------------------------------------------
Mat4 SceneGraph::world_pose(NodeId node) const
{
	CE_ASSERT(m_nodes.has(node), "Node does not exist");

	return m_world_poses[m_sparse_to_packed[node.index]];
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

	// Draw debug
	for (uint32_t i = 0; i < m_world_poses.size(); i++)
	{
		device()->debug_renderer()->add_pose(m_world_poses[i], true);
	}
}

} // namespace crown
