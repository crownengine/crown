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

#include "id_array.h"
#include "container_types.h"
#include "pool_allocator.h"
#include "resource.h"
#include "matrix4x4.h"
#include "render_world_types.h"
#include "material_manager.h"

#define MAX_MESHES 100
#define MAX_SPRITES 512
#define MAX_GUIS 8

namespace crown
{

struct Material;
struct MaterialResource;
struct Mesh;
struct MeshResource;
struct SceneGraph;
struct Sprite;
struct SpriteResource;
struct Gui;
struct Vector2;
struct Vector3;
struct GuiResource;

/// @defgroup Graphics Graphics

/// Global graphics-related functions
///
/// @ingroup Graphics
namespace graphics_system
{
	/// Initializes the graphics system.
	/// This is the place where to create and initialize per-application objects.
	void init();

	/// It should reverse the actions performed by graphics_system::init().
	void shutdown();
}; // namespace graphics_system

/// Manages graphics objects in a World.
///
/// @ingroup Graphics
class RenderWorld
{
public:

	RenderWorld();
	~RenderWorld();

	MeshId create_mesh(MeshResource* mr, SceneGraph& sg, int32_t node);

	/// Destroys the mesh @a id.
	void destroy_mesh(MeshId id);

	/// Returns the mesh @a id.
	Mesh* get_mesh(MeshId mesh);

	SpriteId create_sprite(SpriteResource* sr, SceneGraph& sg, int32_t node);

	/// Destroys the sprite @a id.
	void destroy_sprite(SpriteId id);

	/// Creates the sprite @a id.
	Sprite* get_sprite(SpriteId id);

	GuiId create_gui(uint16_t width, uint16_t height);
	void destroy_gui(GuiId id);
	Gui* get_gui(GuiId id);

	void update(const Matrix4x4& view, const Matrix4x4& projection, uint16_t x, uint16_t y, uint16_t width, uint16_t height, float dt);

private:

	PoolAllocator m_mesh_pool;
	PoolAllocator m_sprite_pool;
	PoolAllocator m_gui_pool;

	IdArray<MAX_MESHES, Mesh*> m_mesh;
	IdArray<MAX_SPRITES, Sprite*> m_sprite;
	IdArray<MAX_GUIS, Gui*> m_guis;
};

} // namespace crown
