/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "render_world_types.h"
#include "vector3.h"
#include "matrix4x4.h"
#include "quaternion.h"
#include "sprite_resource.h"
#include "material.h"
#include "world_types.h"
#include <bgfx.h>

namespace crown
{

struct Sprite
{
	Sprite(RenderWorld& render_world, SceneGraph& sg, UnitId id, const SpriteResource* sr);

	void set_material(MaterialId id);

	void set_frame(uint32_t i);

	void set_depth(int32_t z);

	void render();

public:

	RenderWorld& m_render_world;
	SceneGraph& m_scene_graph;
	const SpriteResource* m_resource;
	MaterialId m_material;
	uint32_t m_frame;
	int32_t _depth;
	UnitId _unit_id;
};

} // namespace crown
