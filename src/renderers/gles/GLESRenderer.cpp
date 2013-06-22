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

#include <GLES2/gl2.h>
#include <algorithm>

#include "Assert.h"
#include "Types.h"
#include "GLESRenderer.h"
#include "GLESUtils.h"
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
		do { GLenum error; CE_ASSERT((error = glGetError()) == GL_NO_ERROR,\
				"OpenGL error: %s", gl_error_to_string(error)); } while (0)
#else
	#define GL_CHECK(function)\
		function;
#endif
		
//-----------------------------------------------------------------------------
GLESRenderer::GLESRenderer() :
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
GLESRenderer::~GLESRenderer()
{
}

//-----------------------------------------------------------------------------
void GLESRenderer::init()
{
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_max_texture_size);
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &m_max_texture_units);
	//glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &m_max_vertex_indices);
	//glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &m_max_vertex_vertices);


	glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, &m_min_max_point_size[0]);
	//glGetFloatv(GL_LINE_WIDTH_RANGE, &m_min_max_line_width[0]);

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

	glDisable(GL_TEXTURE_2D);

	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);

	// Set the default framebuffer clear color
	glClearColor(0.5f, 0.5f, 0.5f, 0.5f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearDepthf(1.0);

	// Enable scissor test
	glEnable(GL_SCISSOR_TEST);

	// Disable dithering
	glDisable(GL_DITHER);

	Log::i("OpenGL Renderer initialized.");

	load_default_shaders();
}

//-----------------------------------------------------------------------------
void GLESRenderer::shutdown()
{
	unload_default_shaders();
}

//-----------------------------------------------------------------------------
VertexBufferId GLESRenderer::create_vertex_buffer(size_t count, VertexFormat format, const void* vertices)
{
	const VertexBufferId id = m_vertex_buffers_id_table.create();

	VertexBuffer& buffer = m_vertex_buffers[id.index];

	glGenBuffers(1, &buffer.gl_object);

	glBindBuffer(GL_ARRAY_BUFFER, buffer.gl_object);
	glBufferData(GL_ARRAY_BUFFER, count * Vertex::bytes_per_vertex(format), vertices, GL_STATIC_DRAW);

	buffer.count = count;
	buffer.format = format;

	return id;
}

//-----------------------------------------------------------------------------
VertexBufferId GLESRenderer::create_dynamic_vertex_buffer(size_t count, VertexFormat format, const void* vertices)
{
	const VertexBufferId id = m_vertex_buffers_id_table.create();

	VertexBuffer& buffer = m_vertex_buffers[id.index];

	glGenBuffers(1, &buffer.gl_object);

	glBindBuffer(GL_ARRAY_BUFFER, buffer.gl_object);
	glBufferData(GL_ARRAY_BUFFER, count * Vertex::bytes_per_vertex(format), vertices, GL_STREAM_DRAW);

	buffer.count = count;
	buffer.format = format;

	return id;
}

//-----------------------------------------------------------------------------
void GLESRenderer::update_vertex_buffer(VertexBufferId id, size_t offset, size_t count, const void* vertices)
{
	CE_ASSERT(m_vertex_buffers_id_table.has(id), "VertexBuffers table does not have vertex buffer %d", id);

	VertexBuffer& buffer = m_vertex_buffers[id.index];

	glBindBuffer(GL_ARRAY_BUFFER, buffer.gl_object);
	glBufferSubData(GL_ARRAY_BUFFER, offset * Vertex::bytes_per_vertex(buffer.format),
					count * Vertex::bytes_per_vertex(buffer.format), vertices);
}

//-----------------------------------------------------------------------------
void GLESRenderer::destroy_vertex_buffer(VertexBufferId id)
{
	CE_ASSERT(m_vertex_buffers_id_table.has(id), "VertexBuffers table does not have vertex buffer %d", id);

	VertexBuffer& buffer = m_vertex_buffers[id.index];

	glDeleteBuffers(1, &buffer.gl_object);

	m_vertex_buffers_id_table.destroy(id);
}

