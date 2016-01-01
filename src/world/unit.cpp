/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "unit.h"
#include "world.h"
#include "memory.h"
#include "log.h"
#include "unit_resource.h"
#include "actor.h"
#include "controller.h"
#include "physics_resource.h"
#include "device.h"
#include "resource_manager.h"
#include "sprite.h"
#include "sprite_animation_player.h"

namespace crown
{

using namespace unit_resource;

Unit::Unit(World& w, UnitId unit_id, const UnitResource* ur, SceneGraph& sg, const Matrix4x4& pose)
	: m_world(w)
	, m_scene_graph(sg)
	, m_sprite_animation(NULL)
	, m_resource(ur)
	, m_id(unit_id)
	, m_num_cameras(0)
	, m_num_sprites(0)
	, m_num_actors(0)
	, m_num_materials(0)
{
	m_controller.component.id = INVALID_ID;
	create_objects(pose);
}

Unit::~Unit()
{
	destroy_objects();
}

void Unit::set_id(const UnitId id)
{
	m_id = id;
}

UnitId Unit::id()
{
	return m_id;
}

const UnitResource*	Unit::resource() const
{
	return m_resource;
}

void Unit::create_objects(const Matrix4x4& pose)
{
	using namespace unit_resource;
	m_scene_graph.create(pose, m_id);

	create_camera_objects();
	create_renderable_objects();
	create_physics_objects();

	set_default_material();

	StringId64 anim_id = sprite_animation(m_resource);
	if (anim_id.id() != 0)
	{
		m_sprite_animation = m_world.sprite_animation_player()->create_sprite_animation((SpriteAnimationResource*) device()->resource_manager()->get(SPRITE_ANIMATION_TYPE, anim_id));
	}
}

void Unit::destroy_objects()
{
	if (m_sprite_animation)
	{
		m_world.sprite_animation_player()->destroy_sprite_animation(m_sprite_animation);
	}

	// Destroy cameras
	for (uint32_t i = 0; i < m_num_cameras; i++)
	{
		m_world.destroy_camera(m_cameras[i].component);
	}
	m_num_cameras = 0;

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

	// Destroy materials
	for (uint32_t i = 0; i < m_num_materials; i++)
	{
		material_manager::get()->destroy_material(m_materials[i].component);
	}
	m_num_materials = 0;

	// Destroy scene graph
	m_scene_graph.destroy(m_scene_graph.get(m_id));
}

void Unit::create_camera_objects()
{
	for (uint32_t i = 0; i < num_cameras(m_resource); i++)
	{
		const UnitCamera* cam = get_camera(m_resource, i);
		const CameraId id = m_world.create_camera(m_scene_graph, m_id, (ProjectionType::Enum)cam->type, cam->near, cam->far);
		add_camera(cam->name, id);
	}
}

void Unit::create_renderable_objects()
{
	for (uint32_t i = 0; i < num_materials(m_resource); i++)
	{
		const UnitMaterial* mat = get_material(m_resource, i);
		add_material(StringId32("default"), material_manager::get()->create_material(mat->id));
	}

	// Create renderables
	for (uint32_t i = 0; i < num_renderables(m_resource); i++)
	{
		const UnitRenderable* ur = get_renderable(m_resource, i);

		if (ur->type == UnitRenderable::MESH)
		{
			// MeshResource* mr = (MeshResource*) device()->resource_manager()->get(MESH_TYPE, ur->resource);
			// m_world.render_world()->create_mesh(mr, m_materials[0].component, m_scene_graph, ur->node);
		}
		else if (ur->type == UnitRenderable::SPRITE)
		{
			SpriteResource* sr = (SpriteResource*) device()->resource_manager()->get(SPRITE_TYPE, ur->resource);
			SpriteId sprite = m_world.render_world()->create_sprite(sr, m_scene_graph, m_id);
			add_sprite(ur->name, sprite);
		}
		else
		{
			CE_FATAL("Oops, bad renderable type");
		}
	}
}

void Unit::create_physics_objects()
{
	using namespace unit_resource;
	using namespace physics_resource;
	if (unit_resource::physics_resource(m_resource).id() != 0)
	{
		const PhysicsResource* pr = (PhysicsResource*) device()->resource_manager()->get(PHYSICS_TYPE, unit_resource::physics_resource(m_resource));

		// Create controller if any
		if (has_controller(pr))
		{
			const ControllerResource* cr = physics_resource::controller(pr);
			set_controller(cr->name, m_world.physics_world()->create_controller(cr, m_scene_graph, m_id));
		}

		// Create actors if any
		for (uint32_t i = 0; i < num_actors(pr); i++)
		{
			const ActorResource* ar = physics_resource::actor(pr, i);

			ActorId id = m_world.physics_world()->create_actor(ar, m_scene_graph, m_id);
			add_actor(ar->name, id);
		}

		// Create joints if any
		for (uint32_t i = 0; i < num_joints(pr); i++)
		{
			const JointResource* jr = physics_resource::joint(pr, i);
			m_world.physics_world()->create_joint(jr, *actor_by_index(jr->actor_0), *actor_by_index(jr->actor_1));
		}
	}
}

void Unit::set_default_material()
{
	if (m_num_materials == 0) return;

	for (uint32_t i = 0; i < m_num_sprites; i++)
	{
		Sprite* s = m_world.render_world()->get_sprite(m_sprites[i].component);
		s->set_material(m_materials[0].component);
	}
}

Vector3 Unit::local_position() const
{
	TransformInstance ti = m_scene_graph.get(m_id);
	return m_scene_graph.local_position(ti);
}

Quaternion Unit::local_rotation() const
{
	TransformInstance ti = m_scene_graph.get(m_id);
	return m_scene_graph.local_rotation(ti);
}

Vector3 Unit::local_scale() const
{
	TransformInstance ti = m_scene_graph.get(m_id);
	return m_scene_graph.local_scale(ti);
}

Matrix4x4 Unit::local_pose() const
{
	TransformInstance ti = m_scene_graph.get(m_id);
	return m_scene_graph.local_pose(ti);
}

Vector3 Unit::world_position() const
{
	TransformInstance ti = m_scene_graph.get(m_id);
	return m_scene_graph.world_position(ti);
}

Quaternion Unit::world_rotation() const
{
	TransformInstance ti = m_scene_graph.get(m_id);
	return m_scene_graph.world_rotation(ti);
}

Matrix4x4 Unit::world_pose() const
{
	TransformInstance ti = m_scene_graph.get(m_id);
	return m_scene_graph.world_pose(ti);
}

void Unit::set_local_position(const Vector3& pos)
{
	TransformInstance ti = m_scene_graph.get(m_id);
	m_scene_graph.set_local_position(ti, pos);
}

void Unit::set_local_rotation(const Quaternion& rot)
{
	TransformInstance ti = m_scene_graph.get(m_id);
	m_scene_graph.set_local_rotation(ti, rot);
}

void Unit::set_local_scale(const Vector3& scale)
{
	TransformInstance ti = m_scene_graph.get(m_id);
	m_scene_graph.set_local_scale(ti, scale);
}

void Unit::set_local_pose(const Matrix4x4& pose)
{
	TransformInstance ti = m_scene_graph.get(m_id);
	m_scene_graph.set_local_pose(ti, pose);
}

void Unit::update()
{
	if (m_sprite_animation)
	{
		sprite(0u)->set_frame(m_sprite_animation->m_cur_frame);
	}
}

void Unit::reload(UnitResource* new_ur)
{
	TransformInstance ti = m_scene_graph.get(m_id);
	Matrix4x4 m = m_scene_graph.world_pose(ti);
	destroy_objects();
	m_resource = new_ur;
	create_objects(m);
}

void Unit::add_component(StringId32 name, Id component, uint32_t& size, Component* array)
{
	Component comp;
	comp.name = name;
	comp.component = component;

	array[size] = comp;
	size++;
}

Id Unit::find_component(const char* name, uint32_t size, Component* array)
{
	return find_component_by_name(StringId32(name), size, array);
}

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

void Unit::add_camera(StringId32 name, CameraId camera)
{
	CE_ASSERT(m_num_cameras < CE_MAX_CAMERA_COMPONENTS, "Max camera number reached");

	add_component(name, camera, m_num_cameras, m_cameras);
}

void Unit::add_sprite(StringId32 name, SpriteId sprite)
{
	CE_ASSERT(m_num_sprites < CE_MAX_SPRITE_COMPONENTS, "Max sprite number reached");

	add_component(name, sprite, m_num_sprites, m_sprites);
}

void Unit::add_actor(StringId32 name, ActorId actor)
{
	CE_ASSERT(m_num_actors < CE_MAX_ACTOR_COMPONENTS, "Max actor number reached");

	add_component(name, actor, m_num_actors, m_actors);
}

void Unit::add_material(StringId32 name, MaterialId material)
{
	CE_ASSERT(m_num_materials < CE_MAX_MATERIAL_COMPONENTS, "Max material number reached");

	add_component(name, material, m_num_materials, m_materials);
}

void Unit::set_controller(StringId32 name, ControllerId controller)
{
	m_controller.name = name;
	m_controller.component = controller;
}

Camera* Unit::camera(const char* name)
{
	CameraId cam = find_component(name, m_num_cameras, m_cameras);

	CE_ASSERT(cam.id != INVALID_ID, "Unit does not have camera with name '%s'", name);

	return m_world.get_camera(cam);
}

Camera* Unit::camera(uint32_t i)
{
	CameraId cam = find_component_by_index(i, m_num_cameras, m_cameras);

	CE_ASSERT(cam.id != INVALID_ID, "Unit does not have camera with index '%d'", i);

	return m_world.get_camera(cam);
}

Sprite*	Unit::sprite(const char* name)
{
	SpriteId sprite = find_component(name, m_num_sprites, m_sprites);

	CE_ASSERT(sprite.id != INVALID_ID, "Unit does not have sprite with name '%s'", name);

	return m_world.render_world()->get_sprite(sprite);
}

Sprite*	Unit::sprite(uint32_t i)
{
	SpriteId sprite = find_component_by_index(i, m_num_sprites, m_sprites);

	CE_ASSERT(sprite.id != INVALID_ID, "Unit does not have sprite with index '%d'", i);

	return m_world.render_world()->get_sprite(sprite);
}

Actor* Unit::actor(const char* name)
{
	ActorId actor = find_component(name, m_num_actors, m_actors);

	CE_ASSERT(actor.id != INVALID_ID, "Unit does not have actor with name '%s'", name);

	return m_world.physics_world()->get_actor(actor);
}

Actor* Unit::actor(uint32_t i)
{
	ActorId actor = find_component_by_index(i, m_num_actors, m_actors);

	CE_ASSERT(actor.id != INVALID_ID, "Unit does not have actor with index '%d'", i);

	return m_world.physics_world()->get_actor(actor);
}

Actor* Unit::actor_by_index(StringId32 name)
{
	ActorId actor = find_component_by_name(name, m_num_actors, m_actors);

	CE_ASSERT(actor.id != INVALID_ID, "Unit does not have actor with name '%d'", name);

	return m_world.physics_world()->get_actor(actor);
}

Controller* Unit::controller()
{
	if (m_controller.component.id != INVALID_ID)
	{
		return m_world.physics_world()->get_controller(m_controller.component);
	}

	return NULL;
}

Material* Unit::material(const char* name)
{
	MaterialId material = find_component(name, m_num_materials, m_materials);
	CE_ASSERT(material.id != INVALID_ID, "Unit does not have material with name '%s'", name);
	return material_manager::get()->lookup_material(material);
}

Material* Unit::material(uint32_t i)
{
	MaterialId material = find_component_by_index(i, m_num_materials, m_materials);
	CE_ASSERT(material.id != INVALID_ID, "Unit does not have material with name '%d'", i);
	return material_manager::get()->lookup_material(material);
}

bool Unit::is_a(StringId64 name)
{
	return m_resource->name == name;
}

void Unit::play_sprite_animation(const char* name, bool loop)
{
	if (m_sprite_animation)
		m_sprite_animation->play(StringId32(name), loop);
}

void Unit::stop_sprite_animation()
{
	if (m_sprite_animation)
		m_sprite_animation->stop();
}

} // namespace crown
