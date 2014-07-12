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
#include "Unit.h"
#include "SceneGraph.h"

namespace crown
{

//-----------------------------------------------------------------------------
Mesh::Mesh(SceneGraph& sg, int32_t node, const MeshResource* mr)
	: m_scene_graph(sg)
	, m_node(node)
	, m_resource(mr)
{
}

//-----------------------------------------------------------------------------
Vector3 Mesh::local_position() const
{
	return m_scene_graph.local_position(m_node);
}

//-----------------------------------------------------------------------------
Quaternion Mesh::local_rotation() const
{
	return m_scene_graph.local_rotation(m_node);
}

//-----------------------------------------------------------------------------
Matrix4x4 Mesh::local_pose() const
{
	return m_scene_graph.local_pose(m_node);
}

//-----------------------------------------------------------------------------
Vector3 Mesh::world_position() const
{
	return m_scene_graph.world_position(m_node);
}

//-----------------------------------------------------------------------------
Quaternion Mesh::world_rotation() const
{
	return m_scene_graph.world_rotation(m_node);
}

//-----------------------------------------------------------------------------
Matrix4x4 Mesh::world_pose() const
{
	return m_scene_graph.world_pose(m_node);
}

//-----------------------------------------------------------------------------
void Mesh::set_local_position(Unit* unit, const Vector3& pos)
{
	unit->set_local_position(m_node, pos);
}

//-----------------------------------------------------------------------------
void Mesh::set_local_rotation(Unit* unit, const Quaternion& rot)
{
	unit->set_local_rotation(m_node, rot);
}

//-----------------------------------------------------------------------------
void Mesh::set_local_pose(Unit* unit, const Matrix4x4& pose)
{
	unit->set_local_pose(m_node, pose);
}

} // namespace crown