//-----------------------------------------------------------------------------
IndexBufferId GLESRenderer::create_index_buffer(size_t count, const void* indices)
{
	const IndexBufferId id = m_index_buffers_id_table.create();

	IndexBuffer& buffer = m_index_buffers[id.index];

	glGenBuffers(1, &buffer.gl_object);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.gl_object);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLushort), indices, GL_STATIC_DRAW);

	buffer.index_count = count;

	return id;
}

//-----------------------------------------------------------------------------
void GLESRenderer::destroy_index_buffer(IndexBufferId id)
{
	CE_ASSERT(m_index_buffers_id_table.has(id), "IndexBuffers table does not have index buffer %d", id);

	IndexBuffer& buffer = m_index_buffers[id.index];

	glDeleteBuffers(1, &buffer.gl_object);

	m_index_buffers_id_table.destroy(id);
}

//-----------------------------------------------------------------------------
TextureId GLESRenderer::create_texture(uint32_t width, uint32_t height, PixelFormat format, const void* data)
{
	const TextureId id = m_textures_id_table.create();

	Texture& gl_texture = m_textures[id.index];

	glGenTextures(1, &gl_texture.gl_object);

	glBindTexture(GL_TEXTURE_2D, gl_texture.gl_object);

	//glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

	// FIXME
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
				 GLES::pixel_format(format), GL_UNSIGNED_BYTE, data);

	gl_texture.format = format;

	return id;
}

//-----------------------------------------------------------------------------
void GLESRenderer::update_texture(TextureId id, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void* data)
{
	CE_ASSERT(m_textures_id_table.has(id), "Textures table does not have texture %d", id);

	Texture& gl_texture = m_textures[id.index];

	glBindTexture(GL_TEXTURE_2D, gl_texture.gl_object);

	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, GLES::pixel_format(gl_texture.format),
					GL_UNSIGNED_BYTE, data);
}

//-----------------------------------------------------------------------------
void GLESRenderer::destroy_texture(TextureId id)
{
	CE_ASSERT(m_textures_id_table.has(id), "Textures table does not have texture %d", id);

	Texture& gl_texture = m_textures[id.index];

	glDeleteTextures(1, &gl_texture.gl_object);
}

//-----------------------------------------------------------------------------
VertexShaderId GLESRenderer::create_vertex_shader(const char* program)
{
	CE_ASSERT(program != NULL, "Vertex shader can not be null");

	const VertexShaderId& id = m_vertex_shaders_id_table.create();

	VertexShader& gl_shader = m_vertex_shaders[id.index];

	gl_shader.gl_object = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(gl_shader.gl_object, 1, &program, NULL);

	glCompileShader(gl_shader.gl_object);

	GLint success;
	glGetShaderiv(gl_shader.gl_object, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		GLchar info_log[256];

		glGetShaderInfoLog(gl_shader.gl_object, 256, NULL, info_log);

		Log::e("Vertex shader compilation failed.");
		Log::e("Log: %s", info_log);
		CE_ASSERT(0, "");
	}

	return id;
}

//-----------------------------------------------------------------------------
void GLESRenderer::destroy_vertex_shader(VertexShaderId id)
{
	CE_ASSERT(m_vertex_shaders_id_table.has(id), "Vertex shaders table does not have vertex shader %d", id);

	VertexShader& gl_shader = m_vertex_shaders[id.index];

	glDeleteShader(gl_shader.gl_object);
}

//-----------------------------------------------------------------------------
PixelShaderId GLESRenderer::create_pixel_shader(const char* program)
{
	CE_ASSERT(program != NULL, "Pixel Shader can not be null");

	const PixelShaderId& id = m_pixel_shaders_id_table.create();

	PixelShader& gl_shader = m_pixel_shaders[id.index];

	gl_shader.gl_object = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(gl_shader.gl_object, 1, &program, NULL);

	glCompileShader(gl_shader.gl_object);

	GLint success;
	glGetShaderiv(gl_shader.gl_object, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		GLchar info_log[256];

		glGetShaderInfoLog(gl_shader.gl_object, 256, NULL, info_log);

		Log::e("Pixel shader compilation failed.");
		Log::e("Log: %s", info_log);
		CE_ASSERT(0, "");
	}

	return id;	
}

//-----------------------------------------------------------------------------
void GLESRenderer::destroy_pixel_shader(PixelShaderId id)
{
	CE_ASSERT(m_pixel_shaders_id_table.has(id),  "Pixel shaders table does not have pixel shader %d", id);

	PixelShader& gl_shader = m_pixel_shaders[id.index];

	glDeleteShader(gl_shader.gl_object);	
}

