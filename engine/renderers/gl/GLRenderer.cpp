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

#include "Config.h"

#if defined(LINUX) || defined(WINDOWS)
	#include <GL/glew.h>
#elif defined(ANDROID)
	#include <GLES2/gl2.h>
#else
	#error "Oops, wrong platform"
#endif

#include <algorithm>
#include "Allocator.h"
#include "Assert.h"
#include "Types.h"
#include "GLRenderer.h"
#include "Log.h"
#include "Vector4.h"
#include "Matrix3x3.h"
#include "Matrix4x4.h"
#include "Device.h"
#include "Hash.h"
#include "StringUtils.h"

namespace crown
{

//-----------------------------------------------------------------------------
const GLenum PRIMITIVE_TYPE_TABLE[] =
{
	GL_TRIANGLES,
	GL_POINTS,
	GL_LINES
};

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
	GL_CLAMP_TO_EDGE,
	GL_REPEAT
};

//-----------------------------------------------------------------------------
const GLTextureFormatInfo TEXTURE_FORMAT_TABLE[PixelFormat::COUNT] =
{
	{ GL_RGB, GL_RGB },
	{ GL_RGBA, GL_RGBA}
};

// Keep in sync with ShaderAttrib
const char* const SHADER_ATTRIB_NAMES[ShaderAttrib::COUNT] =
{
	"a_position",
	"a_normal",
	"a_color",
	"a_tex_coord0",
	"a_tex_coord1",
	"a_tex_coord2",
	"a_tex_coord3"
};

const char* const SHADER_UNIFORM_NAMES[ShaderUniform::COUNT] =
{
	"u_view",
	"u_model",
	"u_model_view",
	"u_model_view_projection",
	"u_time_since_start"
};

const size_t UNIFORM_SIZE_TABLE[UniformType::END] =
{
	sizeof(int32_t) * 1,
	sizeof(int32_t) * 2,
	sizeof(int32_t) * 3,
	sizeof(int32_t) * 4,
	sizeof(float) * 1,
	sizeof(float) * 2,
	sizeof(float) * 3,
	sizeof(float) * 4,
	sizeof(float) * 9,
	sizeof(float) * 16
};

ShaderUniform::Enum name_to_stock_uniform(const char* uniform)
{
	for (uint8_t i = 0; i < ShaderUniform::COUNT; i++)
	{
		if (string::strcmp(uniform, SHADER_UNIFORM_NAMES[i]) == 0)
		{
			return (ShaderUniform::Enum) i;
		}
	}

	return ShaderUniform::COUNT;
}

/// OpenGL renderer
class RendererImplementation
{
public:

	//-----------------------------------------------------------------------------
	RendererImplementation()
		: m_max_texture_size(0), m_max_texture_units(0), m_max_vertex_indices(0), m_max_vertex_vertices(0),
			m_max_anisotropy(0.0f), m_num_uniforms(0)
	{
		m_min_max_point_size[0] = 0.0f;
		m_min_max_point_size[1] = 0.0f;
		m_min_max_line_width[0] = 0.0f;
		m_min_max_line_width[1] = 0.0f;
	}

	//-----------------------------------------------------------------------------
	~RendererImplementation()
	{
	}

