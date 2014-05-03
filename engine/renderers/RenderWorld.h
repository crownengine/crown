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

#include "IdArray.h"
#include "ContainerTypes.h"
#include "PoolAllocator.h"
#include "Resource.h"
#include "Matrix4x4.h"
#include "RenderWorldTypes.h"
#include "RendererTypes.h"

#define MAX_MESHES 100
#define MAX_SPRITES 512
#define MAX_MATERIALS 512
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

namespace render_world_globals
{
	void init();
	void shutdown();

	GPUProgramId default_program();
	GPUProgramId default_texture_program();
	GPUProgramId default_font_program();
	GPUProgramId default_color_program();
	UniformId default_albedo_uniform();
	UniformId default_font_uniform();
	UniformId default_color_uniform();
};

class RenderWorld
{
public:

	RenderWorld();
	~RenderWorld();

	MeshId create_mesh(MeshResource* mr, SceneGraph& sg, int32_t node);
	void destroy_mesh(MeshId id);
	Mesh* get_mesh(MeshId mesh);

	SpriteId create_sprite(SpriteResource* sr, SceneGraph& sg, int32_t node);
	void destroy_sprite(SpriteId id);
	Sprite* get_sprite(SpriteId id);

	MaterialId create_material(MaterialResource* mr);
	void destroy_material(MaterialId id);
	Material* get_material(MaterialId id);

	GuiId create_gui(uint16_t width, uint16_t height);
	void destroy_gui(GuiId id);
	Gui* get_gui(GuiId id);

	void update(const Matrix4x4& view, const Matrix4x4& projection, uint16_t x, uint16_t y, uint16_t width, uint16_t height, float dt);

private:

	PoolAllocator m_mesh_pool;
	PoolAllocator m_sprite_pool;
	PoolAllocator m_material_pool;
	PoolAllocator m_gui_pool;

	IdArray<MAX_MESHES, Mesh*> m_mesh;
	IdArray<MAX_SPRITES, Sprite*> m_sprite;
	IdArray<MAX_MATERIALS, Material*> m_materials;
	IdArray<MAX_GUIS, Gui*> m_guis;
};

} // namespace crown
