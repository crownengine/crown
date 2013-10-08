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


namespace crown
{

typedef	Id ComponentId;

//-----------------------------------------------------------------------------
struct ComponentType
{
	enum Enum
	{
		UNKNOWN,
		RENDER,
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

//-----------------------------------------------------------------------------
struct UnitResource
{
};

class Camera;
class World;

struct Unit
{
					Unit();
	void			create(World& creator, const Vector3& pos, const Quaternion& rot);
	void			destroy();

	void			load(UnitResource* ur);
	void			unload();
	void			reload(UnitResource* new_ur);

	Vector3			local_position(int32_t node = 0) const;
	Quaternion			local_rotation(int32_t node = 0) const;
	Matrix4x4			local_pose(int32_t node = 0) const;

	Vector3			world_position(int32_t node = 0) const;
	Quaternion			world_rotation(int32_t node = 0) const;
	Matrix4x4			world_pose(int32_t node = 0) const;

	void			set_local_position(const Vector3& pos, int32_t node = 0);
	void			set_local_rotation(const Quaternion& rot, int32_t node = 0);
	void			set_local_pose(const Matrix4x4& pose, int32_t node = 0);

	void			add_component(const char* name, uint32_t type, ComponentId component);
	void			remove_component(const char* name);
	Component*		get_component(const char* name);

	Camera*			camera(const char* name);

private:

	bool			next_free_component(uint32_t& index);

public:

	World*			m_creator;
	SceneGraph		m_scene_graph;
	int32_t			m_root_node;

	Camera*			m_camera;

	UnitResource*	m_resource;

	List<Component> m_component_list;
};

} // namespace crown
