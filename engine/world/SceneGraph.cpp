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
#include "Vector3.h"
#include "Matrix4x4.h"
#include "Allocator.h"
#include "StringUtils.h"

#include <string.h>

#define CLEAN		0
#define LOCAL_DIRTY	1
#define WORLD_DIRTY	1 << 2

namespace crown
{

using namespace matrix4x4;

//-----------------------------------------------------------------------------
SceneGraph::SceneGraph(Allocator& a, uint32_t index)
	: m_allocator(&a)
	, m_index(index)
	, m_num_nodes(0)
	, m_flags(NULL)
	, m_world_poses(NULL)
	, m_local_poses(NULL)
	, m_parents(NULL)
	, m_names(NULL)
{
}

//-----------------------------------------------------------------------------
void SceneGraph::create(const Matrix4x4& root, uint32_t count, UnitNode* nodes)
{
	m_num_nodes = count;

	m_flags = (uint8_t*) m_allocator->allocate(sizeof(uint8_t) * count);
	m_world_poses = (Matrix4x4*) m_allocator->allocate(sizeof(Matrix4x4) * count);
	m_local_poses = (Matrix4x4*) m_allocator->allocate(sizeof(Matrix4x4) * count);
	m_parents = (int32_t*) m_allocator->allocate(sizeof(int32_t) * count);
	m_names = (StringId32*) m_allocator->allocate(sizeof(StringId32) * count);

	for (uint32_t i = 0; i < count; i++)
	{
		m_flags[i] = (int) CLEAN;
		m_local_poses[i] = nodes[i].pose;
		m_parents[i] = -1;
		m_names[i] = nodes[i].name;
	}

	// Compute initial world poses
	for (uint32_t i = 1; i < m_num_nodes; i++)
	{
		m_world_poses[i] = root * m_local_poses[i];
	}

	m_world_poses[0] = root;
	m_flags[0] = WORLD_DIRTY;

	update();
}

//-----------------------------------------------------------------------------
void SceneGraph::destroy()
{
	m_allocator->deallocate(m_flags);
	m_allocator->deallocate(m_world_poses);
	m_allocator->deallocate(m_local_poses);
	m_allocator->deallocate(m_parents);
	m_allocator->deallocate(m_names);
}

//-----------------------------------------------------------------------------
int32_t SceneGraph::node(const char* name) const
{
	return node(string::murmur2_32(name, string::strlen(name)));
}

//-----------------------------------------------------------------------------
int32_t SceneGraph::node(StringId32 name) const
{
	for (uint32_t i = 0; i < m_num_nodes; i++)
	{
		if (m_names[i] == name)
		{
			return i;
		}
	}

	CE_FATAL("Node not found");
	return 0;
}

//-----------------------------------------------------------------------------
bool SceneGraph::has_node(const char* name) const
{
	StringId32 name_hash = string::murmur2_32(name, string::strlen(name), 0);

	for (uint32_t i = 0; i < m_num_nodes; i++)
	{
		if (m_names[i] == name_hash)
		{
			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
uint32_t SceneGraph::num_nodes() const
{
	return m_num_nodes;
}

//-----------------------------------------------------------------------------
void SceneGraph::link(int32_t child, int32_t parent)
{
	CE_ASSERT(child < (int32_t) m_num_nodes, "Child node does not exist");
	CE_ASSERT(parent < (int32_t) m_num_nodes, "Parent node does not exist");
	CE_ASSERT(parent < child, "Parent must be < child");

	m_world_poses[child] = matrix4x4::IDENTITY;
	m_local_poses[child] = matrix4x4::IDENTITY;
	m_parents[child] = parent;
}

//-----------------------------------------------------------------------------
void SceneGraph::unlink(int32_t child)
{
	CE_ASSERT(child < (int32_t) m_num_nodes, "Child node does not exist");

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
	CE_ASSERT(node < (int32_t) m_num_nodes, "Node does not exist");

	m_flags[node] |= LOCAL_DIRTY;
	set_translation(m_local_poses[node], pos);
}

//-----------------------------------------------------------------------------
void SceneGraph::set_local_rotation(int32_t node, const Quaternion& rot)
{
	CE_ASSERT(node < (int32_t) m_num_nodes, "Node does not exist");

	m_flags[node] |= LOCAL_DIRTY;
	set_rotation(m_local_poses[node], rot);
}

//-----------------------------------------------------------------------------
void SceneGraph::set_local_pose(int32_t node, const Matrix4x4& pose)
{
	CE_ASSERT(node < (int32_t) m_num_nodes, "Node does not exist");

	m_flags[node] |= LOCAL_DIRTY;
	m_local_poses[node] = pose;
}

//-----------------------------------------------------------------------------
Vector3 SceneGraph::local_position(int32_t node) const
{
	CE_ASSERT(node < (int32_t) m_num_nodes, "Node does not exist");

	return translation(m_local_poses[node]);
}

//-----------------------------------------------------------------------------
Quaternion SceneGraph::local_rotation(int32_t node) const
{
	CE_ASSERT(node < (int32_t) m_num_nodes, "Node does not exist");

	return to_quaternion(m_local_poses[node]);
}

//-----------------------------------------------------------------------------
Matrix4x4 SceneGraph::local_pose(int32_t node) const
{
	CE_ASSERT(node < (int32_t) m_num_nodes, "Node does not exist");

	return m_local_poses[node];
}

//-----------------------------------------------------------------------------
void SceneGraph::set_world_position(int32_t node, const Vector3& pos)
{
	CE_ASSERT(node < (int32_t) m_num_nodes, "Node does not exist");

	m_flags[node] |= WORLD_DIRTY;
	set_translation(m_world_poses[node], pos);
}

//-----------------------------------------------------------------------------
void SceneGraph::set_world_rotation(int32_t node, const Quaternion& rot)
{
	CE_ASSERT(node < (int32_t) m_num_nodes, "Node does not exist");

	m_flags[node] |= WORLD_DIRTY;
	set_rotation(m_world_poses[node], rot);
}

//-----------------------------------------------------------------------------
void SceneGraph::set_world_pose(int32_t node, const Matrix4x4& pose)
{
	CE_ASSERT(node < (int32_t) m_num_nodes, "Node does not exist");

	m_flags[node] |= WORLD_DIRTY;
	m_world_poses[node] = pose;
}

//-----------------------------------------------------------------------------
Vector3 SceneGraph::world_position(int32_t node) const
{
	CE_ASSERT(node < (int32_t) m_num_nodes, "Node does not exist");

	return translation(m_world_poses[node]);
}

//-----------------------------------------------------------------------------
Quaternion SceneGraph::world_rotation(int32_t node) const
{
	CE_ASSERT(node < (int32_t) m_num_nodes, "Node does not exist");

	return to_quaternion(m_world_poses[node]);
}

//-----------------------------------------------------------------------------
Matrix4x4 SceneGraph::world_pose(int32_t node) const
{
	CE_ASSERT(node < (int32_t) m_num_nodes, "Node does not exist");

	return m_world_poses[node];
}

//-----------------------------------------------------------------------------
void SceneGraph::update()
{
	for (uint32_t i = 0; i < m_num_nodes; i++)
	{
		const uint8_t my_flags = m_flags[i];
		const uint8_t parent_flags = m_flags[m_parents[i]];

		if (my_flags & LOCAL_DIRTY || parent_flags & WORLD_DIRTY)
		{
			if (m_parents[i] == -1)
			{
				m_world_poses[i] = m_local_poses[i];
			}
			else
			{
				m_world_poses[i] = m_world_poses[m_parents[i]] * m_local_poses[i];
			}

			m_flags[i] = CLEAN;
		}
	}
}

} // namespace crown