	//-----------------------------------------------------------------------------
	void init()
	{
		m_gl_context.create_context();

		#if defined(LINUX) || defined(WINDOWS)
			GLenum err = glewInit();
			CE_ASSERT(err == GLEW_OK, "Failed to initialize GLEW");
		#endif

		GL_CHECK(glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_max_texture_size));
		GL_CHECK(glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &m_max_texture_units));
		// GL_CHECK(glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &m_max_vertex_indices));
		// GL_CHECK(glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &m_max_vertex_vertices));

		GL_CHECK(glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, &m_min_max_point_size[0]));
		// GL_CHECK(glGetFloatv(GL_LINE_WIDTH_RANGE, &m_min_max_line_width[0]));

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

		#if defined(LINUX) || defined(WINDOWS)
			// Point sprites enabled by default
			GL_CHECK(glEnable(GL_POINT_SPRITE));
			GL_CHECK(glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE));
		#endif

		Log::i("OpenGL Renderer initialized.");
	}

	//-----------------------------------------------------------------------------
	void shutdown()
	{
		m_gl_context.destroy_context();
	}

	//-----------------------------------------------------------------------------
	void render(RenderContext& context)
	{
		//RenderTargetId old_rt;
		//old_rt.id = INVALID_ID;
		uint8_t layer = 0xFF;

		for (uint32_t s = 0; s < context.m_num_states; s++)
		{
			const uint64_t key_s = context.m_keys[s];
			RenderKey key;
			key.decode(key_s);
			const RenderState& cur_state = context.m_states[s];
			const uint64_t flags = cur_state.m_flags;
			//const RenderTargetId& cur_rt = context.m_targets[layer];

			// Check if layer changed
			if (key.m_layer != layer)
			{
				layer = key.m_layer;

				// Viewport
				const ViewRect& viewport = context.m_viewports[layer];
				GL_CHECK(glViewport(viewport.m_x, viewport.m_y, viewport.m_width, viewport.m_height));

				// Clear frame/depth buffer
				const ClearState& clear = context.m_clears[layer];
				if (clear.m_flags & (CLEAR_COLOR | CLEAR_DEPTH))
				{
					GLbitfield gl_clear = (clear.m_flags & CLEAR_COLOR) ? GL_COLOR_BUFFER_BIT : 0;
					gl_clear |= (clear.m_flags & CLEAR_DEPTH) ? GL_DEPTH_BUFFER_BIT : 0;
					GL_CHECK(glClearColor(clear.m_color.r, clear.m_color.g, clear.m_color.b, clear.m_color.a));
					#if defined(LINUX) || defined(WINDOWS)
						GL_CHECK(glClearDepth(clear.m_depth));
					#elif defined(ANDROID)
						GL_CHECK(glClearDepthf(clear.m_depth));
					#endif
					GL_CHECK(glClear(gl_clear));
				}

				GL_CHECK(glEnable(GL_DEPTH_TEST));
			}

			// Scissor
			const ViewRect& scissor = context.m_scissors[layer];
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
			else
			{
				GL_CHECK(glDisable(GL_CULL_FACE));
			}

			// Bind GPU program
			if (cur_state.program.id != INVALID_ID)
			{
				const GPUProgram& gpu_program = m_gpu_programs[cur_state.program.index];
				GL_CHECK(glUseProgram(gpu_program.m_id));
				// Not necessarily here...
				gpu_program.commit();

				for (uint8_t uniform = 0; uniform < gpu_program.m_num_stock_uniforms; uniform++)
				{
					const GLint& uniform_location = gpu_program.m_stock_uniform_locations[uniform];
					const Matrix4x4& view = context.m_view_matrices[layer];
					const Matrix4x4& projection = context.m_projection_matrices[layer];

					switch (gpu_program.m_stock_uniforms[uniform])
					{
						case ShaderUniform::VIEW:
						{
							GL_CHECK(glUniformMatrix4fv(uniform_location, 1, GL_FALSE, view.to_float_ptr()));
							break;
						}
						case ShaderUniform::MODEL:
						{
							GL_CHECK(glUniformMatrix4fv(uniform_location, 1, GL_FALSE, cur_state.pose.to_float_ptr()));
							break;
						}
						case ShaderUniform::MODEL_VIEW:
						{
							GL_CHECK(glUniformMatrix4fv(uniform_location, 1, GL_FALSE, (view *
															cur_state.pose).to_float_ptr()));
							break;
						}
						case ShaderUniform::MODEL_VIEW_PROJECTION:
						{
							GL_CHECK(glUniformMatrix4fv(uniform_location, 1, GL_FALSE, (projection * view *
															cur_state.pose).to_float_ptr()));
							break;
						}
						case ShaderUniform::TIME_SINCE_START:
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
				GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer.m_id));

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
				uint32_t prim_type = (flags & STATE_PRIMITIVE_MASK) >> STATE_PRIMITIVE_SHIFT;
				GLenum gl_prim_type = PRIMITIVE_TYPE_TABLE[prim_type];
				GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer.m_id));
				GL_CHECK(glDrawElements(gl_prim_type, index_buffer.m_index_count, GL_UNSIGNED_SHORT, 0));
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
		m_gl_context.swap_buffers();
	}

private:

	GLContext			m_gl_context;

	// Limits
	int32_t				m_max_texture_size;
	int32_t				m_max_texture_units;
	int32_t				m_max_vertex_indices;
	int32_t				m_max_vertex_vertices;

	float				m_max_anisotropy;
	float				m_min_max_point_size[2];
	float				m_min_max_line_width[2];

	VertexBuffer		m_vertex_buffers[CROWN_MAX_VERTEX_BUFFERS];
	IndexBuffer			m_index_buffers[CROWN_MAX_INDEX_BUFFERS];
	Texture				m_textures[CROWN_MAX_TEXTURES];
	Shader				m_shaders[CROWN_MAX_SHADERS];
	GPUProgram			m_gpu_programs[CROWN_MAX_GPU_PROGRAMS];
	uint32_t			m_num_uniforms;
	Uniform				m_uniforms[CROWN_MAX_UNIFORMS];
	RenderTarget		m_render_targets[CROWN_MAX_RENDER_TARGETS];

private:

	friend class		Renderer;
};

//-----------------------------------------------------------------------------
Renderer::Renderer(Allocator& a)
	: m_allocator(a), m_impl(NULL), m_thread("render-thread"), m_submit(&m_contexts[0]), m_draw(&m_contexts[1]),
		m_is_initialized(false), m_should_run(false)
{
	m_impl = CE_NEW(a, RendererImplementation);
}

