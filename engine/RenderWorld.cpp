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
#include "Resource.h"
#include "Log.h"
#include "SpriteResource.h"
#include "SpriteAnimator.h"

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
	, m_sprite(default_allocator())
{
	Renderer* r = device()->renderer();

	default_vs = r->create_shader(ShaderType::VERTEX, default_vertex);
	default_fs = r->create_shader(ShaderType::FRAGMENT, default_fragment);
	texture_fs = r->create_shader(ShaderType::FRAGMENT, texture_fragment);

	u_albedo_0 = r->create_uniform("u_albedo_0", UniformType::INTEGER_1, 1);
	u_lightmap_0 = r->create_uniform("u_lightmap_0", UniformType::INTEGER_1, 1);
	u_brightness = r->create_uniform("u_brightness", UniformType::FLOAT_1, 1);

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
	r->destroy_uniform(u_lightmap_0);
	r->destroy_uniform(u_brightness);
}

//-----------------------------------------------------------------------------
MeshId RenderWorld::create_mesh(const char* name, int32_t node, const Vector3& pos, const Quaternion& rot)
{
	MeshResource* mr = (MeshResource*) device()->resource_manager()->lookup(MESH_EXTENSION, name);

	MeshId mesh = allocate_mesh(mr, node, pos, rot);

	return mesh;
}

//-----------------------------------------------------------------------------
void RenderWorld::destroy_mesh(MeshId /*id*/)
{
}

//-----------------------------------------------------------------------------
Mesh* RenderWorld::lookup_mesh(MeshId mesh)
{
	CE_ASSERT(m_mesh_table.has(mesh), "Mesh does not exits");

	return &m_mesh[m_sparse_to_packed[mesh.index]];
}

//-----------------------------------------------------------------------------
SpriteId RenderWorld::create_sprite(const char* name, int32_t node, const Vector3& pos, const Quaternion& rot)
{
	SpriteResource* sr = (SpriteResource*) device()->resource_manager()->lookup(SPRITE_EXTENSION, name);

	SpriteId sprite = allocate_sprite(sr, node, pos, rot);

	return sprite;
}

//-----------------------------------------------------------------------------
void RenderWorld::destroy_sprite(SpriteId /*id*/)
{
	// Stub
}

//-----------------------------------------------------------------------------
Sprite*	RenderWorld::lookup_sprite(SpriteId id)
{
	return &m_sprite[m_sprite_sparse_to_packed[id.index]];
}

//-----------------------------------------------------------------------------
void RenderWorld::update(Camera& camera, float /*dt*/)
{
	static uint64_t frames = 0;

	Renderer* r = device()->renderer();

	Matrix4x4 camera_view = camera.world_pose();
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
		/*r->set_texture(0, u_albedo_0, grass_texture, TEXTURE_FILTER_LINEAR | TEXTURE_WRAP_CLAMP_EDGE);
		r->set_uniform(u_brightness, UNIFORM_FLOAT_1, &brightness, 1);*/

		r->set_pose(mesh.m_local_pose);
		r->commit(0);
	}

	for (uint32_t s = 0; s < m_sprite.size(); s++)
	{
		Sprite& sprite = m_sprite[s];

		if (frames % sprite.m_animator->m_frame_rate == 0)
		{
			sprite.m_animator->play_frame();
		}

		r->set_state(STATE_DEPTH_WRITE | STATE_COLOR_WRITE | STATE_ALPHA_WRITE | STATE_CULL_CW);
		r->set_vertex_buffer(sprite.m_vb);
		r->set_index_buffer(sprite.m_ib);
		r->set_program(sprite.m_program);
		r->set_texture(0, sprite.m_uniform, sprite.m_texture, TEXTURE_FILTER_LINEAR | TEXTURE_WRAP_CLAMP_EDGE);

		r->set_pose(sprite.m_local_pose);
		r->commit(0);
	}

	frames++;
}

//-----------------------------------------------------------------------------
MeshId RenderWorld::allocate_mesh(MeshResource* mr, int32_t node, const Vector3& pos, const Quaternion& rot)
{
	MeshId mesh_id = m_mesh_table.create();

	Mesh mesh;
	mesh.create(mr, node, pos, rot);

	uint32_t index = m_mesh.push_back(mesh);
	m_sparse_to_packed[mesh_id.index] = index;

	return mesh_id;
}

//-----------------------------------------------------------------------------
void RenderWorld::deallocate_mesh(MeshId /*id*/)
{
}

//-----------------------------------------------------------------------------
SpriteId RenderWorld::allocate_sprite(SpriteResource* sr, int32_t node, const Vector3& pos, const Quaternion& rot)
{
	SpriteId id = m_sprite_table.create();

	Sprite sprite;
	sprite.create(sr, node, pos, rot);

	uint32_t index = m_sprite.push_back(sprite);
	m_sprite_sparse_to_packed[id.index] = index;

	return id;
}

//-----------------------------------------------------------------------------
void RenderWorld::deallocate_sprite(SpriteId /*id*/)
{
	// Stub
}

} // namespace crown
