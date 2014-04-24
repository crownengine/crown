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

namespace render_world_globals
{
	static const char* default_vertex =
		"precision mediump float;\n"
		"uniform mat4      	u_model_view_projection;\n"

		"attribute vec4    	a_position;\n"
		"attribute vec2    	a_tex_coord0;\n"
		"attribute vec4		a_color;\n"

		"varying vec2		tex_coord0;\n"
		"varying vec4		color;\n"

		"void main(void)\n"
		"{\n"
		"	tex_coord0 = a_tex_coord0;\n"
		"	color = a_color;\n"
		"	gl_Position = u_model_view_projection * a_position;\n"
		"}\n";

	static const char* default_fragment = 
		"precision mediump float;\n"
		"varying vec4 color;\n"
		"void main(void)\n"
		"{\n"
		"	gl_FragColor = color;\n"
		"}\n";

	static const char* texture_fragment = 
		"precision mediump float;\n"
		"varying vec2       tex_coord0;\n"
		"varying vec4       color;\n"

		"uniform sampler2D  u_albedo_0;\n"

		"void main(void)\n"
		"{\n"
		"	gl_FragColor = texture2D(u_albedo_0, tex_coord0);\n"
		"}\n";

	static const char* sdf_vertex =
		"precision mediump float;\n"
		"uniform mat4      	u_model_view_projection;\n"

		"attribute vec4		a_position;\n"
		"attribute vec2		a_tex_coord0;\n"

		"varying vec2		v_tex_coord;\n"
		"varying vec4		v_color;\n"

		"void main(void)\n"
		"{\n"
		"	gl_Position = u_model_view_projection * a_position;\n"
		"	v_tex_coord = a_tex_coord0;\n"
		"	v_color = vec4(1, 1, 1, 1);\n"
		"}\n";

	static const char* sdf_fragment =
		"precision mediump float;\n"
		"uniform sampler2D u_texture;\n"

		"varying vec2 v_tex_coord;\n"
		"varying vec4 v_color;\n"

		"const float smoothing = 1.0/16.0;\n"

		"void main() {\n"
			"float distance = texture2D(u_texture, v_tex_coord).a;\n"
			"float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance);\n"
			"gl_FragColor = vec4(v_color.rgb, alpha);\n"
		"}\n";


	ShaderId default_vs;
	ShaderId default_fs;
	ShaderId texture_fs;
	ShaderId font_vs;
	ShaderId font_fs;
	GPUProgramId texture_program;
	GPUProgramId def_program;
	GPUProgramId font_program;
	UniformId u_albedo_0;
	UniformId u_font;
	UniformId u_color;
	uint32_t num_refs = 0;

	void init()
	{
		if (num_refs)
			return;

		num_refs++;

		Renderer* r = device()->renderer();

		u_font = r->create_uniform("u_font", UniformType::INTEGER_1, 1);
		u_albedo_0 = r->create_uniform("u_albedo_0", UniformType::INTEGER_1, 1);
		u_color = r->create_uniform("u_color", UniformType::FLOAT_4, 1);

		default_vs = r->create_shader(ShaderType::VERTEX, default_vertex);
		default_fs = r->create_shader(ShaderType::FRAGMENT, default_fragment);
		texture_fs = r->create_shader(ShaderType::FRAGMENT, texture_fragment);
		font_vs = r->create_shader(ShaderType::VERTEX, sdf_vertex);
		font_fs = r->create_shader(ShaderType::FRAGMENT, sdf_fragment);

		def_program = r->create_gpu_program(default_vs, default_fs);
		texture_program = r->create_gpu_program(default_vs, texture_fs);
		font_program = r->create_gpu_program(font_vs, font_fs);
	}

	void shutdown()
	{
		num_refs--;

		if (num_refs)
			return;

		Renderer* r = device()->renderer();
		r->destroy_uniform(u_albedo_0);
		r->destroy_uniform(u_font);
		r->destroy_uniform(u_color);
		r->destroy_gpu_program(texture_program);
		r->destroy_gpu_program(def_program);
		r->destroy_gpu_program(font_program);
		r->destroy_shader(default_vs);
		r->destroy_shader(default_fs);
		r->destroy_shader(texture_fs);
		r->destroy_shader(font_vs);
		r->destroy_shader(font_fs);
	}

	GPUProgramId default_program()
	{
		return def_program;
	}

	GPUProgramId default_texture_program()
	{
		return texture_program;
	}

	GPUProgramId default_font_program()
	{
		return font_program;
	}

	UniformId default_albedo_uniform()
	{
		return u_albedo_0;
	}

	UniformId default_font_uniform()
	{
		return u_font;
	}

	UniformId default_color_uniform()
	{
		return u_color;
	}
};

//-----------------------------------------------------------------------------
RenderWorld::RenderWorld()
	: m_mesh_pool(default_allocator(), MAX_MESHES, sizeof(Mesh), CE_ALIGNOF(Mesh))
	, m_sprite_pool(default_allocator(), MAX_SPRITES, sizeof(Sprite), CE_ALIGNOF(Sprite))
	, m_material_pool(default_allocator(), MAX_MATERIALS, sizeof(Material), CE_ALIGNOF(Material))
	, m_gui_pool(default_allocator(), MAX_GUIS, sizeof(Gui), CE_ALIGNOF(Gui))
{
	render_world_globals::init();
}

//-----------------------------------------------------------------------------
RenderWorld::~RenderWorld()
{
	render_world_globals::shutdown();
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
GuiId RenderWorld::create_gui(uint16_t width, uint16_t height)
{
	Gui* gui = CE_NEW(m_gui_pool, Gui)(*this, width, height);
	GuiId id = m_guis.create(gui);
	gui->set_id(id);
	return id;
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
void RenderWorld::update(const Matrix4x4& view, const Matrix4x4& projection, uint16_t x, uint16_t y, uint16_t width, uint16_t height, float dt)
{
	Renderer* r = device()->renderer();

	Matrix4x4 inv_view = view;
	matrix4x4::invert(inv_view);

	r->set_layer_view(0, inv_view);
	r->set_layer_projection(0, projection);
	r->set_layer_viewport(0, x, y, width, height);
	r->set_layer_clear(0, CLEAR_COLOR | CLEAR_DEPTH, Color4(0x353839FF), 1.0f);

	r->set_state(STATE_DEPTH_WRITE | STATE_COLOR_WRITE | STATE_CULL_CCW);
	r->commit(0);

	// Draw all meshes
	for (uint32_t m = 0; m < m_mesh.size(); m++)
	{
		const Mesh* mesh = m_mesh.m_objects[m];

		r->set_state(STATE_DEPTH_WRITE | STATE_COLOR_WRITE | STATE_ALPHA_WRITE | STATE_CULL_CW);
		r->set_vertex_buffer(mesh->m_vbuffer);
		r->set_index_buffer(mesh->m_ibuffer);
		r->set_program(render_world_globals::default_program());
		// r->set_texture(0, render_world_globals::u_albedo_0, grass_texture, TEXTURE_FILTER_LINEAR | TEXTURE_WRAP_CLAMP_EDGE);
		// r->set_uniform(u_brightness, UNIFORM_FLOAT_1, &brightness, 1);

		r->set_pose(mesh->world_pose());
		r->commit(0);
	}

	// Draw all sprites
	for (uint32_t s = 0; s < m_sprite.size(); s++)
	{
		r->set_program(render_world_globals::default_texture_program());
		//m_sprite[s]->update(dt);
		m_sprite[s]->render(*r, render_world_globals::u_albedo_0, dt);
	}
}

} // namespace crown
