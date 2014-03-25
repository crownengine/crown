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
#include "FontResource.h"
#include "Device.h"
#include "OsWindow.h"

#include "GuiRect.h"
#include "GuiTriangle.h"
#include "GuiImage.h"
#include "GuiText.h"

namespace crown
{

ShaderId			gui_default_vs;
ShaderId			gui_default_fs;
ShaderId			gui_texture_fs;
GPUProgramId		gui_default_program;
GPUProgramId		gui_texture_program;
UniformId			gui_albedo_0;

ShaderId			font_vs;
ShaderId			font_fs;
GPUProgramId		font_program;
UniformId			font_uniform;

static const char* default_vertex =
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

static const char* sdf_vertex =
	"uniform mat4      	u_model_view_projection;"

	"attribute vec4		a_position;"
	"attribute vec2		a_tex_coord0;"

	"varying vec2		v_tex_coord;"
	"varying vec4		v_color;"

	"void main(void)"
	"{"
	"	gl_Position = u_model_view_projection * a_position;"
	"	v_tex_coord = a_tex_coord0;"
	"	v_color = vec4(1, 1, 1, 1);"
	"}";

static const char* sdf_fragment =
	"uniform sampler2D u_texture;"

	"varying vec4 v_color;"
	"varying vec2 v_tex_coord;"

	"const float smoothing = 1.0/16.0;"

