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

#pragma once

namespace crown
{

class SceneGraphToPhysicsGraph
{
	SceneGraphToPhysicsGraph(SceneGraph& sg, PhysicsGraph& pg)
		: m_scene_graph(sg)
		, m_physics_graph(pg)
		, m_nodes(default_allocator())
		, m_bodies(default_allocator())
	{
	}

	int32_t add_node(int32_t sg_node, int32_t pg_node)
	{
		m_nodes.push_back(sg_node);
		m_bodies.push_back(pg_node);
		return m_bodies.size() - 1;
	}

	void update()
	{
		for (uint32_t i = 0; i < m_nodes.size(); i++)
		{
			m_physics_graph.m_shapes[m_nodes[i]].set_local_pose(m_scene_graph.m_local_poses[m_nodes[i]]);
		}
	}

private:

	SceneGraph& 	m_scene_graph;
	PhysicsGraph& 	m_physics_graph;

	List<int32_t>	m_nodes;
	List<int32_t>	m_bodies;
};

} // namespace crown