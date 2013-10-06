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
#include "Quat.h"
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
int32_t SceneGraph::create_node(int32_t parent, const Vec3& pos, const Quat& rot)
{
	Mat4 pose(rot, pos);

	m_world_poses.push_back(pose);
	m_local_poses.push_back(pose);
	m_parents.push_back(parent);

	return m_local_poses.size() - 1;
}

//-----------------------------------------------------------------------------
void SceneGraph::destroy_node(int32_t id)
{
	(void)id;
}

//-----------------------------------------------------------------------------
void SceneGraph::link(int32_t child, int32_t parent)
{
	m_parents[child] = parent;
}

//-----------------------------------------------------------------------------
void SceneGraph::unlink(int32_t child)
{
	// Copy world pose before unlinking from parent
	m_local_poses[child] = m_world_poses[m_parents[child]];
	m_parents[child] = -1;
}

//-----------------------------------------------------------------------------
void SceneGraph::set_local_position(int32_t node, const Vec3& pos)
{
	Mat4& local_pose = m_local_poses[node];
	local_pose.set_translation(pos);
}

//-----------------------------------------------------------------------------
void SceneGraph::set_local_rotation(int32_t node, const Quat& rot)
{
	Mat4& local_pose = m_local_poses[node];

	Vec3 local_translation = local_pose.translation();
	local_pose = rot.to_mat4();
	local_pose.set_translation(local_translation);
}

//-----------------------------------------------------------------------------
void SceneGraph::set_local_pose(int32_t node, const Mat4& pose)
{
	m_local_poses[node] = pose;
}

//-----------------------------------------------------------------------------
Vec3 SceneGraph::local_position(int32_t node) const
{
	return m_local_poses[node].translation();
}

//-----------------------------------------------------------------------------
Quat SceneGraph::local_rotation(int32_t /*node*/) const
{
	return Quat(Vec3(1, 0, 0), 0.0f);
}

//-----------------------------------------------------------------------------
Mat4 SceneGraph::local_pose(int32_t node) const
{
	return m_local_poses[node];
}

//-----------------------------------------------------------------------------
Vec3 SceneGraph::world_position(int32_t node) const
{
	return m_world_poses[node].translation();
}

//-----------------------------------------------------------------------------
Quat SceneGraph::world_rotation(int32_t /*node*/) const
{
	return Quat(Vec3(1, 0, 0), 0.0f);
}

//-----------------------------------------------------------------------------
Mat4 SceneGraph::world_pose(int32_t node) const
{
	return m_world_poses[node];
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
