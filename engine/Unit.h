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

#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix4x4.h"
#include "Hash.h"
#include "IdTable.h"
#include "SceneGraph.h"
#include "StringUtils.h"

namespace crown
{

typedef	Id ComponentId;
typedef Id UnitId;

//-----------------------------------------------------------------------------
struct ComponentType
{
	enum Enum
	{
		UNKNOWN,
		CAMERA,
		MESH,
		SPRITE,
		SOUND
	};
};

//-----------------------------------------------------------------------------
struct Component
{
	Id32				id;
	ComponentType::Enum type;
	ComponentId			component;
};

struct ComponentList
{
	ComponentList()
		: m_components(default_allocator())
	{
	}

	//-----------------------------------------------------------------------------
	bool next_free_component(uint32_t& index)
	{
		uint32_t i;
		for (i = 0; i < m_components.size(); i++)
		{
			// id == 0 means free slot
			if (m_components[i].id == 0)
			{
				index = i;
				return true;
			}
		}

		return false;
	}

	//-----------------------------------------------------------------------------
	void add_component(const char* name, uint32_t type, ComponentId component)
	{
		CE_ASSERT_NOT_NULL(name);

		uint32_t key = hash::murmur2_32(name, string::strlen(name), 0);

		uint32_t free_index;

		if (next_free_component(free_index))
		{
			m_components[free_index].id.key = key;
			m_components[free_index].type = (ComponentType::Enum)type;
			m_components[free_index].component = component;
		}

		Component comp;

		comp.id.key = key;
		comp.type = (ComponentType::Enum)type;
		comp.component = component;

		m_components.push_back(comp);
	}

	//-----------------------------------------------------------------------------
	void remove_component(const char* name)
	{
		uint32_t hashed_name = hash::murmur2_32(name, string::strlen(name), 0);

		for (uint32_t i = 0; i < m_components.size(); i++)
		{
			if (m_components[i].id == hashed_name)
			{
				m_components[i].id = 0;
				return;
			}
		}

		CE_FATAL("Component not found!");
	}

	//-----------------------------------------------------------------------------
	Component* get_component(const char* name)
	{
		uint32_t hashed_name = hash::murmur2_32(name, string::strlen(name), 0);

		for (uint32_t i = 0; i < m_components.size(); i++)
		{
			if (m_components[i].id == hashed_name)
			{
				return &m_components[i];
			}
		}

		CE_FATAL("Component not found!");
	}

public:

	List<Component> m_components;
};

//-----------------------------------------------------------------------------
struct UnitResource
{
};

class Camera;
class Mesh;
class Sprite;
class World;

struct Unit
{
					Unit();
	void			create(World& creator, SceneGraph& graph, ComponentList& components, UnitId id, const Vector3& pos, const Quaternion& rot);
	void			destroy();

	void			load(UnitResource* ur);
	void			unload();
	void			reload(UnitResource* new_ur);

	Vector3			local_position(int32_t node = 0) const;
	Quaternion		local_rotation(int32_t node = 0) const;
	Matrix4x4		local_pose(int32_t node = 0) const;

	Vector3			world_position(int32_t node = 0) const;
	Quaternion		world_rotation(int32_t node = 0) const;
	Matrix4x4		world_pose(int32_t node = 0) const;

	void			set_local_position(const Vector3& pos, int32_t node = 0);
	void			set_local_rotation(const Quaternion& rot, int32_t node = 0);
	void			set_local_pose(const Matrix4x4& pose, int32_t node = 0);

	Camera*			camera(const char* name);
	Mesh*			mesh(const char* name);
	Sprite*			sprite(const char* name);

private:

	bool			next_free_component(uint32_t& index);

public:

	World*				m_creator;
	UnitResource*		m_resource;
	UnitId				m_id;

	int32_t				m_root_node;
	SceneGraph*			m_scene_graph;
	ComponentList*		m_component;
};

} // namespace crown
