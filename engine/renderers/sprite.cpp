/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
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

Sprite::Sprite(RenderWorld& render_world, SceneGraph& sg, int32_t node, const SpriteResource* sr)
	: m_render_world(render_world)
	, m_scene_graph(sg)
	, m_node(node)
	, m_resource(sr)
	, m_frame(0)
	, _depth(0)
{
}

Vector3 Sprite::local_position() const
{
	return m_scene_graph.local_position(m_node);
}

Quaternion Sprite::local_rotation() const
{
	return m_scene_graph.local_rotation(m_node);
}

Matrix4x4 Sprite::local_pose() const
{
	return m_scene_graph.local_pose(m_node);
}

Vector3 Sprite::world_position() const
{
	return m_scene_graph.world_position(m_node);
}

Quaternion Sprite::world_rotation() const
{
	return m_scene_graph.world_rotation(m_node);
}

Matrix4x4 Sprite::world_pose() const
{
	return m_scene_graph.world_pose(m_node);
}

void Sprite::set_local_position(Unit* unit, const Vector3& pos)
{
	unit->set_local_position(m_node, pos);
}

void Sprite::set_local_rotation(Unit* unit, const Quaternion& rot)
{
	unit->set_local_rotation(m_node, rot);
}

void Sprite::set_local_pose(Unit* unit, const Matrix4x4& pose)
{
	unit->set_local_pose(m_node, pose);
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
	if (m_material.id != INVALID_ID)
		material_manager::get()->lookup_material(m_material)->bind();

	bgfx::setState(BGFX_STATE_RGB_WRITE
		| BGFX_STATE_ALPHA_WRITE
		| BGFX_STATE_DEPTH_TEST_LEQUAL
		| BGFX_STATE_DEPTH_WRITE
		| BGFX_STATE_CULL_CW
		| BGFX_STATE_MSAA
		| BGFX_STATE_BLEND_ALPHA);
	bgfx::setVertexBuffer(m_resource->vb);
	bgfx::setIndexBuffer(m_resource->ib, m_frame * 6, 6);
	bgfx::setTransform(matrix4x4::to_float_ptr(world_pose()));
	bgfx::submit(0, _depth);
}

} // namespace crown
