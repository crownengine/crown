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

#include "types.h"
#include "math_types.h"
#include "string_utils.h"
#include "scene_graph.h"
#include "string_utils.h"
#include "physics_types.h"
#include "world_types.h"
#include "render_world_types.h"
#include "config.h"
#include "sprite_animation.h"

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
typedef Id MaterialId;

struct Component
{
	uint32_t name;
	ComponentId component;
};


class SceneGraphManager;
class World;
struct Actor;
struct Camera;
struct Controller;
struct Mesh;
struct Sprite;
struct Material;
struct UnitResource;

/// Represents a game entity.
///
/// @ingroup World
struct Unit
{
	Unit(World& w, UnitId unit_id, const ResourceId id, const UnitResource* ur, const Matrix4x4& pose);
	~Unit();

	void set_id(const UnitId id);
	UnitId id();

	const UnitResource*	resource() const;

	/// Returns the node @a name.
	int32_t node(const char* name) const;

	/// Returns whether the unit has the node @a name.
	bool has_node(const char* name) const;

	/// Returns the number of nodes of the unit.
	uint32_t num_nodes() const;

	/// Returns the local position of the unit.
	Vector3 local_position(int32_t node) const;

	/// Returns the local rotation of the unit.
	Quaternion local_rotation(int32_t node) const;

	/// Returns the local pose of the unit.
	Matrix4x4 local_pose(int32_t node) const;

	/// Returns the world position of the unit.
	Vector3 world_position(int32_t node) const;

	/// Returns the world rotation of the unit.
	Quaternion world_rotation(int32_t node) const;

	/// Returns the world pose of the unit.
	Matrix4x4 world_pose(int32_t node) const;

	/// Sets the local position of the unit.
	void set_local_position(int32_t node, const Vector3& pos);

	/// Sets the local rotation of the unit.
	void set_local_rotation(int32_t node, const Quaternion& rot);

	/// Sets the local pose of the unit.
	void set_local_pose(int32_t node, const Matrix4x4& pose);

	/// Links the @a child node to the @a parent node.
	/// After the linking the @a child pose is reset to identity.
	/// @note The @a parent node must be either -1 (meaning no parent), or an index lesser than child.
	void link_node(int32_t child, int32_t parent);

	/// Unlinks @a child from its parent, if any.
	void unlink_node(int32_t child);

	void update();
	void reload(UnitResource* new_ur);

	void add_component(StringId32 name, Id component, uint32_t& size, Component* array);
	Id find_component(const char* name, uint32_t size, Component* array);
	Id find_component_by_index(uint32_t index, uint32_t size, Component* array);
	Id find_component_by_name(StringId32 name, uint32_t size, Component* array);

	void add_camera(StringId32 name, CameraId camera);
	void add_mesh(StringId32 name, MeshId mesh);
	void add_sprite(StringId32 name, SpriteId sprite);
	void add_actor(StringId32 name, ActorId actor);
	void set_controller(StringId32 name, ControllerId controller);
	void add_material(StringId32 name, MaterialId material);

	Camera* camera(const char* name);
	Camera* camera(uint32_t i);

	Mesh* mesh(const char* name);
	Mesh* mesh(uint32_t i);

	Sprite* sprite(const char* name);
	Sprite* sprite(uint32_t i);

	Actor* actor(const char* name);
	Actor* actor(uint32_t i);
	Actor* actor_by_index(StringId32 name);

	Material* material(const char* name);
	Material* material(uint32_t i);

	Controller* controller();

	bool is_a(const char* name);

	void play_sprite_animation(const char* name, bool loop);
	void stop_sprite_animation();

	bool has_key(const char* k) const;
	ValueType::Enum value_type(const char* k);
	bool get_key(const char* k, bool& v) const;
	bool get_key(const char* k, float& v) const;
	bool get_key(const char* k, StringId32& v) const;
	bool get_key(const char* k, Vector3& v) const;
	void set_key(const char* k, bool v);
	void set_key(const char* k, float v);
	void set_key(const char* k, const char* v);
	void set_key(const char* k, const Vector3& v);

private:

	void create_objects(const Matrix4x4& pose);
	void destroy_objects();
	void create_camera_objects();
	void create_renderable_objects();
	void create_physics_objects();
	void set_default_material();

public:

	World& m_world;
	SceneGraph& m_scene_graph;
	SpriteAnimation* m_sprite_animation;
	const ResourceId m_resource_id;
	const UnitResource*	m_resource;
	UnitId m_id;

	uint32_t m_num_cameras;
	Component m_cameras[CE_MAX_CAMERA_COMPONENTS];

	uint32_t m_num_meshes;
	Component m_meshes[CE_MAX_MESH_COMPONENTS];

	uint32_t m_num_sprites;
	Component m_sprites[CE_MAX_SPRITE_COMPONENTS];

	uint32_t m_num_actors;
	Component m_actors[CE_MAX_ACTOR_COMPONENTS];

	uint32_t m_num_materials;
	Component m_materials[CE_MAX_MATERIAL_COMPONENTS];

	Component m_controller;

	char* m_values;
};

} // namespace crown
