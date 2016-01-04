/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "sprite.h"
#include "vector3.h"
#include "quaternion.h"
#include "sprite_resource.h"
#include "allocator.h"
#include "scene_graph.h"
#include "unit.h"
#include "material.h"
#include "render_world.h"
#include "device.h"
#include "material_manager.h"

namespace crown
{

Sprite::Sprite(RenderWorld& render_world, SceneGraph& sg, UnitId id, const SpriteResource* sr)
	: m_render_world(render_world)
	, m_scene_graph(sg)
	, m_resource(sr)
	, m_frame(0)
	, _depth(0)
	, _unit_id(id)
{
}

void Sprite::set_material(MaterialId id)
{
	m_material = id;
}

void Sprite::set_frame(uint32_t i)
{
	m_frame = i;
}

void Sprite::set_depth(int32_t z)
{
	_depth = z;
}

void Sprite::render()
{
	bgfx::setVertexBuffer(m_resource->vb);
	bgfx::setIndexBuffer(m_resource->ib, m_frame * 6, 6);
	TransformInstance ti = m_scene_graph.get(_unit_id);
	bgfx::setTransform(to_float_ptr(m_scene_graph.world_pose(ti)));

	if (m_material.id != INVALID_ID)
		material_manager::get()->lookup_material(m_material)->bind();
}

} // namespace crown
