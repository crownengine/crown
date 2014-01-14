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

#include "RenderWorld.h"
#include "Device.h"
#include "Renderer.h"
#include "Allocator.h"
#include "Camera.h"
#include "Resource.h"
#include "Log.h"
#include "SpriteResource.h"
#include "Mesh.h"
#include "Sprite.h"
#include "Material.h"
#include "Config.h"
#include "Gui.h"
#include "GuiResource.h"

namespace crown
{

TextureId				grass_texture;
TextureId				lightmap_texture;
ShaderId				default_vs;
ShaderId				default_fs;
ShaderId				texture_fs;
GPUProgramId			default_program;
GPUProgramId			texture_program;
UniformId				u_albedo_0;
UniformId				u_lightmap_0;
UniformId				u_brightness;

static const char* default_vertex =
	"uniform mat4      	u_model;"
	"uniform mat4      	u_model_view_projection;"

	"attribute vec4    	a_position;"
	"attribute vec2    	a_tex_coord0;"
	"attribute vec4    	a_color;"

	"varying vec2		tex_coord0;"
	"varying vec4		color;"

	"void main(void)"
	"{"
	"	tex_coord0 = a_tex_coord0;"
	"   color = a_color;"
	"	gl_Position = u_model_view_projection * a_position;"
	"}";

static const char* default_fragment = 
	"void main(void)"
	"{"
	"	gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);"
	"}";

static const char* texture_fragment = 
	"varying vec2       tex_coord0;"
	"varying vec4       color;"

	"uniform sampler2D  u_albedo_0;"

