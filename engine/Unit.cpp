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
#include "World.h"
#include "Allocator.h"
#include "Log.h"
#include "UnitResource.h"
#include "SceneGraphManager.h"
#include "PhysicsGraphManager.h"

#include "Actor.h"

namespace crown
{

static int count = 0;

typedef Id CameraId;
typedef Id SpriteId;

//-----------------------------------------------------------------------------
Unit::Unit(World& w, SceneGraph& sg, const UnitResource* ur, const Matrix4x4& pose)
	: m_world(w)
	, m_scene_graph(sg)
	, m_resource(ur)
	, m_num_cameras(0)
	, m_num_meshes(0)
	, m_num_sprites(0)
{
	// Log debug info
	Log::d("Creating unit...");
	Log::d("Num scene graph nodes = %d", ur->num_scene_graph_nodes());
	Log::d("Num renderables       = %d", ur->num_renderables());
	Log::d("Num cameras           = %d", ur->num_cameras());
	Log::d("Num actors            = %d", ur->num_actors());

	create(pose);
}

//-----------------------------------------------------------------------------
Unit::~Unit()
{
	// Destroy cameras
	for (uint32_t i = 0; i < m_num_cameras; i++)
	{
		m_world.destroy_camera(m_cameras[i].component);
	}

	// Destroy meshes
	for (uint32_t i = 0; i < m_num_meshes; i++)
	{
		m_world.destroy_mesh(m_meshes[i].component);
	}

	// Destroy sprites
	for (uint32_t i = 0; i < m_num_sprites; i++)
	{
		m_world.destroy_sprite(m_sprites[i].component);
	}

	// Destroy actors
	for (uint32_t i = 0; i < m_num_actors; i++)
	{
		m_world.destroy_actor(m_actors[i].component);
	}
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
void Unit::create(const Matrix4x4& pose)
{
	// Create the scene graph
	m_scene_graph.create(m_resource->num_scene_graph_nodes(), m_resource->scene_graph_names(),
							m_resource->scene_graph_poses(), m_resource->scene_graph_parents());
	// Set root node's pose
	m_scene_graph.set_local_pose(0, pose);

	create_camera_objects();
	create_renderable_objects();
	create_physics_objects();
}

//-----------------------------------------------------------------------------
void Unit::create_camera_objects()
{
	for (uint32_t i = 0; i < m_resource->num_cameras(); i++)
	{
		const UnitCamera camera = m_resource->get_camera(i);
		const CameraId cam = m_world.create_camera(m_scene_graph, camera.node);
		add_camera(camera.name, cam);
	}
}

//-----------------------------------------------------------------------------
void Unit::create_renderable_objects()
{
	for (uint32_t i = 0; i < m_resource->num_renderables(); i++)
	{
		const UnitRenderable renderable = m_resource->get_renderable(i);

		switch (renderable.type)
		{
			case UnitRenderable::MESH: add_mesh(renderable.name, m_world.create_mesh(renderable.resource, m_scene_graph, renderable.node)); break;
			case UnitRenderable::SPRITE: add_sprite(renderable.name, m_world.create_sprite(renderable.resource, m_scene_graph, renderable.node)); break;
			default: CE_FATAL("Oops, bad renderable type"); break;
		}
	}
}

//-----------------------------------------------------------------------------
void Unit::create_physics_objects()
{
	const StringId32 name_hash = hash::murmur2_32("actor", string::strlen("actor"), 0);
	add_actor(name_hash, m_world.create_actor(ActorType::DYNAMIC_PHYSICAL));
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
	uint32_t name_hash = hash::murmur2_32(name, string::strlen(name), 0);

	Id comp;
	comp.id = INVALID_ID;

	for (uint32_t i = 0; i < size; i++)
	{
		if (name_hash == array[i].name)
		{
			comp = array[i].component;
		}
	}

	return comp;
}

//-----------------------------------------------------------------------------
Id Unit::find_component(uint32_t index, uint32_t size, Component* array)
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
void Unit::add_camera(StringId32 name, CameraId camera)
{
	CE_ASSERT(m_num_cameras < MAX_CAMERA_COMPONENTS, "Max camera number reached");

	add_component(name, camera, m_num_cameras, m_cameras);
}

//-----------------------------------------------------------------------------
void Unit::add_mesh(StringId32 name, MeshId mesh)
{
	CE_ASSERT(m_num_meshes < MAX_MESH_COMPONENTS, "Max mesh number reached");

	add_component(name, mesh, m_num_meshes, m_meshes);
}

//-----------------------------------------------------------------------------
void Unit::add_sprite(StringId32 name, SpriteId sprite)
{
	CE_ASSERT(m_num_sprites < MAX_SPRITE_COMPONENTS, "Max sprite number reached");

	add_component(name, sprite, m_num_sprites, m_sprites);
}

//-----------------------------------------------------------------------------
void Unit::add_actor(StringId32 name, ActorId actor)
{
	CE_ASSERT(m_num_actors < MAX_ACTOR_COMPONENTS, "Max actor number reached");

	add_component(name, actor, m_num_actors, m_actors);
}

//-----------------------------------------------------------------------------
Camera* Unit::camera(const char* name)
{
	CameraId cam = find_component(name, m_num_cameras, m_cameras);

	CE_ASSERT(cam.id != INVALID_ID, "Unit does not have camera with name '%s'", name);

	return m_world.lookup_camera(cam);
}

//-----------------------------------------------------------------------------
Camera* Unit::camera(uint32_t i)
{
	CameraId cam = find_component(i, m_num_cameras, m_cameras);

	CE_ASSERT(cam.id != INVALID_ID, "Unit does not have camera with index '%d'", i);

	return m_world.lookup_camera(cam);
}

//-----------------------------------------------------------------------------
Mesh* Unit::mesh(const char* name)
{
	MeshId mesh = find_component(name, m_num_meshes, m_meshes);

	CE_ASSERT(mesh.id != INVALID_ID, "Unit does not have mesh with name '%s'", name);

	return m_world.lookup_mesh(mesh);
}

//-----------------------------------------------------------------------------
Mesh* Unit::mesh(uint32_t i)
{
	MeshId mesh = find_component(i, m_num_meshes, m_meshes);

	CE_ASSERT(mesh.id != INVALID_ID, "Unit does not have mesh with index '%d'", i);

	return m_world.lookup_mesh(mesh);
}

//-----------------------------------------------------------------------------
Sprite*	Unit::sprite(const char* name)
{
	SpriteId sprite = find_component(name, m_num_sprites, m_sprites);

	CE_ASSERT(sprite.id != INVALID_ID, "Unit does not have sprite with name '%s'", name);

	return m_world.lookup_sprite(sprite);
}

//-----------------------------------------------------------------------------
Sprite*	Unit::sprite(uint32_t i)
{
	SpriteId sprite = find_component(i, m_num_sprites, m_sprites);

	CE_ASSERT(sprite.id != INVALID_ID, "Unit does not have sprite with index '%d'", i);

	return m_world.lookup_sprite(sprite);
}

//-----------------------------------------------------------------------------
Actor* Unit::actor(const char* name)
{
	ActorId actor = find_component(name, m_num_actors, m_actors);

	CE_ASSERT(actor.id != INVALID_ID, "Unit does not have actor with name '%s'", name);

	return m_world.lookup_actor(actor);
}

//-----------------------------------------------------------------------------
Actor* Unit::actor(uint32_t i)
{
	ActorId actor = find_component(i, m_num_actors, m_actors);

	CE_ASSERT(actor.id != INVALID_ID, "Unit does not have actor with name '%d'", i);

	return m_world.lookup_actor(actor);
}	


} // namespace crown
