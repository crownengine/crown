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

#include <GL/glew.h>
#include <algorithm>

#include "Config.h"

#include "Allocator.h"
#include "Assert.h"
#include "Types.h"
#include "GLRenderer.h"
#include "Log.h"
#include "Vec2.h"
#include "Vec3.h"
#include "Vec4.h"
#include "Mat3.h"
#include "Mat4.h"
#include "Device.h"
#include "Hash.h"
#include "StringUtils.h"

namespace crown
{

//-----------------------------------------------------------------------------
const GLenum TEXTURE_MIN_FILTER_TABLE[] =
{
	0, // Unused
	GL_NEAREST,
	GL_LINEAR,
	GL_NEAREST_MIPMAP_LINEAR,
	GL_LINEAR_MIPMAP_LINEAR
};

//-----------------------------------------------------------------------------
const GLenum TEXTURE_MAG_FILTER_TABLE[] =
{
	0, // Unused
	GL_NEAREST,
	GL_LINEAR,
	GL_LINEAR,
	GL_LINEAR
};

//-----------------------------------------------------------------------------
const GLenum TEXTURE_WRAP_TABLE[] =
{
	0, // Unused
	GL_CLAMP,
	GL_CLAMP_TO_EDGE,
	GL_CLAMP_TO_BORDER,
	GL_REPEAT
};

//-----------------------------------------------------------------------------
GLRenderer::GLRenderer() :
	m_max_texture_size(0),
	m_max_texture_units(0),
	m_max_vertex_indices(0),
	m_max_vertex_vertices(0),
	m_max_anisotropy(0.0f),

	m_textures_id_table(m_allocator, CROWN_MAX_TEXTURES),
	m_active_texture_unit(0),

	m_vertex_buffers_id_table(m_allocator, CROWN_MAX_VERTEX_BUFFERS),
	m_index_buffers_id_table(m_allocator, CROWN_MAX_INDEX_BUFFERS),
	m_shaders_id_table(m_allocator, CROWN_MAX_SHADERS),
	m_gpu_programs_id_table(m_allocator, CROWN_MAX_GPU_PROGRAMS),
	m_uniforms_id_table(m_allocator, CROWN_MAX_UNIFORMS),
	m_render_targets_id_table(m_allocator, CROWN_MAX_RENDER_TARGETS)
{
	m_min_max_point_size[0] = 0.0f;
	m_min_max_point_size[1] = 0.0f;
	m_min_max_line_width[0] = 0.0f;
	m_min_max_line_width[1] = 0.0f;

	// Initialize texture units
	for (uint32_t i = 0; i < CROWN_MAX_TEXTURE_UNITS; i++)
	{
		m_texture_unit[i] = 0;
		m_texture_unit_target[i] = GL_TEXTURE_2D;
	}
}

//-----------------------------------------------------------------------------
GLRenderer::~GLRenderer()
{
}

//-----------------------------------------------------------------------------
void GLRenderer::init()
{
	m_context.create_context();

	GLenum err = glewInit();
	CE_ASSERT(err == GLEW_OK, "Failed to initialize GLEW");

	GL_CHECK(glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_max_texture_size));
	GL_CHECK(glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &m_max_texture_units));
	GL_CHECK(glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &m_max_vertex_indices));
	GL_CHECK(glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &m_max_vertex_vertices));

	GL_CHECK(glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, &m_min_max_point_size[0]));
	GL_CHECK(glGetFloatv(GL_LINE_WIDTH_RANGE, &m_min_max_line_width[0]));

	Log::i("OpenGL Vendor        : %s", glGetString(GL_VENDOR));
	Log::i("OpenGL Renderer      : %s", glGetString(GL_RENDERER));
	Log::i("OpenGL Version       : %s", glGetString(GL_VERSION));
	Log::i("GLSL Version         : %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

	Log::d("Min Point Size       : %f", m_min_max_point_size[0]);
	Log::d("Max Point Size       : %f", m_min_max_point_size[1]);
	Log::d("Min Line Width       : %f", m_min_max_line_width[0]);
	Log::d("Max Line Width       : %f", m_min_max_line_width[1]);
	Log::d("Max Texture Size     : %dx%d", m_max_texture_size, m_max_texture_size);
	Log::d("Max Texture Units    : %d", m_max_texture_units);
	Log::d("Max Vertex Indices   : %d", m_max_vertex_indices);
	Log::d("Max Vertex Vertices  : %d", m_max_vertex_vertices);

	GL_CHECK(glDisable(GL_BLEND));
	GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	GL_CHECK(glBlendEquation(GL_FUNC_ADD));

	GL_CHECK(glShadeModel(GL_SMOOTH));

	// Enable depth test
	GL_CHECK(glEnable(GL_DEPTH_TEST));
	GL_CHECK(glDepthFunc(GL_LEQUAL));
	GL_CHECK(glClearDepth(1.0));

	// Point sprites enabled by default
	GL_CHECK(glEnable(GL_POINT_SPRITE));
	GL_CHECK(glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE));

	Log::i("OpenGL Renderer initialized.");
}