	"void main(void)"
	"{"
	"	gl_FragColor = texture2D(u_albedo_0, tex_coord0);"
	"}";

//-----------------------------------------------------------------------------
RenderWorld::RenderWorld()
	: m_mesh_pool(default_allocator(), MAX_MESHES, sizeof(Mesh), CE_ALIGNOF(Mesh))
	, m_sprite_pool(default_allocator(), MAX_SPRITES, sizeof(Sprite), CE_ALIGNOF(Sprite))
	, m_material_pool(default_allocator(), MAX_MATERIALS, sizeof(Material), CE_ALIGNOF(Material))
	, m_gui_pool(default_allocator(), MAX_GUIS, sizeof(Gui), CE_ALIGNOF(Gui))
{
	Renderer* r = device()->renderer();

	default_vs = r->create_shader(ShaderType::VERTEX, default_vertex);
	default_fs = r->create_shader(ShaderType::FRAGMENT, default_fragment);
	texture_fs = r->create_shader(ShaderType::FRAGMENT, texture_fragment);

	u_albedo_0 = r->create_uniform("u_albedo_0", UniformType::INTEGER_1, 1);

	default_program = r->create_gpu_program(default_vs, default_fs);
	texture_program = r->create_gpu_program(default_vs, texture_fs);
}

//-----------------------------------------------------------------------------
RenderWorld::~RenderWorld()
{
	Renderer* r = device()->renderer();

	r->destroy_shader(default_vs);
	r->destroy_shader(default_fs);
	r->destroy_shader(texture_fs);
	r->destroy_gpu_program(default_program);
	r->destroy_gpu_program(texture_program);
	r->destroy_uniform(u_albedo_0);
}

//-----------------------------------------------------------------------------
MeshId RenderWorld::create_mesh(MeshResource* mr, SceneGraph& sg, int32_t node)
{
	Mesh* mesh = CE_NEW(m_mesh_pool, Mesh)(sg, node, mr);
	return m_mesh.create(mesh);
}

//-----------------------------------------------------------------------------
void RenderWorld::destroy_mesh(MeshId id)
{
	CE_ASSERT(m_mesh.has(id), "Mesh does not exist");

	Mesh* mesh = m_mesh.lookup(id);
	CE_DELETE(m_mesh_pool, mesh);
	m_mesh.destroy(id);
}

//-----------------------------------------------------------------------------
Mesh* RenderWorld::lookup_mesh(MeshId mesh)
{
	CE_ASSERT(m_mesh.has(mesh), "Mesh does not exits");

	return m_mesh.lookup(mesh);
}

//-----------------------------------------------------------------------------
SpriteId RenderWorld::create_sprite(SpriteResource* sr, SceneGraph& sg, int32_t node)
{
	Sprite* sprite = CE_NEW(m_sprite_pool, Sprite)(*this, sg, node, sr);
	return m_sprite.create(sprite);
}

//-----------------------------------------------------------------------------
void RenderWorld::destroy_sprite(SpriteId id)
{
	CE_ASSERT(m_sprite.has(id), "Sprite does not exist");

	Sprite* sprite = m_sprite.lookup(id);
	CE_DELETE(m_sprite_pool, sprite);
	m_sprite.destroy(id);
}

//-----------------------------------------------------------------------------
Sprite*	RenderWorld::lookup_sprite(SpriteId id)
{
	CE_ASSERT(m_sprite.has(id), "Sprite does not exist");

	return m_sprite.lookup(id);
}

//-----------------------------------------------------------------------------
MaterialId RenderWorld::create_material(MaterialResource* mr)
{
	Material* mat = CE_NEW(m_material_pool, Material)(mr);
	return m_materials.create(mat);
}

//-----------------------------------------------------------------------------
void RenderWorld::destroy_material(MaterialId id)
{
	CE_DELETE(m_material_pool, m_materials.lookup(id));
	m_materials.destroy(id);
}

//-----------------------------------------------------------------------------
Material* RenderWorld::lookup_material(MaterialId id)
{
	return m_materials.lookup(id);
}

//-----------------------------------------------------------------------------
GuiId RenderWorld::create_gui(GuiResource* gr)
{
	Renderer* r = device()->renderer();
	Gui* gui = CE_NEW(m_gui_pool, Gui)(*this, gr, *r);
	return m_guis.create(gui);
}

//-----------------------------------------------------------------------------
void RenderWorld::destroy_gui(GuiId id)
{
	CE_ASSERT(m_guis.has(id), "Gui does not exist");

	CE_DELETE(m_gui_pool, m_guis.lookup(id));
	m_guis.destroy(id);
}

//-----------------------------------------------------------------------------
Gui* RenderWorld::lookup_gui(GuiId id)
{
	CE_ASSERT(m_guis.has(id), "Gui does not exist");
	
	return m_guis.lookup(id);
}

//-----------------------------------------------------------------------------
void RenderWorld::update(const Matrix4x4& view, const Matrix4x4& projection, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
	Renderer* r = device()->renderer();

	Matrix4x4 inv_view = view;
	inv_view.invert();

	r->set_layer_view(0, inv_view);
	r->set_layer_projection(0, projection);
	r->set_layer_viewport(0, x, y, width, height);
	r->set_layer_clear(0, CLEAR_COLOR | CLEAR_DEPTH, Color4::LIGHTBLUE, 1.0f);

	r->set_state(STATE_DEPTH_WRITE | STATE_COLOR_WRITE | STATE_CULL_CCW);
	r->commit(0);

	// Draw all meshes
	for (uint32_t m = 0; m < m_mesh.size(); m++)
	{
		const Mesh* mesh = m_mesh.m_objects[m];

		r->set_state(STATE_DEPTH_WRITE | STATE_COLOR_WRITE | STATE_ALPHA_WRITE | STATE_CULL_CW);
		r->set_vertex_buffer(mesh->m_vbuffer);
		r->set_index_buffer(mesh->m_ibuffer);
		r->set_program(default_program);
		// r->set_texture(0, u_albedo_0, grass_texture, TEXTURE_FILTER_LINEAR | TEXTURE_WRAP_CLAMP_EDGE);
		// r->set_uniform(u_brightness, UNIFORM_FLOAT_1, &brightness, 1);

		r->set_pose(mesh->world_pose());
		r->commit(0);
	}

	// Draw all sprites
	for (uint32_t s = 0; s < m_sprite.size(); s++)
	{
		r->set_program(texture_program);
		m_sprite[s]->render(*r, u_albedo_0);
	}

	// Draw all guis
	for (uint32_t g = 0; g < m_guis.size(); g++)
	{
		m_guis[g]->render();
	}
}

} // namespace crown
