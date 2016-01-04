/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "id_array.h"
#include "container_types.h"
#include "math_types.h"
#include "resource_types.h"
#include "pool_allocator.h"
#include "render_world_types.h"
#include "material_manager.h"
#include "resource_types.h"

#define MAX_SPRITES 512
#define MAX_GUIS 8

namespace crown
{

struct Material;
struct SceneGraph;
struct Sprite;
struct Mesh;
struct Gui;
typedef Id UnitId;

/// @defgroup Graphics Graphics

/// Manages graphics objects in a World.
///
/// @ingroup Graphics
class RenderWorld
{
public:

	RenderWorld();
	~RenderWorld();

	SpriteId create_sprite(SpriteResource* sr, SceneGraph& sg, UnitId id);

	/// Destroys the sprite @a id.
	void destroy_sprite(SpriteId id);

	/// Creates the sprite @a id.
	Sprite* get_sprite(SpriteId id);

	GuiId create_gui(uint16_t width, uint16_t height, const char* material);
	void destroy_gui(GuiId id);
	Gui* get_gui(GuiId id);

	void update(const Matrix4x4& view, const Matrix4x4& projection, uint16_t x, uint16_t y, uint16_t width, uint16_t height);

private:

	PoolAllocator m_sprite_pool;
	PoolAllocator m_gui_pool;

	IdArray<MAX_SPRITES, Sprite*> m_sprite;
	IdArray<MAX_GUIS, Gui*> m_guis;
};

} // namespace crown