	"void main() {"
		"float distance = texture2D(u_texture, v_tex_coord).a;"
		"float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance);"
		"gl_FragColor = vec4(v_color.rgb, alpha);"
	"}";

//-----------------------------------------------------------------------------
Gui::Gui(RenderWorld& render_world, GuiResource* gr, Renderer& r)
	: m_render_world(render_world)
	, m_resource(gr)
	, m_r(r)
	, m_resolution(1000, 625)
	, m_visible(true)
	, m_rect_pool(default_allocator(), CE_MAX_GUI_RECTS, sizeof(GuiRect), CE_ALIGNOF(GuiRect))
	, m_triangle_pool(default_allocator(), CE_MAX_GUI_TRIANGLES, sizeof(GuiTriangle), CE_ALIGNOF(GuiTriangle))
	, m_image_pool(default_allocator(), CE_MAX_GUI_IMAGES, sizeof(GuiImage), CE_ALIGNOF(GuiImage))
	, m_text_pool(default_allocator(), CE_MAX_GUI_TEXTS, sizeof(GuiText), CE_ALIGNOF(GuiText))
{
	// orthographic projection
	m_projection.build_projection_ortho_rh(0, m_resolution.x, m_resolution.y, 0, -0.01f, 100.0f);

	// pose
	Vector3 pos = m_resource->gui_position();
	m_pose.load_identity();
	m_pose.set_translation(pos);

	create_gfx();

	// Gui's rects creation
	for (uint32_t i = 0; i < m_resource->num_rects(); i++)
	{
		GuiRectData data = m_resource->get_rect(i);
		Vector3 pos(data.position[0], data.position[1], data.position[2]);
		Vector2 size(data.size[0], data.size[1]);
		Color4 color(data.color[0], data.color[1], data.color[2], data.color[3]);
	
		create_rect(pos, size, color);	
	}

	// Gui's triangles creation
	for (uint32_t i = 0; i < m_resource->num_triangles(); i++)
	{
		GuiTriangleData data = m_resource->get_triangle(i);
		Vector2 p1(data.points[0], data.points[1]);
		Vector2 p2(data.points[2], data.points[3]);
		Vector2 p3(data.points[4], data.points[5]);
		Color4 color(data.color[0], data.color[1], data.color[2], data.color[3]);

		create_triangle(p1, p2, p3, color);
	}

	// Gui's images creation
	for (uint32_t i = 0; i < m_resource->num_images(); i++)
	{
		GuiImageData data = m_resource->get_image(i);
		ResourceId mat = data.material;
		Vector3 pos(data.position[0], data.position[1], 0);
		Vector2 size(data.size[0], data.size[1]);

		create_image(mat, pos, size);
	}
}

//-----------------------------------------------------------------------------
Gui::~Gui()
{
	for (uint32_t i = 0; i < m_rects.size(); i++)
	{
		CE_DELETE(m_rect_pool, m_rects[i]);
	}

	for (uint32_t i = 0; i < m_triangles.size(); i++)
	{
		CE_DELETE(m_triangle_pool, m_triangles[i]);
	}

	for (uint32_t i = 0; i < m_images.size(); i++)
	{
		CE_DELETE(m_image_pool, m_images[i]);
	}

	for (uint32_t i = 0; i < m_texts.size(); i++)
	{
		CE_DELETE(m_text_pool, m_texts[i]);
	}

	destroy_gfx();
}

//-----------------------------------------------------------------------------
const GuiId Gui::id() const
{
	return m_id;
}

//-----------------------------------------------------------------------------
void Gui::set_id(const GuiId id)
{
	m_id = id;
}

//-----------------------------------------------------------------------------
Vector2 Gui::resolution() const
{
	return m_resolution;
}

//-----------------------------------------------------------------------------
void Gui::move(const Vector3& pos)
{
	m_pose.load_identity();
	m_pose.set_translation(pos);
}

//-----------------------------------------------------------------------------
void Gui::show()
{
	m_visible = true;
}

//-----------------------------------------------------------------------------
void Gui::hide()
{
	m_visible = false;
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
	CE_ASSERT(m_triangles.has(id), "GuiTriangle does not exist");

	GuiTriangle* triangle = m_triangles.lookup(id);
	CE_DELETE(m_triangle_pool, triangle);
	m_triangles.destroy(id);
}

//-----------------------------------------------------------------------------
GuiImageId Gui::create_image(ResourceId material, const Vector3& pos, const Vector2& size)
{
	GuiImage* image = CE_NEW(m_image_pool, GuiImage)(m_render_world, m_r, material, pos, size);
	return m_images.create(image);
}

//-----------------------------------------------------------------------------
void Gui::update_image(GuiImageId id, const Vector3& pos, const Vector2& size)
{
	CE_ASSERT(m_images.has(id), "GuiImage does not exists");

	GuiImage* image = m_images.lookup(id);
	image->update(pos, size);
}

//-----------------------------------------------------------------------------
void Gui::destroy_image(GuiImageId id)
{
	CE_ASSERT(m_images.has(id), "GuiImage does not exists");

	GuiImage* image = m_images.lookup(id);
	CE_DELETE(m_image_pool, image);
	m_images.destroy(id);
}

//-----------------------------------------------------------------------------
GuiTextId Gui::create_text(const char* str, const FontResource* font, uint32_t font_size, const Vector3& pos)
{
	GuiText* text = CE_NEW(m_text_pool, GuiText)(m_render_world, m_r, str, font, font_size, pos);
	return m_texts.create(text);
}

//-----------------------------------------------------------------------------
void Gui::update_text(GuiTextId id, const char* str, uint32_t font_size, const Vector3& pos)
{
	CE_ASSERT(m_texts.has(id), "GuiText does not exists");

	GuiText* text = m_texts.lookup(id);
	text->update(str, font_size, pos);
}

//-----------------------------------------------------------------------------
void Gui::destroy_text(GuiTextId id)
{
	CE_ASSERT(m_texts.has(id), "GuiText does not exists");

	GuiText* text = m_texts.lookup(id);
	CE_DELETE(m_text_pool, text);
	m_texts.destroy(id);
}

//-----------------------------------------------------------------------------
void Gui::render()
{
	// resolution
	uint32_t width = 0;
	uint32_t height = 0;
	device()->window()->get_size(width, height);
	m_resolution.x = width;
	m_resolution.y = height;

	m_r.set_layer_view(1, Matrix4x4::IDENTITY);
	m_r.set_layer_projection(1, m_projection);
	m_r.set_layer_viewport(1, m_pose.translation().x, m_pose.translation().y, m_resolution.x, m_resolution.y);

	if (!m_visible) return;

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

	// Render all Images
	for (uint32_t i = 0; i < m_images.size(); i++)
	{
		m_r.set_program(gui_texture_program);
		m_r.set_state(STATE_DEPTH_WRITE 
		| STATE_COLOR_WRITE 
		| STATE_ALPHA_WRITE 
		| STATE_CULL_CW
		| STATE_PRIMITIVE_TRIANGLES
		| STATE_BLEND_EQUATION_ADD 
		| STATE_BLEND_FUNC(STATE_BLEND_FUNC_SRC_ALPHA, STATE_BLEND_FUNC_ONE_MINUS_SRC_ALPHA));
		m_r.set_pose(m_pose);

		m_images[i]->render(gui_albedo_0);
	}

	// Render all Texts
	for (uint32_t i = 0; i < m_texts.size(); i++)
	{
		m_r.set_program(font_program);
		m_r.set_state(STATE_DEPTH_WRITE 
		| STATE_COLOR_WRITE 
		| STATE_ALPHA_WRITE 
		| STATE_CULL_CW
		| STATE_PRIMITIVE_TRIANGLES
		| STATE_BLEND_EQUATION_ADD 
		| STATE_BLEND_FUNC(STATE_BLEND_FUNC_SRC_ALPHA, STATE_BLEND_FUNC_ONE_MINUS_SRC_ALPHA));
		m_r.set_pose(m_pose);

		m_texts[i]->render(font_uniform);
	}
}

//-----------------------------------------------------------------------------
void Gui::create_gfx()
{
	gui_default_vs = m_r.create_shader(ShaderType::VERTEX, default_vertex);
	gui_default_fs = m_r.create_shader(ShaderType::FRAGMENT, default_fragment);
	gui_texture_fs = m_r.create_shader(ShaderType::FRAGMENT, texture_fragment);
	gui_default_program = m_r.create_gpu_program(gui_default_vs, gui_default_fs);
	gui_texture_program = m_r.create_gpu_program(gui_default_vs, gui_texture_fs);
	gui_albedo_0 = m_r.create_uniform("u_albedo_0", UniformType::INTEGER_1, 1);

	font_vs = m_r.create_shader(ShaderType::VERTEX, sdf_vertex);
	font_fs = m_r.create_shader(ShaderType::FRAGMENT, sdf_fragment);
	font_program = m_r.create_gpu_program(font_vs, font_fs);
	font_uniform = m_r.create_uniform("u_texture", UniformType::INTEGER_1, 1);
}

//-----------------------------------------------------------------------------
void Gui::destroy_gfx()
{
	m_r.destroy_uniform(gui_albedo_0);
	m_r.destroy_gpu_program(gui_texture_program);
	m_r.destroy_gpu_program(gui_default_program);
	m_r.destroy_shader(gui_texture_fs);
	m_r.destroy_shader(gui_default_fs);
	m_r.destroy_shader(gui_default_vs);

	m_r.destroy_uniform(font_uniform);
	m_r.destroy_gpu_program(font_program);
	m_r.destroy_shader(font_vs);
	m_r.destroy_shader(font_fs);
}

} // namespace crown