//-----------------------------------------------------------------------------
Renderer::~Renderer()
{
	CE_DELETE(m_allocator, m_impl);
}

//-----------------------------------------------------------------------------
void Renderer::init_impl()
{
	m_impl->init();
}

//-----------------------------------------------------------------------------
void Renderer::shutdown_impl()
{
	m_impl->shutdown();
}

//-----------------------------------------------------------------------------
void Renderer::render_impl()
{
	m_impl->render(*m_draw);

	m_draw->clear();
}

//-----------------------------------------------------------------------------
void Renderer::create_vertex_buffer_impl(VertexBufferId id, size_t count, VertexFormat::Enum format, const void* vertices)
{
	m_impl->m_vertex_buffers[id.index].create(count, format, vertices);
}

//-----------------------------------------------------------------------------
void Renderer::create_dynamic_vertex_buffer_impl(VertexBufferId id, size_t count, VertexFormat::Enum format)
{
	m_impl->m_vertex_buffers[id.index].create(count, format, NULL);
}

//-----------------------------------------------------------------------------
void Renderer::update_vertex_buffer_impl(VertexBufferId id, size_t offset, size_t count, const void* vertices)
{
	m_impl->m_vertex_buffers[id.index].update(offset, count, vertices);
}

//-----------------------------------------------------------------------------
void Renderer::destroy_vertex_buffer_impl(VertexBufferId id)
{
	m_impl->m_vertex_buffers[id.index].destroy();
}

//-----------------------------------------------------------------------------
void Renderer::create_index_buffer_impl(IndexBufferId id, size_t count, const void* indices)
{
	m_impl->m_index_buffers[id.index].create(count, indices);
}

//-----------------------------------------------------------------------------
void Renderer::create_dynamic_index_buffer_impl(IndexBufferId id, size_t count)
{
	m_impl->m_index_buffers[id.index].create(count, NULL);
}

//-----------------------------------------------------------------------------
void Renderer::update_index_buffer_impl(IndexBufferId id, size_t offset, size_t count, const void* indices)
{
	m_impl->m_index_buffers[id.index].update(offset, count, indices);	
}

//-----------------------------------------------------------------------------
void Renderer::destroy_index_buffer_impl(IndexBufferId id)
{
	m_impl->m_index_buffers[id.index].destroy();
}

//-----------------------------------------------------------------------------
void Renderer::create_texture_impl(TextureId id, uint32_t width, uint32_t height, PixelFormat::Enum format, const void* data)
{
	m_impl->m_textures[id.index].create(width, height, format, data);
}

//-----------------------------------------------------------------------------
void Renderer::update_texture_impl(TextureId id, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void* data)
{
	m_impl->m_textures[id.index].update(x, y, width, height, data);	
}

//-----------------------------------------------------------------------------
void Renderer::destroy_texture_impl(TextureId id)
{
	m_impl->m_textures[id.index].destroy();
}

//-----------------------------------------------------------------------------
void Renderer::create_shader_impl(ShaderId id, ShaderType::Enum type, const char* text)
{
	m_impl->m_shaders[id.index].create(type, text);
}

//-----------------------------------------------------------------------------
void Renderer::destroy_shader_impl(ShaderId id)
{
	m_impl->m_shaders[id.index].destroy();
}

//-----------------------------------------------------------------------------
void Renderer::create_gpu_program_impl(GPUProgramId id, ShaderId vertex, ShaderId pixel)
{
	Shader& vs = m_impl->m_shaders[vertex.index];
	Shader& ps = m_impl->m_shaders[pixel.index];
	m_impl->m_gpu_programs[id.index].create(vs, ps, m_impl->m_num_uniforms, m_impl->m_uniforms);
}

//-----------------------------------------------------------------------------
void Renderer::destroy_gpu_program_impl(GPUProgramId id)
{
	m_impl->m_gpu_programs[id.index].destroy();
}

//-----------------------------------------------------------------------------
void Renderer::create_uniform_impl(UniformId id, const char* name, UniformType::Enum type, uint8_t num)
{
	m_impl->m_uniforms[id.index].create(name, type, num);
	m_impl->m_num_uniforms++;
}

//-----------------------------------------------------------------------------
void Renderer::update_uniform_impl(UniformId id, size_t size, const void* data)
{
	m_impl->m_uniforms[id.index].update(size, data);
}

//-----------------------------------------------------------------------------
void Renderer::destroy_uniform_impl(UniformId id)
{
	m_impl->m_uniforms[id.index].destroy();
	m_impl->m_num_uniforms--;
}

// //-----------------------------------------------------------------------------
// void Renderer::create_render_target_impl(RenderTargetId id, uint16_t width, uint16_t height, RenderTargetFormat::Enum format)
// {

// }

// //-----------------------------------------------------------------------------
// void Renderer::destroy_render_target_impl(RenderTargetId id)
// {

// }

} // namespace crown