//-----------------------------------------------------------------------------
GPUProgramId GLESRenderer::create_gpu_program(VertexShaderId vs, PixelShaderId ps)
{
	CE_ASSERT(m_vertex_shaders_id_table.has(vs), "Vertex shaders table does not have vertex shader %d", vs);
	CE_ASSERT(m_pixel_shaders_id_table.has(ps), "Pixel shaders table does not have pixel shader %d", ps);

	const GPUProgramId id = m_gpu_programs_id_table.create();

	GPUProgram& gl_program = m_gpu_programs[id.index];

	gl_program.gl_object = glCreateProgram();

	glAttachShader(gl_program.gl_object, m_vertex_shaders[id.index].gl_object);
	glAttachShader(gl_program.gl_object, m_pixel_shaders[id.index].gl_object);

	glBindAttribLocation(gl_program.gl_object, SA_VERTEX, "vertex");
	glBindAttribLocation(gl_program.gl_object, SA_COORDS, "coords");
	glBindAttribLocation(gl_program.gl_object, SA_NORMAL, "normal");

	glLinkProgram(gl_program.gl_object);

	GLint success;
	glGetProgramiv(gl_program.gl_object, GL_LINK_STATUS, &success);

	if (!success)
	{
		GLchar info_log[256];
		glGetProgramInfoLog(gl_program.gl_object, 256, NULL, info_log);
		Log::e("GPU program compilation failed.\n");
		Log::e("Log: %s", info_log);
		CE_ASSERT(0, "");
	}

	return id;
}

