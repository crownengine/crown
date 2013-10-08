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
#include "StringUtils.h"

namespace crown
{

typedef Id CameraId;

Unit::Unit()
	: m_creator(NULL)
	, m_resource(NULL)
	, m_component_list(default_allocator(), 8) // test value
{
}

//-----------------------------------------------------------------------------
void Unit::create(World& creator, const Vector3& pos, const Quaternion& rot)
{
	m_creator = &creator;
	m_root_node = m_scene_graph.create_node(-1, pos, rot);

	int32_t camera_node = m_scene_graph.create_node(m_root_node, Vector3::ZERO, Quaternion::IDENTITY);
	CameraId camera = m_creator->create_camera(camera_node);
	m_camera = m_creator->lookup_camera(camera);
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
	return m_scene_graph.local_position(node);
}

//-----------------------------------------------------------------------------
Quaternion Unit::local_rotation(int32_t node) const
{
	return m_scene_graph.local_rotation(node);
}

//-----------------------------------------------------------------------------
Matrix4x4 Unit::local_pose(int32_t node) const
{
	return m_scene_graph.local_pose(node);
}

//-----------------------------------------------------------------------------
Vector3 Unit::world_position(int32_t node) const
{
	return m_scene_graph.world_position(node);
}

//-----------------------------------------------------------------------------
Quaternion Unit::world_rotation(int32_t node) const
{
	return m_scene_graph.world_rotation(node);
}

//-----------------------------------------------------------------------------
Matrix4x4 Unit::world_pose(int32_t node) const
{
	return m_scene_graph.world_pose(node);
}

//-----------------------------------------------------------------------------
void Unit::set_local_position(const Vector3& pos, int32_t node)
{
	m_scene_graph.set_local_position(node, pos);
}

//-----------------------------------------------------------------------------
void Unit::set_local_rotation(const Quaternion& rot, int32_t node)
{
	m_scene_graph.set_local_rotation(node, rot);
}

//-----------------------------------------------------------------------------
void Unit::set_local_pose(const Matrix4x4& pose, int32_t node)
{
	m_scene_graph.set_local_pose(node, pose);
}

//-----------------------------------------------------------------------------
Camera* Unit::camera(const char* /*name*/)
{
	return m_camera;
}

//-----------------------------------------------------------------------------
bool Unit::next_free_component(uint32_t& index)
{
	uint32_t i;
	for (i = 0; i < m_component_list.size(); i++)
	{
		// id == 0 means free slot
		if (m_component_list[i].id == 0)
		{
			index = i;
			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
void Unit::add_component(const char* name, uint32_t type, ComponentId component)
{
	CE_ASSERT_NOT_NULL(name);

	uint32_t key = hash::murmur2_32(name, string::strlen(name), 0);

	uint32_t free_index;

	if (next_free_component(free_index))
	{
		m_component_list[free_index].id.key = key;
		m_component_list[free_index].type = (ComponentType::Enum)type;
		m_component_list[free_index].component = component;
	}

	Component comp;

	comp.id.key = key;
	comp.type = (ComponentType::Enum)type;
	comp.component = component;

	m_component_list.push_back(comp);
}

//-----------------------------------------------------------------------------
void Unit::remove_component(const char* name)
{
	uint32_t hashed_name = hash::murmur2_32(name, string::strlen(name), 0);

	for (uint32_t i = 0; i < m_component_list.size(); i++)
	{
		if (m_component_list[i].id == hashed_name)
		{
			m_component_list[i].id = 0;
			return;
		}
	}

	CE_FATAL("Component not found!");
}

//-----------------------------------------------------------------------------
Component* Unit::get_component(const char* name)
{
	uint32_t hashed_name = hash::murmur2_32(name, string::strlen(name), 0);

	for (uint32_t i = 0; i < m_component_list.size(); i++)
	{
		if (m_component_list[i].id == hashed_name)
		{
			return &m_component_list[i];
		}
	}

	CE_FATAL("Component not found!");
}


} // namespace crown