//-----------------------------------------------------------------------------
void GLRenderer::shutdown()
{
	m_context.destroy_context();
}

//-----------------------------------------------------------------------------
VertexBufferId GLRenderer::create_vertex_buffer(size_t count, VertexFormat format, const void* vertices)
{
	const VertexBufferId id = m_vertex_buffers_id_table.create();

	VertexBuffer& buffer = m_vertex_buffers[id.index];
	buffer.create(count, format, vertices);

	return id;
}

//-----------------------------------------------------------------------------
void GLRenderer::update_vertex_buffer(VertexBufferId id, size_t offset, size_t count, const void* vertices)
{
	CE_ASSERT(m_vertex_buffers_id_table.has(id), "Vertex buffer does not exist");

	VertexBuffer& buffer = m_vertex_buffers[id.index];
	buffer.update(offset, count, vertices);
}

//-----------------------------------------------------------------------------
void GLRenderer::destroy_vertex_buffer(VertexBufferId id)
{
	CE_ASSERT(m_vertex_buffers_id_table.has(id), "Vertex buffer does not exist");

	VertexBuffer& buffer = m_vertex_buffers[id.index];
	buffer.destroy();

	m_vertex_buffers_id_table.destroy(id);
}

//-----------------------------------------------------------------------------
IndexBufferId GLRenderer::create_index_buffer(size_t count, const void* indices)
{
	const IndexBufferId id = m_index_buffers_id_table.create();

	IndexBuffer& buffer = m_index_buffers[id.index];
	buffer.create(count, indices);

	return id;
}

//-----------------------------------------------------------------------------
void GLRenderer::destroy_index_buffer(IndexBufferId id)
{
	CE_ASSERT(m_index_buffers_id_table.has(id), "Index buffer does not exist");

	IndexBuffer& buffer = m_index_buffers[id.index];
	buffer.destroy();

	m_index_buffers_id_table.destroy(id);
}

//-----------------------------------------------------------------------------
TextureId GLRenderer::create_texture(uint32_t width, uint32_t height, PixelFormat format, const void* data)
{
	const TextureId id = m_textures_id_table.create();
	Texture& texture = m_textures[id.index];

	texture.create(width, height, format, data);

	return id;
}

//-----------------------------------------------------------------------------
void GLRenderer::update_texture(TextureId id, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void* data)
{
	CE_ASSERT(m_textures_id_table.has(id), "Texture does not exist");

	Texture& texture = m_textures[id.index];
	texture.update(x, y, width, height, data);
}

//-----------------------------------------------------------------------------
void GLRenderer::destroy_texture(TextureId id)
{
	CE_ASSERT(m_textures_id_table.has(id), "Texture does not exist");

	Texture& texture = m_textures[id.index];
	texture.destroy();

	m_textures_id_table.destroy(id);
}

//-----------------------------------------------------------------------------
ShaderId GLRenderer::create_shader(ShaderType type, const char* text)
{
	CE_ASSERT_NOT_NULL(text);

	const ShaderId& id = m_shaders_id_table.create();

	Shader& shader = m_shaders[id.index];
	shader.create(type, text);

	return id;	
}

//-----------------------------------------------------------------------------
void GLRenderer::destroy_shader(ShaderId id)
{
	CE_ASSERT(m_shaders_id_table.has(id), "Shader does not exist");

	Shader& shader = m_shaders[id.index];
	shader.destroy();

	m_shaders_id_table.destroy(id);
}

//-----------------------------------------------------------------------------
GPUProgramId GLRenderer::create_gpu_program(ShaderId vertex, ShaderId pixel)
{
	CE_ASSERT(m_shaders_id_table.has(vertex), "Vertex shader does not exist");
	CE_ASSERT(m_shaders_id_table.has(pixel), "Pixel shader does not exist");

	const GPUProgramId id = m_gpu_programs_id_table.create();
	GPUProgram& program = m_gpu_programs[id.index];

	program.create(m_shaders[vertex.index], m_shaders[pixel.index]);

	return id;
}

