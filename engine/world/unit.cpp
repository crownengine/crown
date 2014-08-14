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

#include "unit.h"
#include "world.h"
#include "allocator.h"
#include "log.h"
#include "unit_resource.h"
#include "scene_graph_manager.h"
#include "actor.h"
#include "controller.h"
#include "physics_resource.h"
#include "device.h"
#include "resource_manager.h"
#include "sprite.h"

namespace crown
{

//-----------------------------------------------------------------------------
Unit::Unit(World& w, UnitId unit_id, const ResourceId id, const UnitResource* ur, const Matrix4x4& pose)
	: m_world(w)
	, m_scene_graph(*w.scene_graph_manager()->create_scene_graph())
	, m_resource_id(id)
	, m_resource(ur)
	, m_id(unit_id)
	, m_num_cameras(0)
	, m_num_meshes(0)
	, m_num_sprites(0)
	, m_num_actors(0)
	, m_values(NULL)
{
	m_controller.component.id = INVALID_ID;
	create_objects(pose);
}

//-----------------------------------------------------------------------------
Unit::~Unit()
{
	destroy_objects();
	m_world.scene_graph_manager()->destroy_scene_graph(&m_scene_graph);
}

//-----------------------------------------------------------------------------
void Unit::set_id(const UnitId id)
{
	m_id = id;
}

//-----------------------------------------------------------------------------
UnitId Unit::id()
{
	return m_id;
}

//-----------------------------------------------------------------------------
const UnitResource*	Unit::resource() const
{
	return m_resource;
}

//-----------------------------------------------------------------------------
void Unit::create_objects(const Matrix4x4& pose)
{
	// Create the scene graph
	m_scene_graph.create(pose, m_resource->num_scene_graph_nodes(), m_resource->scene_graph_nodes());

	create_camera_objects();
	create_renderable_objects();
	create_physics_objects();

	set_default_material();

	m_values = (char*) default_allocator().allocate(m_resource->values_size());
	memcpy(m_values, m_resource->values(), m_resource->values_size());
}

//-----------------------------------------------------------------------------
void Unit::destroy_objects()
{
	default_allocator().deallocate(m_values);

	// Destroy cameras
	for (uint32_t i = 0; i < m_num_cameras; i++)
	{
		m_world.destroy_camera(m_cameras[i].component);
	}
	m_num_cameras = 0;

	// Destroy meshes
	for (uint32_t i = 0; i < m_num_meshes; i++)
	{
		m_world.render_world()->destroy_mesh(m_meshes[i].component);
	}
	m_num_meshes = 0;

	// Destroy sprites
	for (uint32_t i = 0; i < m_num_sprites; i++)
	{
		m_world.render_world()->destroy_sprite(m_sprites[i].component);
	}
	m_num_sprites = 0;

	// Destroy actors
	for (uint32_t i = 0; i < m_num_actors; i++)
	{
		m_world.physics_world()->destroy_actor(m_actors[i].component);
	}
	m_num_actors = 0;

	// Destroy controller
	if (m_controller.component.id != INVALID_ID)
	{
		m_world.physics_world()->destroy_controller(m_controller.component);
		m_controller.component.id = INVALID_ID;
	}

	// Destroy scene graph
	m_scene_graph.destroy();
}

//-----------------------------------------------------------------------------
void Unit::create_camera_objects()
{
	for (uint32_t i = 0; i < m_resource->num_cameras(); i++)
	{
		const UnitCamera cam = m_resource->get_camera(i);
		const CameraId id = m_world.create_camera(m_scene_graph, cam.node, cam.type, cam.near, cam.far);
		add_camera(cam.name, id);
	}
}

//-----------------------------------------------------------------------------
void Unit::create_renderable_objects()
{
	// Create renderables
	for (uint32_t i = 0; i < m_resource->num_renderables(); i++)
	{
		const UnitRenderable renderable = m_resource->get_renderable(i);

		if (renderable.type == UnitRenderable::MESH)
		{
			MeshResource* mr = (MeshResource*) device()->resource_manager()->get(renderable.resource);
			MeshId mesh = m_world.render_world()->create_mesh(mr, m_scene_graph, renderable.node);
			add_mesh(renderable.name, mesh);
		}
		else if (renderable.type == UnitRenderable::SPRITE)
		{
			SpriteResource* sr = (SpriteResource*) device()->resource_manager()->get(renderable.resource);
			SpriteId sprite = m_world.render_world()->create_sprite(sr, m_scene_graph, renderable.node);
			add_sprite(renderable.name, sprite);
		}
		else
		{
			CE_FATAL("Oops, bad renderable type");
		}
	}
}

//-----------------------------------------------------------------------------
void Unit::create_physics_objects()
{
	if (m_resource->physics_resource().type != 0)
	{
		const PhysicsResource* pr = (PhysicsResource*) device()->resource_manager()->get(m_resource->physics_resource());

		// Create controller if any
		if (pr->has_controller())
		{
			set_controller(pr->controller().name, m_world.physics_world()->create_controller(pr, m_scene_graph, 0));
		}

		// Create actors if any
		for (uint32_t i = 0; i < pr->num_actors(); i++)
		{
			const PhysicsActor& actor = pr->actor(i);

			ActorId id = m_world.physics_world()->create_actor(pr, i, m_scene_graph, m_scene_graph.node(actor.node), m_id);
			add_actor(actor.name, id);
		}

		// Create joints if any
		for (uint32_t i = 0; i < pr->num_joints(); i++)
		{
			const PhysicsJoint& joint = pr->joint(i);

			Actor* a1 = actor_by_index(joint.actor_0);
			Actor* a2 = actor_by_index(joint.actor_1);

			m_world.physics_world()->create_joint(pr, i, *a1, *a2);
		}
	}
}

//-----------------------------------------------------------------------------
void Unit::set_default_material()
{
	if (m_resource->num_materials() == 0) return;

	for (uint32_t i = 0; i < m_num_sprites; i++)
	{
		Sprite* s = m_world.render_world()->get_sprite(m_sprites[i].component);
		s->set_material(m_resource->get_material(0).id);
	}
}

//-----------------------------------------------------------------------------
int32_t Unit::node(const char* name) const
{
	return m_scene_graph.node(name);
}

//-----------------------------------------------------------------------------
bool Unit::has_node(const char* name) const
{
	return m_scene_graph.has_node(name);
}

//-----------------------------------------------------------------------------
uint32_t Unit::num_nodes() const
{
	return m_scene_graph.num_nodes();
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
void Unit::set_local_position(int32_t node, const Vector3& pos)
{
	m_scene_graph.set_local_position(node, pos);
}

//-----------------------------------------------------------------------------
void Unit::set_local_rotation(int32_t node, const Quaternion& rot)
{
	m_scene_graph.set_local_rotation(node, rot);
}

//-----------------------------------------------------------------------------
void Unit::set_local_pose(int32_t node, const Matrix4x4& pose)
{
	m_scene_graph.set_local_pose(node, pose);
}

//-----------------------------------------------------------------------------
void Unit::link_node(int32_t child, int32_t parent)
{
	m_scene_graph.link(child, parent);
}

//-----------------------------------------------------------------------------
void Unit::unlink_node(int32_t child)
{
	m_scene_graph.unlink(child);
}

//-----------------------------------------------------------------------------
void Unit::update()
{
}

//-----------------------------------------------------------------------------
void Unit::reload(UnitResource* new_ur)
{
	Matrix4x4 m = m_scene_graph.world_pose(0);
	destroy_objects();
	m_resource = new_ur;
	create_objects(m);
}

//-----------------------------------------------------------------------------
void Unit::add_component(StringId32 name, Id component, uint32_t& size, Component* array)
{
	Component comp;
	comp.name = name;
	comp.component = component;

	array[size] = comp;
	size++;
}

//-----------------------------------------------------------------------------
Id Unit::find_component(const char* name, uint32_t size, Component* array)
{
	const uint32_t name_hash = string::murmur2_32(name, string::strlen(name), 0);
	return find_component_by_name(name_hash, size, array);
}

//-----------------------------------------------------------------------------
Id Unit::find_component_by_index(uint32_t index, uint32_t size, Component* array)
{
	Id comp;
	comp.id = INVALID_ID;

	if (index < size)
	{
		comp = array[index].component;
	}

	return comp;
}

//-----------------------------------------------------------------------------
Id Unit::find_component_by_name(StringId32 name, uint32_t size, Component* array)
{
	Id comp;
	comp.id = INVALID_ID;

	for (uint32_t i = 0; i < size; i++)
	{
		if (name == array[i].name)
		{
			comp = array[i].component;
		}
	}

	return comp;
}

//-----------------------------------------------------------------------------
void Unit::add_camera(StringId32 name, CameraId camera)
{
	CE_ASSERT(m_num_cameras < CE_MAX_CAMERA_COMPONENTS, "Max camera number reached");

	add_component(name, camera, m_num_cameras, m_cameras);
}

//-----------------------------------------------------------------------------
void Unit::add_mesh(StringId32 name, MeshId mesh)
{
	CE_ASSERT(m_num_meshes < CE_MAX_MESH_COMPONENTS, "Max mesh number reached");

	add_component(name, mesh, m_num_meshes, m_meshes);
}

//-----------------------------------------------------------------------------
void Unit::add_sprite(StringId32 name, SpriteId sprite)
{
	CE_ASSERT(m_num_sprites < CE_MAX_SPRITE_COMPONENTS, "Max sprite number reached");

	add_component(name, sprite, m_num_sprites, m_sprites);
}

//-----------------------------------------------------------------------------
void Unit::add_actor(StringId32 name, ActorId actor)
{
	CE_ASSERT(m_num_actors < CE_MAX_ACTOR_COMPONENTS, "Max actor number reached");

	add_component(name, actor, m_num_actors, m_actors);
}

//-----------------------------------------------------------------------------
void Unit::set_controller(StringId32 name, ControllerId controller)
{
	m_controller.name = name;
	m_controller.component = controller;
}

//-----------------------------------------------------------------------------
Camera* Unit::camera(const char* name)
{
	CameraId cam = find_component(name, m_num_cameras, m_cameras);

	CE_ASSERT(cam.id != INVALID_ID, "Unit does not have camera with name '%s'", name);

	return m_world.get_camera(cam);
}

//-----------------------------------------------------------------------------
Camera* Unit::camera(uint32_t i)
{
	CameraId cam = find_component_by_index(i, m_num_cameras, m_cameras);

	CE_ASSERT(cam.id != INVALID_ID, "Unit does not have camera with index '%d'", i);

	return m_world.get_camera(cam);
}

//-----------------------------------------------------------------------------
Mesh* Unit::mesh(const char* name)
{
	MeshId mesh = find_component(name, m_num_meshes, m_meshes);

	CE_ASSERT(mesh.id != INVALID_ID, "Unit does not have mesh with name '%s'", name);

	return m_world.render_world()->get_mesh(mesh);
}

//-----------------------------------------------------------------------------
Mesh* Unit::mesh(uint32_t i)
{
	MeshId mesh = find_component_by_index(i, m_num_meshes, m_meshes);

	CE_ASSERT(mesh.id != INVALID_ID, "Unit does not have mesh with index '%d'", i);

	return m_world.render_world()->get_mesh(mesh);
}

//-----------------------------------------------------------------------------
Sprite*	Unit::sprite(const char* name)
{
	SpriteId sprite = find_component(name, m_num_sprites, m_sprites);

	CE_ASSERT(sprite.id != INVALID_ID, "Unit does not have sprite with name '%s'", name);

	return m_world.render_world()->get_sprite(sprite);
}

//-----------------------------------------------------------------------------
Sprite*	Unit::sprite(uint32_t i)
{
	SpriteId sprite = find_component_by_index(i, m_num_sprites, m_sprites);

	CE_ASSERT(sprite.id != INVALID_ID, "Unit does not have sprite with index '%d'", i);

	return m_world.render_world()->get_sprite(sprite);
}

//-----------------------------------------------------------------------------
Actor* Unit::actor(const char* name)
{
	ActorId actor = find_component(name, m_num_actors, m_actors);

	CE_ASSERT(actor.id != INVALID_ID, "Unit does not have actor with name '%s'", name);

	return m_world.physics_world()->get_actor(actor);
}

//-----------------------------------------------------------------------------
Actor* Unit::actor(uint32_t i)
{
	ActorId actor = find_component_by_index(i, m_num_actors, m_actors);

	CE_ASSERT(actor.id != INVALID_ID, "Unit does not have actor with index '%d'", i);

	return m_world.physics_world()->get_actor(actor);
}

//-----------------------------------------------------------------------------
Actor* Unit::actor_by_index(StringId32 name)
{
	ActorId actor = find_component_by_name(name, m_num_actors, m_actors);

	// CE_ASSERT(actor.id != INVALID_ID, "Unit does not have actor with name '%d'", name);

	return m_world.physics_world()->get_actor(actor);
}

//-----------------------------------------------------------------------------
Controller* Unit::controller()
{
	if (m_controller.component.id != INVALID_ID)
	{
		return m_world.physics_world()->get_controller(m_controller.component);
	}

	return NULL;
}

//-----------------------------------------------------------------------------
Material* Unit::material(const char* name)
{
/*	MaterialId material = find_component(name, m_num_materials, m_materials);

	CE_ASSERT(material.id != INVALID_ID, "Unit does not have material with name '%s'", name);

	return m_world.render_world()->get_material(material);*/
	return NULL;
}

//-----------------------------------------------------------------------------
Material* Unit::material(uint32_t i)
{
/*	MaterialId material = find_component_by_index(i, m_num_materials, m_materials);

	CE_ASSERT(material.id != INVALID_ID, "Unit does not have material with name '%d'", i);

	return m_world.render_world()->get_material(material);*/
	return NULL;
}

//-----------------------------------------------------------------------------
bool Unit::is_a(const char* name)
{
	return m_resource_id == ResourceId("unit", name);
}

//-----------------------------------------------------------------------------
void Unit::play_sprite_animation(const char* name, bool loop)
{
	sprite(0u)->play_animation(name, loop);
}

//-----------------------------------------------------------------------------
void Unit::stop_sprite_animation()
{
	sprite(0u)->stop_animation();
}

//-----------------------------------------------------------------------------
bool Unit::has_key(const char* k) const
{
	return m_resource->has_key(k);
}

//-----------------------------------------------------------------------------
ValueType::Enum Unit::value_type(const char* k)
{
	Key key;
	m_resource->get_key(k, key);
	return (ValueType::Enum) key.type;
}

//-----------------------------------------------------------------------------
bool Unit::get_key(const char* k, bool& v) const
{
	Key key;
	bool has = m_resource->get_key(k, key);
	v = *(uint32_t*)(m_values + key.offset);
	return has;
}

//-----------------------------------------------------------------------------
bool Unit::get_key(const char* k, float& v) const
{
	Key key;
	bool has = m_resource->get_key(k, key);
	v = *(float*)(m_values + key.offset);
	return has;
}

//-----------------------------------------------------------------------------
bool Unit::get_key(const char* k, StringId32& v) const
{
	Key key;
	bool has = m_resource->get_key(k, key);
	v = *(StringId32*)(m_values + key.offset);
	return has;
}

//-----------------------------------------------------------------------------
bool Unit::get_key(const char* k, Vector3& v) const
{
	Key key;
	bool has = m_resource->get_key(k, key);
	v = *(Vector3*)(m_values + key.offset);
	return has;
}

//-----------------------------------------------------------------------------
void Unit::set_key(const char* k, bool v)
{
	Key key;
	m_resource->get_key(k, key);
	*(uint32_t*)(m_values + key.offset) = v;
}

//-----------------------------------------------------------------------------
void Unit::set_key(const char* k, float v)
{
	Key key;
	m_resource->get_key(k, key);
	*(float*)(m_values + key.offset) = v;
}

//-----------------------------------------------------------------------------
void Unit::set_key(const char* k, const char* v)
{
	Key key;
	m_resource->get_key(k, key);
	*(StringId32*)(m_values + key.offset) = string::murmur2_32(v, string::strlen(v));
}

//-----------------------------------------------------------------------------
void Unit::set_key(const char* k, const Vector3& v)
{
	Key key;
	m_resource->get_key(k, key);
	*(Vector3*)(m_values + key.offset) = v;
}

} // namespace crown
