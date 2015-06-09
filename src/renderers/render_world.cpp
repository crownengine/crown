/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "render_world.h"
#include "memory.h"
#include "camera.h"
#include "log.h"
#include "sprite_resource.h"
#include "sprite.h"
#include "material.h"
#include "config.h"
#include "gui.h"
#include "mesh_resource.h"
#include "scene_graph.h"
#include <bgfx.h>

namespace crown
{

RenderWorld::RenderWorld()
	: m_sprite_pool(default_allocator(), MAX_SPRITES, sizeof(Sprite), CE_ALIGNOF(Sprite))
	, m_gui_pool(default_allocator(), MAX_GUIS, sizeof(Gui), CE_ALIGNOF(Gui))
{
}

RenderWorld::~RenderWorld()
{
}

SpriteId RenderWorld::create_sprite(SpriteResource* sr, SceneGraph& sg, UnitId id)
{
	Sprite* sprite = CE_NEW(m_sprite_pool, Sprite)(*this, sg, id, sr);
	return id_array::create(m_sprite, sprite);
}

void RenderWorld::destroy_sprite(SpriteId id)
{
	CE_DELETE(m_sprite_pool, id_array::get(m_sprite, id));
	id_array::destroy(m_sprite, id);
}

Sprite*	RenderWorld::get_sprite(SpriteId id)
{
	return id_array::get(m_sprite, id);
}

GuiId RenderWorld::create_gui(uint16_t width, uint16_t height, const char* material)
{
	Gui* gui = CE_NEW(m_gui_pool, Gui)(width, height, material);
	GuiId id = id_array::create(m_guis, gui);
	gui->set_id(id);
	return id;
}

void RenderWorld::destroy_gui(GuiId id)
{
	CE_DELETE(m_gui_pool, id_array::get(m_guis, id));
	id_array::destroy(m_guis, id);
}

Gui* RenderWorld::get_gui(GuiId id)
{
	return id_array::get(m_guis, id);
}

void RenderWorld::update(const Matrix4x4& view, const Matrix4x4& projection, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
	// Set view 0 clear state.
	bgfx::setViewClear(0
		, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
		, 0x353839FF
		, 1.0f
		, 0);

	// Set view and projection matrix for view 0.
	bgfx::setViewTransform(0, to_float_ptr(view), to_float_ptr(projection));
	bgfx::setViewRect(0, 0, 0, width, height);

	// This dummy draw call is here to make sure that view 0 is cleared
	// if no other draw calls are submitted to view 0.
	bgfx::submit(0);

	// Draw all sprites
	for (uint32_t s = 0; s < id_array::size(m_sprite); s++)
	{
		m_sprite[s]->render();
	}
}

} // namespace crown
