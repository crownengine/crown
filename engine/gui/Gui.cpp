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

#include "Gui.h"
#include "Assert.h"
#include "Renderer.h"
#include "Vector3.h"
#include "Vector2.h"
#include "GuiResource.h"
#include "RenderWorld.h"
#include "RendererTypes.h"
#include "Vector3.h"
#include "Vector2.h"
#include "Color4.h"

#include "GuiRect.h"
#include "GuiTriangle.h"


namespace crown
{

ShaderId			gui_default_vs;
ShaderId			gui_default_fs;
ShaderId			gui_texture_fs;
GPUProgramId		gui_default_program;
GPUProgramId		gui_texture_program;
UniformId			gui_albedo_0;

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
	"varying vec4            color;"

	"void main(void)"
	"{"
	"	gl_FragColor = color;"
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
Gui::Gui(RenderWorld& render_world, GuiResource* gr, Renderer& r)
	: m_render_world(render_world)
	, m_resource(gr)
	, m_r(r)
	, m_rect_pool(default_allocator(), MAX_GUI_RECTS, sizeof(GuiRect), CE_ALIGNOF(GuiRect))
	, m_triangle_pool(default_allocator(), MAX_GUI_TRIANGLES, sizeof(GuiTriangle), CE_ALIGNOF(GuiTriangle))
{
	// orthographic projection
	Vector2 size = m_resource->gui_size();
	m_projection.build_projection_ortho_rh(0, size.x, size.y, 0, -0.01f, 100.0f);

	// pose
	Vector3 pos = m_resource->gui_position();
	m_pose.load_identity();
	m_pose.set_translation(pos);

	// FIXME FIXME FIXME -- Shaders init should not be here
	gui_default_vs = m_r.create_shader(ShaderType::VERTEX, default_vertex);
	gui_default_fs = m_r.create_shader(ShaderType::FRAGMENT, default_fragment);
	gui_texture_fs = m_r.create_shader(ShaderType::FRAGMENT, texture_fragment);
	gui_default_program = m_r.create_gpu_program(gui_default_vs, gui_default_fs);
	gui_texture_program = m_r.create_gpu_program(gui_default_vs, gui_texture_fs);
	gui_albedo_0 = m_r.create_uniform("u_albedo_0", UniformType::INTEGER_1, 1);

	// Gui's rects creation
	for (uint32_t i = 0; i < m_resource->num_rects(); i++)
	{
		GuiRectData data = m_resource->get_rect(i);
		Vector3 pos(data.position[0], data.position[1], data.position[2]);
		Vector2 size(data.size[0], data.size[1]);
		Color4 color(data.color[0], data.color[1], data.color[2], data.color[3]);
	
		create_rect(pos, size, color);	
	}

	for (uint32_t i = 0; i < m_resource->num_triangles(); i++)
	{
		GuiTriangleData data = m_resource->get_triangle(i);
		Vector2 p1(data.points[0], data.points[1]);
		Vector2 p2(data.points[2], data.points[3]);
		Vector2 p3(data.points[4], data.points[5]);
		Color4 color(data.color[0], data.color[1], data.color[2], data.color[3]);

		create_triangle(p1, p2, p3, color);
	}
}

//-----------------------------------------------------------------------------
Gui::~Gui()
{
	for (uint32_t i = 0; i < m_resource->num_rects(); i++)
	{
		CE_DELETE(m_rect_pool, m_rects[i]);
	}

	for (uint32_t i = 0; i < m_resource->num_triangles(); i++)
	{
		CE_DELETE(m_triangle_pool, m_triangles[i]);
	}

	m_r.destroy_uniform(gui_albedo_0);
	m_r.destroy_gpu_program(gui_texture_program);
	m_r.destroy_gpu_program(gui_default_program);
	m_r.destroy_shader(gui_texture_fs);
	m_r.destroy_shader(gui_default_fs);
	m_r.destroy_shader(gui_default_vs);
}

//-----------------------------------------------------------------------------
void Gui::move(const Vector3& pos)
{
	m_pose.load_identity();
	m_pose.set_translation(pos);
}

//-----------------------------------------------------------------------------
GuiRectId Gui::create_rect(const Vector3& pos, const Vector2& size, const Color4& color)
{
	GuiRect* rect = CE_NEW(m_rect_pool, GuiRect)(m_r, pos, size, color);
	return m_rects.create(rect);
}

//-----------------------------------------------------------------------------
void Gui::update_rect(GuiRectId id, const Vector3& pos, const Vector2& size, const Color4& color)
{
	CE_ASSERT(m_rects.has(id), "GuiRect does not exists");

	GuiRect* rect = m_rects.lookup(id);
	rect->update(pos, size, color);
}

//-----------------------------------------------------------------------------
void Gui::destroy_rect(GuiRectId id)
{
	CE_ASSERT(m_rects.has(id), "GuiRect does not exist");

	GuiRect* rect = m_rects.lookup(id);
	CE_DELETE(m_rect_pool, rect);
	m_rects.destroy(id);
}

//-----------------------------------------------------------------------------
GuiTriangleId Gui::create_triangle(const Vector2& p1, const Vector2& p2, const Vector2& p3, const Color4& color)
{
	GuiTriangle* triangle = CE_NEW(m_triangle_pool, GuiTriangle)(m_r, p1, p2, p3, color);
	return m_triangles.create(triangle);
}

//-----------------------------------------------------------------------------
void Gui::update_triangle(GuiTriangleId id, const Vector2& p1, const Vector2& p2, const Vector2& p3, const Color4& color)
{
	CE_ASSERT(m_triangles.has(id), "GuiTriangle does not exists");

	GuiTriangle* triangle = m_triangles.lookup(id);
	triangle->update(p1, p2, p3, color);
}

//-----------------------------------------------------------------------------
void Gui::destroy_triangle(GuiTriangleId id)
{
	CE_ASSERT(m_triangles.has(id), "Guitriangle does not exist");

	GuiTriangle* triangle = m_triangles.lookup(id);
	CE_DELETE(m_triangle_pool, triangle);
	m_triangles.destroy(id);
}

//-----------------------------------------------------------------------------
void Gui::render()
{
	Vector2 size = m_resource->gui_size();

	m_r.set_layer_view(1, Matrix4x4::IDENTITY);
	m_r.set_layer_projection(1, m_projection);
	m_r.set_layer_viewport(1, m_pose.translation().x, m_pose.translation().y, size.x, size.y);

	// Render all Rects
	for (uint32_t i = 0; i < m_rects.size(); i++)
	{
		m_r.set_program(gui_default_program);
		m_r.set_state(STATE_DEPTH_WRITE 
		| STATE_COLOR_WRITE
		| STATE_ALPHA_WRITE
		| STATE_CULL_CW
		| STATE_PRIMITIVE_LINES
		| STATE_BLEND_EQUATION_ADD 
		| STATE_BLEND_FUNC(STATE_BLEND_FUNC_SRC_ALPHA, STATE_BLEND_FUNC_ONE_MINUS_SRC_ALPHA));
		m_r.set_pose(m_pose);

		m_rects[i]->render();
	}

	// Render all Triangles
	for (uint32_t i = 0; i < m_triangles.size(); i++)
	{
		m_r.set_program(gui_default_program);
		m_r.set_state(STATE_DEPTH_WRITE 
		| STATE_COLOR_WRITE
		| STATE_ALPHA_WRITE
		| STATE_CULL_CW
		| STATE_PRIMITIVE_LINES
		| STATE_BLEND_EQUATION_ADD 
		| STATE_BLEND_FUNC(STATE_BLEND_FUNC_SRC_ALPHA, STATE_BLEND_FUNC_ONE_MINUS_SRC_ALPHA));
		m_r.set_pose(m_pose);

		m_triangles[i]->render();
	}
}

} // namespace crown