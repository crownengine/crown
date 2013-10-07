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
#include "ResourceManager.h"
#include "Renderer.h"
#include "Allocator.h"
#include "Camera.h"

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
	"in vec4           a_position;"
	"in vec4           a_normal;"
	"in vec2           a_tex_coord0;"
	"in vec4           a_color;"

	"uniform mat4      u_model;"
	"uniform mat4      u_model_view_projection;"

	"varying out vec2  tex_coord0;"
	"varying out vec4  color;"

	"void main(void)"
	"{"
	"	tex_coord0 = a_tex_coord0;"
	"   color = a_color;"
	"	gl_Position = u_model_view_projection * a_position;"
	"	gl_FrontColor = vec4(vec3(1, 0, 0), 1.0);"
	"}";

static const char* default_fragment = 
	"void main(void)"
	"{"
	"	gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);"
	"}";

static const char* texture_fragment = 
	"in vec2            tex_coord0;"
	"in vec4            color;"

	"uniform sampler2D  u_albedo_0;"
	"uniform sampler2D  u_lightmap_0;"
	"uniform float      u_brightness;"

	"void main(void)"
	"{"
	"	gl_FragColor = texture(u_albedo_0, tex_coord0) * texture(u_lightmap_0, tex_coord0) * color * u_brightness;"
	"}";

//-----------------------------------------------------------------------------
RenderWorld::RenderWorld()
	: m_mesh(default_allocator())
{
	Renderer* r = device()->renderer();

	default_vs = r->create_shader(SHADER_VERTEX, default_vertex);
	default_fs = r->create_shader(SHADER_FRAGMENT, default_fragment);
	texture_fs = r->create_shader(SHADER_FRAGMENT, texture_fragment);

	u_albedo_0 = r->create_uniform("u_albedo_0", UNIFORM_INTEGER_1, 1);
	u_lightmap_0 = r->create_uniform("u_lightmap_0", UNIFORM_INTEGER_1, 1);
	u_brightness = r->create_uniform("u_brightness", UNIFORM_FLOAT_1, 1);

	default_program = r->create_gpu_program(default_vs, default_fs);
	texture_program = r->create_gpu_program(default_vs, texture_fs);

	create_mesh("monkey");
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
	r->destroy_uniform(u_lightmap_0);
	r->destroy_uniform(u_brightness);
}

//-----------------------------------------------------------------------------
MeshId RenderWorld::create_mesh(const char* name)
{
	MeshResource* mr = (MeshResource*) device()->resource_manager()->lookup("mesh", name);

	MeshId mesh = allocate_mesh();
	m_mesh[mesh.index].create(mr);

	return mesh;
}

//-----------------------------------------------------------------------------
void RenderWorld::destroy_mesh(MeshId /*id*/)
{
}

//-----------------------------------------------------------------------------
void RenderWorld::update(Camera& camera, float /*dt*/)
{
	Renderer* r = device()->renderer();

	Mat4 camera_view = camera.local_pose();
	camera_view.invert();

	r->set_layer_view(0, camera_view);
	r->set_layer_projection(0, camera.m_projection);
	r->set_layer_viewport(0, 0, 0, 1000, 625);
	r->set_layer_clear(0, CLEAR_COLOR | CLEAR_DEPTH, Color4::LIGHTBLUE, 1.0f);

	r->set_state(STATE_DEPTH_WRITE | STATE_COLOR_WRITE | STATE_CULL_CCW);
	r->commit(0);

	// Draw all meshes
	for (uint32_t m = 0; m < m_mesh.size(); m++)
	{
		const Mesh& mesh = m_mesh[m];

		r->set_state(STATE_DEPTH_WRITE | STATE_COLOR_WRITE | STATE_ALPHA_WRITE | STATE_CULL_CW);
		r->set_vertex_buffer(mesh.m_vbuffer);
		r->set_index_buffer(mesh.m_ibuffer);
		r->set_program(default_program);
/*		r->set_texture(0, u_albedo_0, grass_texture, TEXTURE_FILTER_LINEAR | TEXTURE_WRAP_CLAMP_EDGE);
		r->set_uniform(u_brightness, UNIFORM_FLOAT_1, &brightness, 1);*/

		r->set_pose(mesh.m_local_pose);
		r->commit(0);
	}
}

//-----------------------------------------------------------------------------
MeshId RenderWorld::allocate_mesh()
{
	MeshId mesh = m_mesh_table.create();
	
	if (m_mesh.size() <= mesh.index)
	{
		m_mesh.resize(1);
	}

	return mesh;
}

//-----------------------------------------------------------------------------
void RenderWorld::deallocate_mesh(MeshId /*id*/)
{
}

} // namespace crown