//-----------------------------------------------------------------------------
void GLRenderer::destroy_gpu_program(GPUProgramId id)
{
	CE_ASSERT(m_gpu_programs_id_table.has(id), "GPU program does not exist");

	GPUProgram& program = m_gpu_programs[id.index];

	program.destroy();

	m_gpu_programs_id_table.destroy(id);
}

//-----------------------------------------------------------------------------
UniformId GLRenderer::create_uniform(const char* name, UniformType type)
{
	const UniformId id = m_uniforms_id_table.create(); 
	Uniform& uniform = m_uniforms[id.index];

	uniform.m_name = hash::murmur2_32(name, string::strlen(name), 0);
	uniform.m_type = type;

	return id;
}

//-----------------------------------------------------------------------------
void GLRenderer::destroy_uniform(UniformId id)
{
	CE_ASSERT(m_uniforms_id_table.has(id), "Uniform does not exist");

	m_uniforms_id_table.destroy(id);
}

//-----------------------------------------------------------------------------
RenderTargetId GLRenderer::create_render_target(uint16_t width, uint16_t height, RenderTargetFormat format)
{
	const RenderTargetId id = m_render_targets_id_table.create();
	RenderTarget& target = m_render_targets[id.index];

	target.create(width, height, format);

	return id;
}

//-----------------------------------------------------------------------------
void GLRenderer::destroy_render_target(RenderTargetId id)
{
	CE_ASSERT(m_render_targets_id_table.has(id), "Render target does not exist");

	RenderTarget& target = m_render_targets[id.index];

	target.destroy();

	m_render_targets_id_table.destroy(id);
}

// //-----------------------------------------------------------------------------
// void GLRenderer::set_depth_test(bool test)
// {
// 	if (test)
// 	{
// 		GL_CHECK(glEnable(GL_DEPTH_TEST));
// 	}
// 	else
// 	{
// 		GL_CHECK(glDisable(GL_DEPTH_TEST));
// 	}
// }

// //-----------------------------------------------------------------------------
// void GLRenderer::set_depth_func(CompareFunction func)
// {
// 	GLenum gl_func = GL::compare_function(func);

// 	GL_CHECK(glDepthFunc(gl_func));
// }

