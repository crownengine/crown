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

namespace crown
{

static const char* default_vertex =
	"uniform mat4      	u_model;"
	"uniform mat4      	u_model_view_projection;"

	"attribute vec4    	a_position;"
	"attribute vec4    	a_normal;"
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
	"varying vec2            tex_coord0;"
	"varying vec4            color;"

	"uniform sampler2D  u_albedo_0;"

	"void main(void)"
	"{"
	"	gl_FragColor = color;"
	"}";

//-----------------------------------------------------------------------------
Gui::Gui(RenderWorld& render_world, GuiResource* gr, Renderer& r)
	: m_render_world(render_world)
	, m_resource(gr)
	, m_r(r)
{
	Vector2 size = m_resource->gui_size();
	// orthographic projection
	m_projection.build_projection_ortho_rh(0, size.x, size.y, 0, -0.01f, 100.0f);

	// pose
	Vector3 pos = m_resource->gui_position();
	m_pose.load_identity();
	m_pose.set_translation(pos);

	m_default_vs = m_r.create_shader(ShaderType::VERTEX, default_vertex);
	m_default_fs = m_r.create_shader(ShaderType::FRAGMENT, default_fragment);
	m_default_program = m_r.create_gpu_program(m_default_vs, m_default_fs);
}

//-----------------------------------------------------------------------------
Gui::~Gui()
{
	m_r.destroy_gpu_program(m_default_program);
	m_r.destroy_shader(m_default_fs);
	m_r.destroy_shader(m_default_vs);
}

//-----------------------------------------------------------------------------
void Gui::move(const Vector3& pos)
{
	m_pose.load_identity();
	m_pose.set_translation(pos);
}

//-----------------------------------------------------------------------------
void Gui::render(Renderer& r, UniformId uniform)
{
	Vector2 size = m_resource->gui_size();

	r.set_layer_view(1, Matrix4x4::IDENTITY);
	r.set_layer_projection(1, m_projection);
	r.set_layer_viewport(1, m_pose.translation().x, m_pose.translation().y, size.x, size.y);
	r.set_program(m_default_program);
	r.set_state(STATE_DEPTH_WRITE | STATE_COLOR_WRITE | STATE_ALPHA_WRITE | STATE_CULL_CW | STATE_PRIMITIVE_LINES);	
	r.set_pose(m_pose);

	// draw gui's rects
	r.set_vertex_buffer(m_resource->rect_vertex_buffer());
	r.set_index_buffer(m_resource->rect_index_buffer());
	r.commit(1);

	r.set_program(m_default_program);
	r.set_state(STATE_DEPTH_WRITE | STATE_COLOR_WRITE | STATE_ALPHA_WRITE | STATE_CULL_CW | STATE_PRIMITIVE_LINES);	
	r.set_pose(m_pose);

	// draw gui's triangles
	r.set_vertex_buffer(m_resource->triangle_vertex_buffer());
	r.set_index_buffer(m_resource->triangle_index_buffer());
	r.commit(1);
}

} // namespace crown