//-----------------------------------------------------------------------------
void GLESRenderer::destroy_gpu_program(GPUProgramId id)
{
	CE_ASSERT(m_gpu_programs_id_table.has(id), "GPU programs table does not have gpu program %d", id);

	GPUProgram& gl_program = m_gpu_programs[id.index];

	glDeleteProgram(gl_program.gl_object);
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_gpu_program_bool_uniform(GPUProgramId id, const char* name, bool value)
{
	CE_ASSERT(m_gpu_programs_id_table.has(id), "GPU programs table does not have gpu program %d", id);

	const GLint uniform = find_gpu_program_uniform(m_gpu_programs[id.index].gl_object, name);

	glUniform1i(uniform, (GLint) value);
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_gpu_program_int_uniform(GPUProgramId id, const char* name, int value)
{
	CE_ASSERT(m_gpu_programs_id_table.has(id), "GPU programs table does not have gpu program %d", id);

	const GLint uniform = find_gpu_program_uniform(m_gpu_programs[id.index].gl_object, name);

	glUniform1i(uniform, (GLint) value);
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_gpu_program_vec2_uniform(GPUProgramId id, const char* name, const Vec2& value)
{
	CE_ASSERT(m_gpu_programs_id_table.has(id), "GPU programs table does not have gpu program %d", id);

	const GLint uniform = find_gpu_program_uniform(m_gpu_programs[id.index].gl_object, name);

	glUniform2fv(uniform, 1, value.to_float_ptr());
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_gpu_program_vec3_uniform(GPUProgramId id, const char* name, const Vec3& value)
{
	CE_ASSERT(m_gpu_programs_id_table.has(id), "GPU programs table does not have gpu program %d", id);

	const GLint uniform = find_gpu_program_uniform(m_gpu_programs[id.index].gl_object, name);

	glUniform3fv(uniform, 1, value.to_float_ptr());
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_gpu_program_vec4_uniform(GPUProgramId id, const char* name, const Vec4& value)
{
	CE_ASSERT(m_gpu_programs_id_table.has(id), "GPU programs table does not have gpu program %d", id);

	const GLint uniform = find_gpu_program_uniform(m_gpu_programs[id.index].gl_object, name);

	glUniform4fv(uniform, 1, value.to_float_ptr());
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_gpu_porgram_mat3_uniform(GPUProgramId id, const char* name, const Mat3& value)
{
	CE_ASSERT(m_gpu_programs_id_table.has(id), "GPU programs table does not have gpu program %d", id);

	const GLint uniform = find_gpu_program_uniform(m_gpu_programs[id.index].gl_object, name);

	glUniformMatrix3fv(uniform, 1, GL_FALSE, value.to_float_ptr());
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_gpu_program_mat4_uniform(GPUProgramId id, const char* name, const Mat4& value)
{
	CE_ASSERT(m_gpu_programs_id_table.has(id), "GPU programs table does not have gpu program %d", id);

	const GLint uniform = find_gpu_program_uniform(m_gpu_programs[id.index].gl_object, name);

	glUniformMatrix4fv(uniform, 1, GL_FALSE, value.to_float_ptr());
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_gpu_program_sampler_uniform(GPUProgramId id, const char* name, uint32_t value)
{
	CE_ASSERT(m_gpu_programs_id_table.has(id), "GPU programs table does not have gpu program %d", id);

	const GLint uniform = find_gpu_program_uniform(m_gpu_programs[id.index].gl_object, name);

	glUniform1i(uniform, (GLint) value);	
}

//-----------------------------------------------------------------------------
void GLESRenderer::bind_gpu_program(GPUProgramId id) const
{
	CE_ASSERT(m_gpu_programs_id_table.has(id), "GPU programs table does not have gpu program %d", id);

	const GPUProgram& gl_program = m_gpu_programs[id.index];

	glUseProgram(gl_program.gl_object);
}

//-----------------------------------------------------------------------------
// RenderBufferId GLESRenderer::create_render_buffer(uint32_t width, uint32_t height, PixelFormat format)
// {
// 	const RenderBufferId id = m_render_buffers_id_table.create();

// 	GLRenderBuffer& buffer = m_render_buffers[id.index];

// 	if (GLEW_EXT_framebuffer_object)
// 	{
// 		glGenFramebuffersEXT(1, &buffer.gl_frame_buffer);
// 		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, buffer.gl_frame_buffer);

// 		glGenRenderbuffersEXT(1, &buffer.gl_render_buffer);
// 		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, buffer.gl_render_buffer);

// 		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA8, width, height);

// 		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, buffer.gl_render_buffer);

// 		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
// 	}

// 	return id;
// }

//-----------------------------------------------------------------------------
// void GLESRenderer::destroy_render_buffer(RenderBufferId id)
// {
// 	GLRenderBuffer& buffer = m_render_buffers[id.index];

// 	if (GLEW_EXT_framebuffer_object)
// 	{
// 		glDeleteFramebuffersEXT(1, &buffer.gl_frame_buffer);
// 		glDeleteRenderbuffersEXT(1, &buffer.gl_render_buffer);
// 	}

// 	m_render_buffers_id_table.destroy(id);
// }

//-----------------------------------------------------------------------------
void GLESRenderer::set_clear_color(const Color4& color)
{
	glClearColor(color.r, color.g, color.b, color.a);
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_ambient_light(const Color4& color)
{
	m_ambient_light_color = color;
}

//-----------------------------------------------------------------------------
void GLESRenderer::bind_texture(uint32_t unit, TextureId texture)
{
	CE_ASSERT(m_textures_id_table.has(texture), "Textures table does not have texture %d", texture);

	if (!activate_texture_unit(unit))
	{
		return;
	}

	m_texture_unit_target[unit] = GL_TEXTURE_2D;
	m_texture_unit[unit] = m_textures[texture.index].gl_object;

	glEnable(m_texture_unit_target[unit]);
	glBindTexture(m_texture_unit_target[unit], m_texture_unit[unit]);
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_texturing(uint32_t unit, bool texturing)
{
	if (!activate_texture_unit(unit))
		return;

	if (texturing)
	{
		glEnable(m_texture_unit_target[unit]);
	}
	else
	{
		glDisable(m_texture_unit_target[unit]);
	}
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_texture_wrap(uint32_t unit, TextureWrap wrap)
{
	GLenum glWrap = GLES::texture_wrap(wrap);

	glTexParameteri(m_texture_unit_target[unit], GL_TEXTURE_WRAP_S, glWrap);
	glTexParameteri(m_texture_unit_target[unit], GL_TEXTURE_WRAP_T, glWrap);
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_texture_filter(uint32_t unit, TextureFilter filter)
{
	if (!activate_texture_unit(unit))
		return;

	GLint minFilter;
	GLint magFilter;

	GLES::texture_filter(filter, minFilter, magFilter);

	glTexParameteri(m_texture_unit_target[unit], GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(m_texture_unit_target[unit], GL_TEXTURE_MAG_FILTER, magFilter);
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_backface_culling(bool culling)
{
	if (culling)
	{
		glEnable(GL_CULL_FACE);
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_depth_test(bool test)
{
	if (test)
	{
		glEnable(GL_DEPTH_TEST);
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_depth_write(bool write)
{
	glDepthMask((GLboolean) write);
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_depth_func(CompareFunction func)
{
	GLenum glFunc = GLES::compare_function(func);

	glDepthFunc(glFunc);
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_blending(bool blending)
{
	if (blending)
	{
		glEnable(GL_BLEND);
	}
	else
	{
		glDisable(GL_BLEND);
	}
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_blending_params(BlendEquation equation, BlendFunction src, BlendFunction dst, const Color4& color)
{
	GLenum glEquation = GLES::blend_equation(equation);

	glBlendEquation(glEquation);

	GLenum glSrcFactor = GLES::blend_function(src);
	GLenum glDstFactor = GLES::blend_function(dst);

	glBlendFunc(glSrcFactor, glDstFactor);

	glBlendColor(color.r, color.g, color.b, color.a);
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_color_write(bool write)
{
	if (write)
	{
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	}
	else
	{
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	}
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_front_face(FrontFace face)
{
	GLenum glFace = GL_CCW;

	if (face == FF_CW)
	{
		glFace = GL_CW;
	}

	glFrontFace(glFace);
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_viewport_params(int32_t x, int32_t y, int32_t width, int32_t height)
{
	m_viewport[0] = x;
	m_viewport[1] = y;
	m_viewport[2] = width;
	m_viewport[3] = height;

	glViewport(x, y, width, height);
}

//-----------------------------------------------------------------------------
void GLESRenderer::get_viewport_params(int32_t& x, int32_t& y, int32_t& width, int32_t& height)
{
	x = m_viewport[0];
	y = m_viewport[1];
	width = m_viewport[2];
	height = m_viewport[3];
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_scissor(bool scissor)
{
	if (scissor)
	{
		glEnable(GL_SCISSOR_TEST);
	}
	else
	{
		glDisable(GL_SCISSOR_TEST);
	}
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_scissor_params(int32_t x, int32_t y, int32_t width, int32_t height)
{
	m_scissor[0] = x;
	m_scissor[1] = y;
	m_scissor[2] = width;
	m_scissor[3] = height;

	glScissor(x, y, width, height);
}

//-----------------------------------------------------------------------------
void GLESRenderer::get_scissor_params(int32_t& x, int32_t& y, int32_t& width, int32_t& height)
{
	x = m_scissor[0];
	y = m_scissor[1];
	width = m_scissor[2];
	height = m_scissor[3];
}

//-----------------------------------------------------------------------------
void GLESRenderer::frame()
{
	// Clear frame/depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Bind the default gpu program
	bind_gpu_program(m_default_gpu_program);

	set_gpu_program_mat4_uniform(m_default_gpu_program, "mvp_matrix", m_model_view_projection_matrix);
	set_gpu_program_vec3_uniform(m_default_gpu_program, "color", Vec3(0, 1, 0));

	glFinish();
	check_gl_errors();
}

//-----------------------------------------------------------------------------
Mat4 GLESRenderer::get_matrix(MatrixType type) const
{
	return m_matrix[type];
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_matrix(MatrixType type, const Mat4& matrix)
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
			CE_ASSERT(0, "");
		}
	}
}

//-----------------------------------------------------------------------------
void GLESRenderer::bind_vertex_buffer(VertexBufferId vb) const
{
	CE_ASSERT(m_vertex_buffers_id_table.has(vb), "Vertex buffers table does not have vertex buffer %d", vb);

	const VertexBuffer& vertex_buffer = m_vertex_buffers[vb.index];

	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer.gl_object));

	switch (vertex_buffer.format)
	{
		case VF_XY_FLOAT_32:
		{
			glEnableVertexAttribArray(SA_VERTEX);
			glVertexAttribPointer(SA_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, 0);
			break;
		}
		case VF_XYZ_FLOAT_32:
		{
			glEnableVertexAttribArray(SA_VERTEX);
			glVertexAttribPointer(SA_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
			break;
		}
		case VF_UV_FLOAT_32:
		{
			glEnableVertexAttribArray(SA_COORDS);
			glVertexAttribPointer(SA_COORDS, 2, GL_FLOAT, GL_FALSE, 0, 0);
			break;
		}
		case VF_UVT_FLOAT_32:
		{
			glEnableVertexAttribArray(SA_COORDS);
			glVertexAttribPointer(SA_COORDS, 3, GL_FLOAT, GL_FALSE, 0, 0);
			break;
		}
		case VF_XYZ_NORMAL_FLOAT_32:
		{
			glEnableVertexAttribArray(SA_NORMAL);
			glVertexAttribPointer(SA_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, 0);
			break;
		}
		case VF_XYZ_UV_XYZ_NORMAL_FLOAT_32:
		{
			break;
		}
		default:
		{
			CE_ASSERT(0, "");
			break;
		}
	}
}

//-----------------------------------------------------------------------------
void GLESRenderer::draw_triangles(IndexBufferId id) const
{
	CE_ASSERT(m_index_buffers_id_table.has(id), "Index buffers table does not have index buffer %d", id);

	const IndexBuffer& index_buffer = m_index_buffers[id.index];

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer.gl_object);

	glDrawElements(GL_TRIANGLES, index_buffer.index_count, GL_UNSIGNED_SHORT, 0);
}

//-----------------------------------------------------------------------------
// void GLESRenderer::bind_render_buffer(RenderBufferId id) const
// {
// 	CE_ASSERT(m_render_buffers_id_table.has(id));

// 	const GLRenderBuffer& render_buffer = m_render_buffers[id.index];
// }

//-----------------------------------------------------------------------------
void GLESRenderer::draw_lines(const float* vertices, const float* colors, uint32_t count)
{
	// glBindBuffer(GL_ARRAY_BUFFER, 0);
	// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// glEnableClientState(GL_VERTEX_ARRAY);
	// glEnableClientState(GL_COLOR_ARRAY);

	// glVertexPointer(3, GL_FLOAT, 0, vertices);
	// glColorPointer(4, GL_FLOAT, 0, colors);

	// glDrawArrays(GL_LINES, 0, count);

	// glDisableClientState(GL_COLOR_ARRAY);
	// glDisableClientState(GL_VERTEX_ARRAY);
}

//-----------------------------------------------------------------------------
void GLESRenderer::load_default_shaders()
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
void GLESRenderer::unload_default_shaders()
{
	ResourceManager* resman = device()->resource_manager();

	destroy_gpu_program(m_default_gpu_program);

	resman->unload(m_default_pixel_shader);
	resman->unload(m_default_vertex_shader);
}

//-----------------------------------------------------------------------------
void GLESRenderer::reload_default_shaders()
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
bool GLESRenderer::activate_texture_unit(uint32_t unit)
{
	if (unit >= (uint32_t) m_max_texture_units)
	{
		return false;
	}

	glActiveTexture(GL_TEXTURE0 + unit);
	m_active_texture_unit = unit;

	return true;
}

//-----------------------------------------------------------------------------
GLint GLESRenderer::find_gpu_program_uniform(GLuint program, const char* name) const
{
	GLint uniform = glGetUniformLocation(program, name);

	CE_ASSERT(uniform != -1, "Uniform not found in GPU program %d", program);

	return uniform;
}

//-----------------------------------------------------------------------------
void GLESRenderer::check_gl_errors() const
{
	GLenum error;

	while ((error = glGetError()))
	{
		switch (error)
		{
			case GL_INVALID_ENUM:
				Log::e("GLRenderer: GL_INVALID_ENUM");
				break;
			case GL_INVALID_VALUE:
				Log::e("GLRenderer: GL_INVALID_VALUE");
				break;
			case GL_INVALID_OPERATION:
				Log::e("GLRenderer: GL_INVALID_OPERATION");
				break;
			case GL_OUT_OF_MEMORY:
				Log::e("GLRenderer: GL_OUT_OF_MEMORY");
				break;
		}
	}
}

} // namespace crown