//-----------------------------------------------------------------------------
void GLRenderer::frame()
{
	//RenderTargetId old_rt;
	//old_rt.id = INVALID_ID;
	uint8_t layer = 0xFF;

	for (uint32_t s = 0; s < m_render_context.m_num_states; s++)
	{
		const uint64_t key_s = m_render_context.m_keys[s];
		RenderKey key;
		key.decode(key_s);
		const RenderState& cur_state = m_render_context.m_states[s];
		const uint64_t flags = cur_state.m_flags;
		//const RenderTargetId& cur_rt = m_render_context.m_targets[layer];

		// Check if layer changed
		if (key.m_layer != layer)
		{
			layer = key.m_layer;

			// Viewport
			const ViewRect& viewport = m_render_context.m_viewports[layer];
			GL_CHECK(glViewport(viewport.m_x, viewport.m_y, viewport.m_width, viewport.m_height));

			// Clear frame/depth buffer
			const ClearState& clear = m_render_context.m_clears[layer];
			if (clear.m_flags & (CLEAR_COLOR | CLEAR_DEPTH))
			{
				GLbitfield gl_clear = (clear.m_flags & CLEAR_COLOR) ? GL_COLOR_BUFFER_BIT : 0;
				gl_clear |= (clear.m_flags & CLEAR_DEPTH) ? GL_DEPTH_BUFFER_BIT : 0;
				GL_CHECK(glClearColor(clear.m_color.r, clear.m_color.g, clear.m_color.b, clear.m_color.a));
				GL_CHECK(glClearDepth(clear.m_depth));
				GL_CHECK(glClear(gl_clear));
			}
		}

		// Scissor
		const ViewRect& scissor = m_render_context.m_scissors[layer];
		if (scissor.area() != 0)
		{
			GL_CHECK(glEnable(GL_SCISSOR_TEST));
			GL_CHECK(glScissor(scissor.m_x, scissor.m_y, scissor.m_width, scissor.m_height));
		}
		else
		{
			GL_CHECK(glDisable(GL_SCISSOR_TEST));
		}

		// Depth write
		if (flags & (STATE_DEPTH_WRITE))
		{
			GL_CHECK(glDepthMask(flags & STATE_DEPTH_WRITE));
		}

		// Color/Alpha write
		if (flags & (STATE_COLOR_WRITE | STATE_ALPHA_WRITE))
		{
			GLboolean cw = !!(flags & STATE_COLOR_WRITE);
			GLboolean aw = !!(flags & STATE_ALPHA_WRITE);
			GL_CHECK(glColorMask(cw, cw, cw, aw));
		}

		// Face culling
		if (flags & (STATE_CULL_CW | STATE_CULL_CCW))
		{
			if (flags & STATE_CULL_CW)
			{
				GL_CHECK(glEnable(GL_CULL_FACE));
				GL_CHECK(glCullFace(GL_BACK));
			}
			else if (flags & STATE_CULL_CCW)
			{
				GL_CHECK(glEnable(GL_CULL_FACE));
				GL_CHECK(glCullFace(GL_FRONT));
			}
		}
		else
		{
			GL_CHECK(glDisable(GL_CULL_FACE));
		}

		// Bind GPU program
		if (cur_state.program.id != INVALID_ID)
		{
			const GPUProgram& gpu_program = m_gpu_programs[cur_state.program.index];
			GL_CHECK(glUseProgram(gpu_program.m_id));

			for (uint8_t uniform = 0; uniform < gpu_program.m_num_stock_uniforms; uniform++)
			{
				const GLint& uniform_location = gpu_program.m_stock_uniform_locations[uniform];
				const Mat4& view = m_render_context.m_view_matrices[layer];
				const Mat4& projection = m_render_context.m_projection_matrices[layer];

				switch (gpu_program.m_stock_uniforms[uniform])
				{
					case UNIFORM_VIEW:
					{
						GL_CHECK(glUniformMatrix4fv(uniform_location, 1, GL_FALSE, view.to_float_ptr()));
						break;
					}
					case UNIFORM_MODEL:
					{
						GL_CHECK(glUniformMatrix4fv(uniform_location, 1, GL_FALSE, cur_state.pose.to_float_ptr()));
						break;
					}
					case UNIFORM_MODEL_VIEW:
					{
						GL_CHECK(glUniformMatrix4fv(uniform_location, 1, GL_FALSE, (view *
														cur_state.pose).to_float_ptr()));
						break;
					}
					case UNIFORM_MODEL_VIEW_PROJECTION:
					{
						GL_CHECK(glUniformMatrix4fv(uniform_location, 1, GL_FALSE, (projection * view *
														cur_state.pose).to_float_ptr()));
						break;
					}
					case UNIFORM_TIME_SINCE_START:
					{
						GL_CHECK(glUniform1f(uniform_location, device()->time_since_start()));
						break;
					}
					default:
					{
						CE_ASSERT(false, "Oops, wrong stock uniform!");
						break;
					}
				}
			}
		}
		else
		{
			GL_CHECK(glUseProgram(0));
		}

		// Bind array buffers
		const VertexBufferId& vb = cur_state.vb;
		if (vb.id != INVALID_ID)
		{
			const VertexBuffer& vertex_buffer = m_vertex_buffers[vb.index];
			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer.m_id);

			const GPUProgram& gpu_program = m_gpu_programs[cur_state.program.index];
			gpu_program.bind_attributes(vertex_buffer.m_format);
		}
		else
		{
			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
		}

		const IndexBufferId& ib = cur_state.ib;
		if (ib.id != INVALID_ID)
		{
			const IndexBuffer& index_buffer = m_index_buffers[ib.index];
			GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer.m_id));
			GL_CHECK(glDrawElements(GL_TRIANGLES, index_buffer.m_index_count, GL_UNSIGNED_SHORT, 0));
		}
		else
		{
			GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
		}

		// Bind textures
		{
			uint64_t flags = STATE_TEXTURE_0;
			for (uint32_t unit = 0; unit < STATE_MAX_TEXTURES; unit++)
			{
				const Sampler& sampler = cur_state.samplers[unit];

				if (sampler.sampler_id.id != INVALID_ID)
				{
					switch (sampler.flags & SAMPLER_MASK)
					{
						case SAMPLER_TEXTURE:
						{
							Texture& texture = m_textures[sampler.sampler_id.index];
							texture.commit(unit, sampler.flags);
							break;
						}
						default:
						{
							CE_ASSERT(false, "Oops, sampler unknown");
							break;
						}
					}
				}

				flags <<= 1;
			}
		}
	}

	GL_CHECK(glFinish());

	m_render_context.clear();
	m_context.swap_buffers();
}

//-----------------------------------------------------------------------------
Renderer* Renderer::create(Allocator& a)
{
	return CE_NEW(a, GLRenderer);
}

//-----------------------------------------------------------------------------
void Renderer::destroy(Allocator& a, Renderer* renderer)
{
	CE_DELETE(a, renderer);
}

} // namespace crown

