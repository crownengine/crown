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
	, m_shapes(default_allocator())
	, m_local_poses(default_allocator())
	, m_nodes(default_allocator())
{
}

//-----------------------------------------------------------------------------
int32_t	PhysicsGraph::create(int32_t graph_node, Shape shape)
{
	m_shapes.push_back(shape);
	m_local_poses.push_back(Matrix4x4::IDENTITY);
	m_nodes.push_back(graph_node);
	return m_nodes.size() - 1;
}

//-----------------------------------------------------------------------------
Vector3	PhysicsGraph::local_position(int32_t node) const
{
	m_shapes[node].local_position();

}

//-----------------------------------------------------------------------------
Quaternion PhysicsGraph::local_rotation(int32_t node) const
{
	m_shapes[node].local_rotation();
}

//-----------------------------------------------------------------------------
Matrix4x4 PhysicsGraph::local_pose(int32_t node) const
{
	m_shapes[node].local_pose();
}

//-----------------------------------------------------------------------------
Vector3	PhysicsGraph::world_position(int32_t node) const
{
	m_shapes[node].world_position();
}

//-----------------------------------------------------------------------------
Quaternion PhysicsGraph::world_rotation(int32_t node) const
{
	m_shapes[node].world_rotation();
}

//-----------------------------------------------------------------------------
Matrix4x4 PhysicsGraph::world_pose(int32_t node) const
{
	m_shapes[node].world_pose();
}

//-----------------------------------------------------------------------------
void PhysicsGraph::set_local_position(int32_t node, const Vector3& position)
{
	m_shapes[node].set_local_position(position);
}

//-----------------------------------------------------------------------------
void PhysicsGraph::set_local_rotation(int32_t node, const Quaternion& rotation)
{
	m_shapes[node].set_local_rotation(rotation);
}

//-----------------------------------------------------------------------------
void PhysicsGraph::set_local_pose(int32_t node, const Matrix4x4& pose)
{
	m_shapes[node].set_local_pose(pose);
}

//-----------------------------------------------------------------------------
void PhysicsGraph::update()
{
	for (uint32_t i = 0; i < m_local_poses.size(); i++)
	{
		Matrix4x4 a = m_local_poses[i] = m_shapes[i].world_pose();
		printf("|%.1f|%.1f|%.1f|%.1f|\n", a.m[0], a.m[4], a.m[8], a.m[12]);
		printf("|%.1f|%.1f|%.1f|%.1f|\n", a.m[1], a.m[5], a.m[9], a.m[13]);
		printf("|%.1f|%.1f|%.1f|%.1f|\n", a.m[2], a.m[6], a.m[10], a.m[14]);
		printf("|%.1f|%.1f|%.1f|%.1f|\n", a.m[3], a.m[7], a.m[11], a.m[15]);
	}
}


} // namespace crown