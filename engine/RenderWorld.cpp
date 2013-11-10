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
	"uniform mat4      	u_model;"
	"uniform mat4      	u_model_view_projection;"

	"in vec4           	a_position;"
	"in vec4           	a_normal;"
	"in vec2           	a_tex_coord0;"
	"in vec4           	a_color;"

	"varying out vec2	tex_coord0;"
	"varying out vec4	color;"

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
	"in vec2            tex_coord0;"
	"in vec4            color;"

	"uniform sampler2D  u_albedo_0;"

	"void main(void)"
	"{"
	"	gl_FragColor = texture(u_albedo_0, tex_coord0);"
	"}";

//-----------------------------------------------------------------------------
RenderWorld::RenderWorld()
	: m_mesh_pool(default_allocator(), MAX_MESHES, sizeof(Mesh))
	, m_mesh(default_allocator())
	, m_sprite_pool(default_allocator(), MAX_SPRITES, sizeof(Sprite))
	, m_sprite(default_allocator())
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
MeshId RenderWorld::create_mesh(ResourceId id, int32_t node, const Vector3& pos, const Quaternion& rot)
{
	MeshResource* mr = (MeshResource*) device()->resource_manager()->data(id);

	// Allocate memory for mesh
	Mesh* mesh = (Mesh*) m_mesh_pool.allocate(sizeof(Mesh));

	// Create mesh id
	const MeshId mesh_id = m_mesh.create(mesh);
	mesh->create(mr, node, pos, rot);

	return mesh_id;
}

//-----------------------------------------------------------------------------
void RenderWorld::destroy_mesh(MeshId /*id*/)
{
}

//-----------------------------------------------------------------------------
Mesh* RenderWorld::lookup_mesh(MeshId mesh)
{
	CE_ASSERT(m_mesh.has(mesh), "Mesh does not exits");

	return m_mesh.lookup(mesh);
}

//-----------------------------------------------------------------------------
SpriteId RenderWorld::create_sprite(ResourceId id, int32_t node, const Vector3& pos, const Quaternion& rot)
{
	SpriteResource* sr = (SpriteResource*) device()->resource_manager()->data(id);

	// Allocate memory for sprite
	Sprite* sprite = (Sprite*) m_sprite_pool.allocate(sizeof(Sprite));

	// Create sprite id
	const SpriteId sprite_id = m_sprite.create(sprite);
	sprite->create(sr, node, pos, rot);

	return sprite_id;
}

//-----------------------------------------------------------------------------
void RenderWorld::destroy_sprite(SpriteId /*id*/)
{
	// Stub
}

//-----------------------------------------------------------------------------
Sprite*	RenderWorld::lookup_sprite(SpriteId id)
{
	CE_ASSERT(m_sprite.has(id), "Sprite does not exist");

	return m_sprite.lookup(id);
}

//-----------------------------------------------------------------------------
void RenderWorld::update(const Matrix4x4& view, const Matrix4x4& projection, uint16_t x, uint16_t y, uint16_t width, uint16_t height, float /*dt*/)
{
	static uint64_t frames = 0;

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
	const List<Mesh*>& meshes = m_mesh.m_objects;

	for (uint32_t m = 0; m < meshes.size(); m++)
	{
		const Mesh* mesh = meshes[m];

		r->set_state(STATE_DEPTH_WRITE | STATE_COLOR_WRITE | STATE_ALPHA_WRITE | STATE_CULL_CW);
		r->set_vertex_buffer(mesh->m_vbuffer);
		r->set_index_buffer(mesh->m_ibuffer);
		r->set_program(default_program);
		/*r->set_texture(0, u_albedo_0, grass_texture, TEXTURE_FILTER_LINEAR | TEXTURE_WRAP_CLAMP_EDGE);
		r->set_uniform(u_brightness, UNIFORM_FLOAT_1, &brightness, 1);*/

		r->set_pose(mesh->m_local_pose);
		r->commit(0);
	}

	const List<Sprite*>& sprites = m_sprite.m_objects;
	for (uint32_t s = 0; s < sprites.size(); s++)
	{
		Sprite* sprite = sprites[s];

		if (frames % sprite->m_animator->m_frame_rate == 0)
		{
			sprite->m_animator->play_frame();
		}

		r->set_state(STATE_DEPTH_WRITE 
			| STATE_COLOR_WRITE 
			| STATE_ALPHA_WRITE 
			| STATE_CULL_CW 
			| STATE_BLEND_EQUATION_ADD 
			| STATE_BLEND_FUNC(STATE_BLEND_FUNC_SRC_ALPHA, STATE_BLEND_FUNC_ONE_MINUS_SRC_ALPHA));
		r->set_vertex_buffer(sprite->m_vb);
		r->set_index_buffer(sprite->m_ib);
		r->set_program(texture_program);
		r->set_texture(0, u_albedo_0, sprite->m_texture, TEXTURE_FILTER_LINEAR | TEXTURE_WRAP_CLAMP_EDGE);

		r->set_pose(sprite->m_world_pose);
		r->commit(0);
	}

	frames++;
}

} // namespace crown
