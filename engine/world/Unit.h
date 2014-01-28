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

#include "Types.h"
#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix4x4.h"
#include "Hash.h"
#include "SceneGraph.h"
#include "StringUtils.h"
#include "PhysicsTypes.h"
#include "WorldTypes.h"
#include "RenderWorldTypes.h"

namespace crown
{

//-----------------------------------------------------------------------------
struct ComponentType
{
	enum Enum
	{
		UNKNOWN,
		CAMERA,
		MESH,
		SPRITE,
		ACTOR
	};
};

typedef	Id ComponentId;

struct Component
{
	uint32_t name;
	ComponentId component;
};


class Material;
class SceneGraphManager;
class World;
struct Actor;
struct Camera;
struct Controller;
struct Mesh;
struct Sprite;
struct UnitResource;

#define MAX_CAMERA_COMPONENTS 16
#define MAX_MESH_COMPONENTS 16
#define MAX_SPRITE_COMPONENTS 16
#define MAX_ACTOR_COMPONENTS 16
#define MAX_MATERIAL_COMPONENTS 16

struct Unit
{
						Unit(World& w, const UnitResource* ur, const Matrix4x4& pose);
						~Unit();

	void				set_id(const UnitId id);
	UnitId				id();

	const UnitResource*	resource() const;

	int32_t				node(const char* name) const;
	bool				has_node(const char* name) const;
	uint32_t			num_nodes() const;

	Vector3				local_position(int32_t node) const;
	Quaternion			local_rotation(int32_t node) const;
	Matrix4x4			local_pose(int32_t node) const;

	Vector3				world_position(int32_t node) const;
	Quaternion			world_rotation(int32_t node) const;
	Matrix4x4			world_pose(int32_t node) const;

	void				set_local_position(int32_t node, const Vector3& pos);
	void				set_local_rotation(int32_t node, const Quaternion& rot);
	void				set_local_pose(int32_t node, const Matrix4x4& pose);

	void				link_node(int32_t child, int32_t parent);
	void				unlink_node(int32_t child);

	void				update();
	void				reload(UnitResource* new_ur);

	void				add_component(StringId32 name, Id component, uint32_t& size, Component* array);
	Id					find_component(const char* name, uint32_t size, Component* array);
	Id					find_component(uint32_t index, uint32_t size, Component* array);
	Id 					find_component_by_index(StringId32 name, uint32_t size, Component* array);

	void				add_camera(StringId32 name, CameraId camera);
	void				add_mesh(StringId32 name, MeshId mesh);
	void				add_sprite(StringId32 name, SpriteId sprite);
	void				add_actor(StringId32 name, ActorId actor);
	void				set_controller(StringId32 name, ControllerId controller);
	void				add_material(StringId32 name, MaterialId material);

	Camera*				camera(const char* name);
	Camera*				camera(uint32_t i);

	Mesh*				mesh(const char* name);
	Mesh*				mesh(uint32_t i);

	Sprite*				sprite(const char* name);
	Sprite*				sprite(uint32_t i);

	Actor*				actor(const char* name);
	Actor*				actor(uint32_t i);
	Actor*				actor_by_index(StringId32 name);

	Material*			material(const char* name);
	Material*			material(uint32_t i);

	Controller*			controller();

private:

	void				create_objects(const Matrix4x4& pose);
	void				destroy_objects();
	void				create_camera_objects();
	void				create_renderable_objects();
	void				create_physics_objects();
	void				set_default_material();

public:

	World&				m_world;
	SceneGraph&			m_scene_graph;
	const UnitResource*	m_resource;
	UnitId				m_id;

	uint32_t			m_num_cameras;
	Component			m_cameras[MAX_CAMERA_COMPONENTS];

	uint32_t			m_num_meshes;
	Component			m_meshes[MAX_MESH_COMPONENTS];

	uint32_t			m_num_sprites;
	Component			m_sprites[MAX_SPRITE_COMPONENTS];

	uint32_t			m_num_actors;
	Component 			m_actors[MAX_ACTOR_COMPONENTS];

	uint32_t			m_num_materials;
	Component			m_materials[MAX_MATERIAL_COMPONENTS];

	Component			m_controller;
};

} // namespace crown
