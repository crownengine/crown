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

#include "Unit.h"
#include "IdTable.h"
#include "World.h"
#include "Allocator.h"

namespace crown
{

typedef Id CameraId;

Unit::Unit()
	: m_creator(NULL)
	, m_resource(NULL)
	, m_scene_graph(NULL)
	, m_component(NULL)
{
}

//-----------------------------------------------------------------------------
void Unit::create(World& creator, SceneGraph& graph, ComponentList& components, UnitId id, const Vector3& pos, const Quaternion& rot)
{
	m_root_node = graph.create_node(-1, pos, rot);

	m_scene_graph = &graph;
	m_component = &components;
	m_creator = &creator;
	m_id = id;
}

//-----------------------------------------------------------------------------
void Unit::destroy()
{

}

//-----------------------------------------------------------------------------
void Unit::load(UnitResource* ur)
{
	m_resource = ur;
}

//-----------------------------------------------------------------------------
void Unit::unload()
{
}

//-----------------------------------------------------------------------------
void Unit::reload(UnitResource* new_ur)
{
	(void)new_ur;
}

//-----------------------------------------------------------------------------
Vector3 Unit::local_position(int32_t node) const
{
	return m_scene_graph->local_position(node);
}

//-----------------------------------------------------------------------------
Quaternion Unit::local_rotation(int32_t node) const
{
	return m_scene_graph->local_rotation(node);
}

//-----------------------------------------------------------------------------
Matrix4x4 Unit::local_pose(int32_t node) const
{
	return m_scene_graph->local_pose(node);
}

//-----------------------------------------------------------------------------
Vector3 Unit::world_position(int32_t node) const
{
	return m_scene_graph->world_position(node);
}

//-----------------------------------------------------------------------------
Quaternion Unit::world_rotation(int32_t node) const
{
	return m_scene_graph->world_rotation(node);
}

//-----------------------------------------------------------------------------
Matrix4x4 Unit::world_pose(int32_t node) const
{
	return m_scene_graph->world_pose(node);
}

//-----------------------------------------------------------------------------
void Unit::set_local_position(const Vector3& pos, int32_t node)
{
	m_scene_graph->set_local_position(node, pos);
}

//-----------------------------------------------------------------------------
void Unit::set_local_rotation(const Quaternion& rot, int32_t node)
{
	m_scene_graph->set_local_rotation(node, rot);
}

//-----------------------------------------------------------------------------
void Unit::set_local_pose(const Matrix4x4& pose, int32_t node)
{
	m_scene_graph->set_local_pose(node, pose);
}

//-----------------------------------------------------------------------------
Camera* Unit::camera(const char* name)
{
	Component* c = m_component->get_component(name);

	return m_creator->lookup_camera(c->component);
}

//-----------------------------------------------------------------------------
Mesh* Unit::mesh(const char* name)
{
	Component* c = m_component->get_component(name);

	return m_creator->lookup_mesh(c->component);
}

//-----------------------------------------------------------------------------
Sprite*	Unit::sprite(const char* name)
{
	Component* c = m_component->get_component(name);

	return m_creator->lookup_sprite(c->component);
}

} // namespace crown
