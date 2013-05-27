/*
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

#include "Assert.h"
#include "Types.h"
#include "GLRenderer.h"
#include "GLUtils.h"
#include "Log.h"
#include "Material.h"
#include "Vec2.h"
#include "Vec3.h"
#include "Vec4.h"
#include "Mat3.h"
#include "Mat4.h"
#include "Device.h"
#include "ResourceManager.h"
#include "VertexShaderResource.h"
#include "PixelShaderResource.h"

namespace crown
{

//-----------------------------------------------------------------------------
static const char* gl_error_to_string(GLenum error)
{
	switch (error)
	{
		case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
		case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
		default: return "UNKNOWN_GL_ERROR";
	}
}

//-----------------------------------------------------------------------------
#ifdef CROWN_DEBUG
	#define GL_CHECK(function)\
		function;\
		do { GLenum error; ce_assert((error = glGetError()) == GL_NO_ERROR,\
				"OpenGL error: %s", gl_error_to_string(error)); } while (0)
#else
	#define GL_CHECK(function)\
		function;
#endif

//-----------------------------------------------------------------------------
GLRenderer::GLRenderer() :
	m_max_texture_size(0),
	m_max_texture_units(0),
	m_max_vertex_indices(0),
	m_max_vertex_vertices(0),
	m_max_anisotropy(0.0f),

	m_ambient_light_color(Color4::GRAY),

	m_textures_id_table(m_allocator, MAX_TEXTURES),
	m_active_texture_unit(0),

	m_vertex_buffers_id_table(m_allocator, MAX_VERTEX_BUFFERS),
	m_index_buffers_id_table(m_allocator, MAX_INDEX_BUFFERS),
	m_vertex_shaders_id_table(m_allocator, MAX_VERTEX_SHADERS),
	m_pixel_shaders_id_table(m_allocator, MAX_PIXEL_SHADERS),
	m_gpu_programs_id_table(m_allocator, 128)
	//m_render_buffers_id_table(m_allocator, MAX_RENDER_BUFFERS)
{
	m_min_max_point_size[0] = 0.0f;
	m_min_max_point_size[1] = 0.0f;
	m_min_max_line_width[0] = 0.0f;
	m_min_max_line_width[1] = 0.0f;

	// Initialize viewport and scissor
	m_viewport[0] = 0;
	m_viewport[1] = 0;
	m_viewport[2] = 0;
	m_viewport[3] = 0;

	m_scissor[0] = 0;
	m_scissor[1] = 0;
	m_scissor[2] = 0;
	m_scissor[3] = 0;

	// Initialize texture units
	for (uint32_t i = 0; i < MAX_TEXTURE_UNITS; i++)
	{
		m_texture_unit[i] = 0;
		m_texture_unit_target[i] = GL_TEXTURE_2D;
	}

	// Initialize the matrices
	for (uint32_t i = 0; i < MT_COUNT; i++)
	{
		m_matrix[i].load_identity();
	}

	m_model_view_matrix.load_identity();
	m_model_view_projection_matrix.load_identity();
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
	ce_assert(err == GLEW_OK, "Failed to initialize GLEW");

	GL_CHECK(glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_max_texture_size));
	GL_CHECK(glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &m_max_texture_units));
	GL_CHECK(glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &m_max_vertex_indices));
	GL_CHECK(glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &m_max_vertex_vertices));

	// Check for anisotropic filter support
	if (GLEW_EXT_texture_filter_anisotropic)
	{
		GL_CHECK(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &m_max_anisotropy));
	}

	GL_CHECK(glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, &m_min_max_point_size[0]));
	GL_CHECK(glGetFloatv(GL_LINE_WIDTH_RANGE, &m_min_max_line_width[0]));

	Log::i("OpenGL Vendor\t: %s", glGetString(GL_VENDOR));
	Log::i("OpenGL Renderer\t: %s", glGetString(GL_RENDERER));
	Log::i("OpenGL Version\t: %s", glGetString(GL_VERSION));

	Log::d("Min Point Size\t: %f", m_min_max_point_size[0]);
	Log::d("Max Point Size\t: %f", m_min_max_point_size[1]);
	Log::d("Min Line Width\t: %f", m_min_max_line_width[0]);
	Log::d("Max Line Width\t: %f", m_min_max_line_width[1]);
	Log::d("Max Texture Size\t: %dx%d", m_max_texture_size, m_max_texture_size);
	Log::d("Max Texture Units\t: %d", m_max_texture_units);
	Log::d("Max Vertex Indices\t: %d", m_max_vertex_indices);
	Log::d("Max Vertex Vertices\t: %d", m_max_vertex_vertices);
	Log::d("Max Anisotropy\t: %f", m_max_anisotropy);

	GL_CHECK(glDisable(GL_TEXTURE_2D));

	GL_CHECK(glDisable(GL_BLEND));
	GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	GL_CHECK(glBlendEquation(GL_FUNC_ADD));

	GL_CHECK(glFrontFace(GL_CCW));
	GL_CHECK(glEnable(GL_CULL_FACE));
	GL_CHECK(glShadeModel(GL_SMOOTH));

	// Set the default framebuffer clear color
	GL_CHECK(glClearColor(0.5f, 0.5f, 0.5f, 0.5f));

	// Enable depth test
	GL_CHECK(glEnable(GL_DEPTH_TEST));
	GL_CHECK(glDepthFunc(GL_LEQUAL));
	GL_CHECK(glClearDepth(1.0));

	// Enable scissor test
	GL_CHECK(glEnable(GL_SCISSOR_TEST));

	// Disable dithering
	GL_CHECK(glDisable(GL_DITHER));

	// Point sprites enabled by default
	GL_CHECK(glEnable(GL_POINT_SPRITE));
	GL_CHECK(glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE));

	Log::i("OpenGL Renderer initialized.");

	load_default_shaders();
}

//-----------------------------------------------------------------------------
void GLRenderer::shutdown()
{
	unload_default_shaders();

	m_context.destroy_context();
}

//-----------------------------------------------------------------------------
VertexBufferId GLRenderer::create_vertex_buffer(size_t count, VertexFormat format, const void* vertices)
{
	const VertexBufferId id = m_vertex_buffers_id_table.create();

	VertexBuffer& buffer = m_vertex_buffers[id.index];

	GL_CHECK(glGenBuffers(1, &buffer.gl_object));

	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, buffer.gl_object));
	GL_CHECK(glBufferData(GL_ARRAY_BUFFER, count * Vertex::bytes_per_vertex(format), vertices, GL_STATIC_DRAW));

	buffer.count = count;
	buffer.format = format;

	return id;
}

//-----------------------------------------------------------------------------
VertexBufferId GLRenderer::create_dynamic_vertex_buffer(size_t count, VertexFormat format, const void* vertices)
{
	const VertexBufferId id = m_vertex_buffers_id_table.create();

	VertexBuffer& buffer = m_vertex_buffers[id.index];

	GL_CHECK(glGenBuffers(1, &buffer.gl_object));

	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, buffer.gl_object));
	GL_CHECK(glBufferData(GL_ARRAY_BUFFER, count * Vertex::bytes_per_vertex(format), vertices, GL_STREAM_DRAW));

	buffer.count = count;
	buffer.format = format;

	return id;
}

//-----------------------------------------------------------------------------
void GLRenderer::update_vertex_buffer(VertexBufferId id, size_t offset, size_t count, const void* vertices)
{
	ce_assert(m_vertex_buffers_id_table.has(id), "Vertex buffer does not exist");

	VertexBuffer& buffer = m_vertex_buffers[id.index];

	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, buffer.gl_object));
	GL_CHECK(glBufferSubData(GL_ARRAY_BUFFER, offset * Vertex::bytes_per_vertex(buffer.format),
					count * Vertex::bytes_per_vertex(buffer.format), vertices));
}

//-----------------------------------------------------------------------------
void GLRenderer::destroy_vertex_buffer(VertexBufferId id)
{
	ce_assert(m_vertex_buffers_id_table.has(id), "Vertex buffer does not exist");

	VertexBuffer& buffer = m_vertex_buffers[id.index];

	GL_CHECK(glDeleteBuffers(1, &buffer.gl_object));

	m_vertex_buffers_id_table.destroy(id);
}

//-----------------------------------------------------------------------------
IndexBufferId GLRenderer::create_index_buffer(size_t count, const void* indices)
{
	const IndexBufferId id = m_index_buffers_id_table.create();

	IndexBuffer& buffer = m_index_buffers[id.index];

	GL_CHECK(glGenBuffers(1, &buffer.gl_object));

	GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.gl_object));
	GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLushort), indices, GL_STATIC_DRAW));

	buffer.index_count = count;

	return id;
}

//-----------------------------------------------------------------------------
void GLRenderer::destroy_index_buffer(IndexBufferId id)
{
	ce_assert(m_index_buffers_id_table.has(id), "Index buffer does not exist");

	IndexBuffer& buffer = m_index_buffers[id.index];

	GL_CHECK(glDeleteBuffers(1, &buffer.gl_object));

	m_index_buffers_id_table.destroy(id);
}

//-----------------------------------------------------------------------------
TextureId GLRenderer::create_texture(uint32_t width, uint32_t height, PixelFormat format, const void* data)
{
	const TextureId id = m_textures_id_table.create();

	Texture& gl_texture = m_textures[id.index];

	GL_CHECK(glGenTextures(1, &gl_texture.gl_object));

	GL_CHECK(glBindTexture(GL_TEXTURE_2D, gl_texture.gl_object));

	GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE));

	// FIXME
	GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
				 GL::pixel_format(format), GL_UNSIGNED_BYTE, data));

	gl_texture.format = format;

	return id;
}

//-----------------------------------------------------------------------------
void GLRenderer::update_texture(TextureId id, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void* data)
{
	ce_assert(m_textures_id_table.has(id), "Texture does not exist");

	Texture& gl_texture = m_textures[id.index];

	GL_CHECK(glBindTexture(GL_TEXTURE_2D, gl_texture.gl_object));

	GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, GL::pixel_format(gl_texture.format),
					GL_UNSIGNED_BYTE, data));
}

//-----------------------------------------------------------------------------
void GLRenderer::destroy_texture(TextureId id)
{
	ce_assert(m_textures_id_table.has(id), "Texture does not exist");

	Texture& gl_texture = m_textures[id.index];

	GL_CHECK(glDeleteTextures(1, &gl_texture.gl_object));
}

//-----------------------------------------------------------------------------
VertexShaderId GLRenderer::create_vertex_shader(const char* program)
{
	ce_assert(program != NULL, "Program must be != NULL");

	const VertexShaderId& id = m_vertex_shaders_id_table.create();

	VertexShader& gl_shader = m_vertex_shaders[id.index];

	gl_shader.gl_object = GL_CHECK(glCreateShader(GL_VERTEX_SHADER));

	GL_CHECK(glShaderSource(gl_shader.gl_object, 1, &program, NULL));

	GL_CHECK(glCompileShader(gl_shader.gl_object));

	GLint success;
	GL_CHECK(glGetShaderiv(gl_shader.gl_object, GL_COMPILE_STATUS, &success));

	if (!success)
	{
		GLchar info_log[256];

		GL_CHECK(glGetShaderInfoLog(gl_shader.gl_object, 256, NULL, info_log));

		Log::e("Vertex shader compilation failed.");
		Log::e("Log: %s", info_log);
		ce_assert(0, "");
	}

	return id;
}

//-----------------------------------------------------------------------------
void GLRenderer::destroy_vertex_shader(VertexShaderId id)
{
	ce_assert(m_vertex_shaders_id_table.has(id), "Vertex shader does not exist");

	VertexShader& gl_shader = m_vertex_shaders[id.index];

	GL_CHECK(glDeleteShader(gl_shader.gl_object));
}

//-----------------------------------------------------------------------------
PixelShaderId GLRenderer::create_pixel_shader(const char* program)
{
	ce_assert(program != NULL, "Program must be != NULL");

	const PixelShaderId& id = m_pixel_shaders_id_table.create();

	PixelShader& gl_shader = m_pixel_shaders[id.index];

	gl_shader.gl_object = GL_CHECK(glCreateShader(GL_FRAGMENT_SHADER));

	GL_CHECK(glShaderSource(gl_shader.gl_object, 1, &program, NULL));

	GL_CHECK(glCompileShader(gl_shader.gl_object));

	GLint success;
	GL_CHECK(glGetShaderiv(gl_shader.gl_object, GL_COMPILE_STATUS, &success));

	if (!success)
	{
		GLchar info_log[256];

		GL_CHECK(glGetShaderInfoLog(gl_shader.gl_object, 256, NULL, info_log));

		Log::e("Pixel shader compilation failed.");
		Log::e("Log: %s", info_log);
		ce_assert(0, "");
	}

	return id;	
}

//-----------------------------------------------------------------------------
void GLRenderer::destroy_pixel_shader(PixelShaderId id)
{
	ce_assert(m_pixel_shaders_id_table.has(id), "Pixel shader does not exist");

	PixelShader& gl_shader = m_pixel_shaders[id.index];

	GL_CHECK(glDeleteShader(gl_shader.gl_object));	
}

//-----------------------------------------------------------------------------
GPUProgramId GLRenderer::create_gpu_program(VertexShaderId vs, PixelShaderId ps)
{
	ce_assert(m_vertex_shaders_id_table.has(vs), "Vertex shader does not exist");
	ce_assert(m_pixel_shaders_id_table.has(ps), "Pixel shader does not exist");

	const GPUProgramId id = m_gpu_programs_id_table.create();

	GPUProgram& gl_program = m_gpu_programs[id.index];

	gl_program.gl_object = GL_CHECK(glCreateProgram());

	GL_CHECK(glAttachShader(gl_program.gl_object, m_vertex_shaders[id.index].gl_object));
	GL_CHECK(glAttachShader(gl_program.gl_object, m_pixel_shaders[id.index].gl_object));

	GL_CHECK(glBindAttribLocation(gl_program.gl_object, SA_VERTEX, "vertex"));
	GL_CHECK(glBindAttribLocation(gl_program.gl_object, SA_COORDS, "coords"));
	GL_CHECK(glBindAttribLocation(gl_program.gl_object, SA_NORMAL, "normal"));

	GL_CHECK(glLinkProgram(gl_program.gl_object));

	GLint success;
	GL_CHECK(glGetProgramiv(gl_program.gl_object, GL_LINK_STATUS, &success));

	if (!success)
	{
		GLchar info_log[256];
		GL_CHECK(glGetProgramInfoLog(gl_program.gl_object, 256, NULL, info_log));
		Log::e("GPU program compilation failed.\n");
		Log::e("Log: %s", info_log);
	}

	return id;
}

//-----------------------------------------------------------------------------
void GLRenderer::destroy_gpu_program(GPUProgramId id)
{
	ce_assert(m_gpu_programs_id_table.has(id), "GPU program does not exist");

	GPUProgram& gl_program = m_gpu_programs[id.index];

	GL_CHECK(glDeleteProgram(gl_program.gl_object));
}

//-----------------------------------------------------------------------------
void GLRenderer::set_gpu_program_bool_uniform(GPUProgramId id, const char* name, bool value)
{
	ce_assert(m_gpu_programs_id_table.has(id), "GPU program does not exist");

	const GLint uniform = find_gpu_program_uniform(m_gpu_programs[id.index].gl_object, name);

	GL_CHECK(glUniform1i(uniform, (GLint) value));
}

//-----------------------------------------------------------------------------
void GLRenderer::set_gpu_program_int_uniform(GPUProgramId id, const char* name, int value)
{
	ce_assert(m_gpu_programs_id_table.has(id), "GPU program does not exist");

	const GLint uniform = find_gpu_program_uniform(m_gpu_programs[id.index].gl_object, name);

	GL_CHECK(glUniform1i(uniform, (GLint) value));
}

//-----------------------------------------------------------------------------
void GLRenderer::set_gpu_program_vec2_uniform(GPUProgramId id, const char* name, const Vec2& value)
{
	ce_assert(m_gpu_programs_id_table.has(id), "GPU program does not exist");

	const GLint uniform = find_gpu_program_uniform(m_gpu_programs[id.index].gl_object, name);

	GL_CHECK(glUniform2fv(uniform, 1, value.to_float_ptr()));
}

//-----------------------------------------------------------------------------
void GLRenderer::set_gpu_program_vec3_uniform(GPUProgramId id, const char* name, const Vec3& value)
{
	ce_assert(m_gpu_programs_id_table.has(id), "GPU program does not exist");

	const GLint uniform = find_gpu_program_uniform(m_gpu_programs[id.index].gl_object, name);

	GL_CHECK(glUniform3fv(uniform, 1, value.to_float_ptr()));
}

//-----------------------------------------------------------------------------
void GLRenderer::set_gpu_program_vec4_uniform(GPUProgramId id, const char* name, const Vec4& value)
{
	ce_assert(m_gpu_programs_id_table.has(id), "GPU program does not exist");

	const GLint uniform = find_gpu_program_uniform(m_gpu_programs[id.index].gl_object, name);

	GL_CHECK(glUniform4fv(uniform, 1, value.to_float_ptr()));
}

//-----------------------------------------------------------------------------
void GLRenderer::set_gpu_porgram_mat3_uniform(GPUProgramId id, const char* name, const Mat3& value)
{
	ce_assert(m_gpu_programs_id_table.has(id), "GPU program does not exist");

	const GLint uniform = find_gpu_program_uniform(m_gpu_programs[id.index].gl_object, name);

	GL_CHECK(glUniformMatrix3fv(uniform, 1, GL_FALSE, value.to_float_ptr()));
}

//-----------------------------------------------------------------------------
void GLRenderer::set_gpu_program_mat4_uniform(GPUProgramId id, const char* name, const Mat4& value)
{
	ce_assert(m_gpu_programs_id_table.has(id), "GPU program does not exist");

	const GLint uniform = find_gpu_program_uniform(m_gpu_programs[id.index].gl_object, name);

	GL_CHECK(glUniformMatrix4fv(uniform, 1, GL_FALSE, value.to_float_ptr()));
}

//-----------------------------------------------------------------------------
void GLRenderer::set_gpu_program_sampler_uniform(GPUProgramId id, const char* name, uint32_t value)
{
	ce_assert(m_gpu_programs_id_table.has(id), "GPU program does not exist");

	const GLint uniform = find_gpu_program_uniform(m_gpu_programs[id.index].gl_object, name);

	GL_CHECK(glUniform1i(uniform, (GLint) value));	
}

//-----------------------------------------------------------------------------
void GLRenderer::bind_gpu_program(GPUProgramId id) const
{
	ce_assert(m_gpu_programs_id_table.has(id), "GPU program does not exist");

	const GPUProgram& gl_program = m_gpu_programs[id.index];

	GL_CHECK(glUseProgram(gl_program.gl_object));
}

//-----------------------------------------------------------------------------
// RenderBufferId GLRenderer::create_render_buffer(uint32_t width, uint32_t height, PixelFormat format)
// {
// 	const RenderBufferId id = m_render_buffers_id_table.create();

// 	GLRenderBuffer& buffer = m_render_buffers[id.index];

// 	if (GLEW_EXT_framebuffer_object)
// 	{
// 		GL_CHECK(glGenFramebuffersEXT(1, &buffer.GL_CHECK(gl_frame_buffer));
// 		GL_CHECK(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, buffer.GL_CHECK(gl_frame_buffer));

// 		GL_CHECK(glGenRenderbuffersEXT(1, &buffer.GL_CHECK(gl_render_buffer));
// 		GL_CHECK(glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, buffer.GL_CHECK(gl_render_buffer));

// 		GL_CHECK(glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA8, width, height));

// 		GL_CHECK(glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, buffer.GL_CHECK(gl_render_buffer));

// 		GL_CHECK(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0));
// 	}

// 	return id;
// }

//-----------------------------------------------------------------------------
// void GLRenderer::destroy_render_buffer(RenderBufferId id)
// {
// 	GLRenderBuffer& buffer = m_render_buffers[id.index];

// 	if (GLEW_EXT_framebuffer_object)
// 	{
// 		GL_CHECK(glDeleteFramebuffersEXT(1, &buffer.GL_CHECK(gl_frame_buffer));
// 		GL_CHECK(glDeleteRenderbuffersEXT(1, &buffer.GL_CHECK(gl_render_buffer));
// 	}

// 	m_render_buffers_id_table.destroy(id);
// }

//-----------------------------------------------------------------------------
void GLRenderer::set_clear_color(const Color4& color)
{
	GL_CHECK(glClearColor(color.r, color.g, color.b, color.a));
}

//-----------------------------------------------------------------------------
void GLRenderer::set_ambient_light(const Color4& color)
{
	m_ambient_light_color = color;
}

//-----------------------------------------------------------------------------
void GLRenderer::bind_texture(uint32_t unit, TextureId texture)
{
	ce_assert(m_textures_id_table.has(texture), "Texture does not exist");

	if (!activate_texture_unit(unit))
	{
		return;
	}

	m_texture_unit_target[unit] = GL_TEXTURE_2D;
	m_texture_unit[unit] = m_textures[texture.index].gl_object;

	GL_CHECK(glEnable(m_texture_unit_target[unit]));
	GL_CHECK(glBindTexture(m_texture_unit_target[unit], m_texture_unit[unit]));
}

//-----------------------------------------------------------------------------
void GLRenderer::set_texturing(uint32_t unit, bool texturing)
{
	if (!activate_texture_unit(unit))
		return;

	if (texturing)
	{
		GL_CHECK(glEnable(m_texture_unit_target[unit]));
	}
	else
	{
		GL_CHECK(glDisable(m_texture_unit_target[unit]));
	}
}

//-----------------------------------------------------------------------------
void GLRenderer::set_texture_wrap(uint32_t unit, TextureWrap wrap)
{
	GLenum gl_wrap = GL::texture_wrap(wrap);

	GL_CHECK(glTexParameteri(m_texture_unit_target[unit], GL_TEXTURE_WRAP_S, gl_wrap));
	GL_CHECK(glTexParameteri(m_texture_unit_target[unit], GL_TEXTURE_WRAP_T, gl_wrap));
}

//-----------------------------------------------------------------------------
void GLRenderer::set_texture_filter(uint32_t unit, TextureFilter filter)
{
	if (!activate_texture_unit(unit))
		return;

	GLint min_filter;
	GLint mag_filter;

	GL::texture_filter(filter, min_filter, mag_filter);

	GL_CHECK(glTexParameteri(m_texture_unit_target[unit], GL_TEXTURE_MIN_FILTER, min_filter));
	GL_CHECK(glTexParameteri(m_texture_unit_target[unit], GL_TEXTURE_MAG_FILTER, mag_filter));
}

//-----------------------------------------------------------------------------
void GLRenderer::set_backface_culling(bool culling)
{
	if (culling)
	{
		GL_CHECK(glEnable(GL_CULL_FACE));
	}
	else
	{
		GL_CHECK(glDisable(GL_CULL_FACE));
	}
}

//-----------------------------------------------------------------------------
void GLRenderer::set_depth_test(bool test)
{
	if (test)
	{
		GL_CHECK(glEnable(GL_DEPTH_TEST));
	}
	else
	{
		GL_CHECK(glDisable(GL_DEPTH_TEST));
	}
}

//-----------------------------------------------------------------------------
void GLRenderer::set_depth_write(bool write)
{
	GL_CHECK(glDepthMask((GLboolean) write));
}

//-----------------------------------------------------------------------------
void GLRenderer::set_depth_func(CompareFunction func)
{
	GLenum gl_func = GL::compare_function(func);

	GL_CHECK(glDepthFunc(gl_func));
}

//-----------------------------------------------------------------------------
void GLRenderer::set_blending(bool blending)
{
	if (blending)
	{
		GL_CHECK(glEnable(GL_BLEND));
	}
	else
	{
		GL_CHECK(glDisable(GL_BLEND));
	}
}

//-----------------------------------------------------------------------------
void GLRenderer::set_blending_params(BlendEquation equation, BlendFunction src, BlendFunction dst, const Color4& color)
{
	GLenum gl_equation = GL::blend_equation(equation);

	GL_CHECK(glBlendEquation(gl_equation));

	GLenum gl_src_factor = GL::blend_function(src);
	GLenum gl_dst_factor = GL::blend_function(dst);

	GL_CHECK(glBlendFunc(gl_src_factor, gl_dst_factor));

	GL_CHECK(glBlendColor(color.r, color.g, color.b, color.a));
}

//-----------------------------------------------------------------------------
void GLRenderer::set_color_write(bool write)
{
	if (write)
	{
		GL_CHECK(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
	}
	else
	{
		GL_CHECK(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));
	}
}

//-----------------------------------------------------------------------------
void GLRenderer::set_front_face(FrontFace face)
{
	const GLenum gl_face = (face == FF_CCW) ? GL_CCW : GL_CW;

	GL_CHECK(glFrontFace(gl_face));
}

//-----------------------------------------------------------------------------
void GLRenderer::set_viewport_params(int32_t x, int32_t y, int32_t width, int32_t height)
{
	m_viewport[0] = x;
	m_viewport[1] = y;
	m_viewport[2] = width;
	m_viewport[3] = height;

	GL_CHECK(glViewport(x, y, width, height));
}

//-----------------------------------------------------------------------------
void GLRenderer::get_viewport_params(int32_t& x, int32_t& y, int32_t& width, int32_t& height)
{
	x = m_viewport[0];
	y = m_viewport[1];
	width = m_viewport[2];
	height = m_viewport[3];
}

//-----------------------------------------------------------------------------
void GLRenderer::set_scissor(bool scissor)
{
	if (scissor)
	{
		GL_CHECK(glEnable(GL_SCISSOR_TEST));
	}
	else
	{
		GL_CHECK(glDisable(GL_SCISSOR_TEST));
	}
}

//-----------------------------------------------------------------------------
void GLRenderer::set_scissor_params(int32_t x, int32_t y, int32_t width, int32_t height)
{
	m_scissor[0] = x;
	m_scissor[1] = y;
	m_scissor[2] = width;
	m_scissor[3] = height;

	GL_CHECK(glScissor(x, y, width, height));
}

//-----------------------------------------------------------------------------
void GLRenderer::get_scissor_params(int32_t& x, int32_t& y, int32_t& width, int32_t& height)
{
	x = m_scissor[0];
	y = m_scissor[1];
	width = m_scissor[2];
	height = m_scissor[3];
}

//-----------------------------------------------------------------------------
void GLRenderer::frame()
{
	// Clear frame/depth buffer
	GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	// Bind the default gpu program
	bind_gpu_program(m_default_gpu_program);

	set_gpu_program_mat4_uniform(m_default_gpu_program, "mvp_matrix", m_model_view_projection_matrix);
	set_gpu_program_vec3_uniform(m_default_gpu_program, "color", Vec3(0, 1, 0));

	GL_CHECK(glFinish());

	m_context.swap_buffers();
}

//-----------------------------------------------------------------------------
Mat4 GLRenderer::get_matrix(MatrixType type) const
{
	return m_matrix[type];
}

//-----------------------------------------------------------------------------
void GLRenderer::set_matrix(MatrixType type, const Mat4& matrix)
{
	m_matrix[type] = matrix;

	switch (type)
	{
		case MT_VIEW:
		case MT_MODEL:
		{
			m_model_view_matrix = m_matrix[MT_VIEW] * m_matrix[MT_MODEL];
			break;
		}
		case MT_PROJECTION:
		{
			m_model_view_projection_matrix =  m_matrix[MT_PROJECTION] * m_model_view_matrix;
			break;
		}
		default:
		{
			break;
			ce_assert(0, "");
		}
	}
}

//-----------------------------------------------------------------------------
void GLRenderer::bind_vertex_buffer(VertexBufferId vb) const
{
	ce_assert(m_vertex_buffers_id_table.has(vb), "Vertex buffer does not exist");

	const VertexBuffer& vertex_buffer = m_vertex_buffers[vb.index];

	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer.gl_object));

	switch (vertex_buffer.format)
	{
		case VF_XY_FLOAT_32:
		{
			GL_CHECK(glEnableVertexAttribArray(SA_VERTEX));
			GL_CHECK(glVertexAttribPointer(SA_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, 0));
			break;
		}
		case VF_XYZ_FLOAT_32:
		{
			GL_CHECK(glEnableVertexAttribArray(SA_VERTEX));
			GL_CHECK(glVertexAttribPointer(SA_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, 0));
			break;
		}
		case VF_UV_FLOAT_32:
		{
			GL_CHECK(glEnableVertexAttribArray(SA_COORDS));
			GL_CHECK(glVertexAttribPointer(SA_COORDS, 2, GL_FLOAT, GL_FALSE, 0, 0));
			break;
		}
		case VF_UVT_FLOAT_32:
		{
			GL_CHECK(glEnableVertexAttribArray(SA_COORDS));
			GL_CHECK(glVertexAttribPointer(SA_COORDS, 3, GL_FLOAT, GL_FALSE, 0, 0));
			break;
		}
		case VF_XYZ_NORMAL_FLOAT_32:
		{
			GL_CHECK(glEnableVertexAttribArray(SA_NORMAL));
			GL_CHECK(glVertexAttribPointer(SA_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, 0));
			break;
		}
		case VF_XYZ_UV_XYZ_NORMAL_FLOAT_32:
		{
			break;
		}
		default:
		{
			ce_assert(0, "Vertex format unknown");
			break;
		}
	}
}

//-----------------------------------------------------------------------------
void GLRenderer::draw_triangles(IndexBufferId id) const
{
	ce_assert(m_index_buffers_id_table.has(id), "Index buffer does not exist");

	const IndexBuffer& index_buffer = m_index_buffers[id.index];

	GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer.gl_object));

	GL_CHECK(glDrawElements(GL_TRIANGLES, index_buffer.index_count, GL_UNSIGNED_SHORT, 0));
}

//-----------------------------------------------------------------------------
// void GLRenderer::bind_render_buffer(RenderBufferId id) const
// {
// 	ce_assert(m_render_buffers_id_table.has(id), "Render buffer does not exist");

// 	const GLRenderBuffer& render_buffer = m_render_buffers[id.index];
// }

//-----------------------------------------------------------------------------
void GLRenderer::draw_lines(const float* vertices, const float* colors, uint32_t count)
{
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
	GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

	GL_CHECK(glEnableClientState(GL_VERTEX_ARRAY));
	GL_CHECK(glEnableClientState(GL_COLOR_ARRAY));

	GL_CHECK(glVertexPointer(3, GL_FLOAT, 0, vertices));
	GL_CHECK(glColorPointer(4, GL_FLOAT, 0, colors));

	GL_CHECK(glDrawArrays(GL_LINES, 0, count));

	GL_CHECK(glDisableClientState(GL_COLOR_ARRAY));
	GL_CHECK(glDisableClientState(GL_VERTEX_ARRAY));
}

//-----------------------------------------------------------------------------
void GLRenderer::load_default_shaders()
{
	ResourceManager* resman = device()->resource_manager();

	// Load default vertex/pixel shaders
	m_default_vertex_shader = resman->load("default/default.vs");
	m_default_pixel_shader = resman->load("default/default.ps");

	// Wait for loading
	resman->flush();

	// Obtain resource data
	VertexShaderResource* vs = (VertexShaderResource*)resman->data(m_default_vertex_shader);
	PixelShaderResource* ps = (PixelShaderResource*)resman->data(m_default_pixel_shader);

	// Create and bind the default program
	m_default_gpu_program = create_gpu_program(vs->vertex_shader(), ps->pixel_shader());
}

//-----------------------------------------------------------------------------
void GLRenderer::unload_default_shaders()
{
	ResourceManager* resman = device()->resource_manager();

	destroy_gpu_program(m_default_gpu_program);

	resman->unload(m_default_pixel_shader);
	resman->unload(m_default_vertex_shader);
}

//-----------------------------------------------------------------------------
void GLRenderer::reload_default_shaders()
{
	ResourceManager* resman = device()->resource_manager();

	resman->reload(m_default_vertex_shader);
	resman->reload(m_default_pixel_shader);

	// Destroy old gpu program
	destroy_gpu_program(m_default_gpu_program);

	// Obtain resource data
	VertexShaderResource* vs = (VertexShaderResource*)resman->data(m_default_vertex_shader);
	PixelShaderResource* ps = (PixelShaderResource*)resman->data(m_default_pixel_shader);

	// Create and bind the new default program
	m_default_gpu_program = create_gpu_program(vs->vertex_shader(), ps->pixel_shader());
}

//-----------------------------------------------------------------------------
bool GLRenderer::activate_texture_unit(uint32_t unit)
{
	if (unit >= (uint32_t) m_max_texture_units)
	{
		return false;
	}

	GL_CHECK(glActiveTexture(GL_TEXTURE0 + unit));
	m_active_texture_unit = unit;

	return true;
}

//-----------------------------------------------------------------------------
GLint GLRenderer::find_gpu_program_uniform(GLuint program, const char* name) const
{
	GLint uniform = GL_CHECK(glGetUniformLocation(program, name));

	ce_assert(uniform != -1, "Uniform does not exist");

	return uniform;
}

//-----------------------------------------------------------------------------
Renderer* Renderer::create()
{
	return new GLRenderer;
}

//-----------------------------------------------------------------------------
void Renderer::destroy(Renderer* renderer)
{
	delete renderer;
}

} // namespace crown

