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

#include "Config.h"

#include <GL/glew.h>
#include "Assert.h"
#include <algorithm>

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
#define gl_check(function)\
	function;\
	do { GLenum error; ce_assert((error = glGetError()) == GL_NO_ERROR,\
			"OpenGL error: %s", gl_error_to_string(error)); } while (0)

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
	GLenum err = glewInit();
	ce_assert(err == GLEW_OK, "Failed to initialize GLEW");

	gl_check(glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_max_texture_size));
	gl_check(glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &m_max_texture_units));
	gl_check(glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &m_max_vertex_indices));
	gl_check(glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &m_max_vertex_vertices));

	// Check for anisotropic filter support
	if (GLEW_EXT_texture_filter_anisotropic)
	{
		gl_check(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &m_max_anisotropy));
	}

	gl_check(glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, &m_min_max_point_size[0]));
	gl_check(glGetFloatv(GL_LINE_WIDTH_RANGE, &m_min_max_line_width[0]));

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

	gl_check(glDisable(GL_TEXTURE_2D));

	gl_check(glDisable(GL_BLEND));
	gl_check(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	gl_check(glBlendEquation(GL_FUNC_ADD));

	gl_check(glFrontFace(GL_CCW));
	gl_check(glEnable(GL_CULL_FACE));
	gl_check(glShadeModel(GL_SMOOTH));

	// Set the default framebuffer clear color
	gl_check(glClearColor(0.5f, 0.5f, 0.5f, 0.5f));

	// Enable depth test
	gl_check(glEnable(GL_DEPTH_TEST));
	gl_check(glDepthFunc(GL_LEQUAL));
	gl_check(glClearDepth(1.0));

	// Enable scissor test
	gl_check(glEnable(GL_SCISSOR_TEST));

	// Disable dithering
	gl_check(glDisable(GL_DITHER));

	// Point sprites enabled by default
	gl_check(glEnable(GL_POINT_SPRITE));
	gl_check(glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE));

	Log::i("OpenGL Renderer initialized.");

	load_default_shaders();
}

//-----------------------------------------------------------------------------
void GLRenderer::shutdown()
{
	unload_default_shaders();
}

//-----------------------------------------------------------------------------
VertexBufferId GLRenderer::create_vertex_buffer(size_t count, VertexFormat format, const void* vertices)
{
	const VertexBufferId id = m_vertex_buffers_id_table.create();

	VertexBuffer& buffer = m_vertex_buffers[id.index];

	gl_check(glGenBuffers(1, &buffer.gl_object));

	gl_check(glBindBuffer(GL_ARRAY_BUFFER, buffer.gl_object));
	gl_check(glBufferData(GL_ARRAY_BUFFER, count * Vertex::bytes_per_vertex(format), vertices, GL_STATIC_DRAW));

	buffer.count = count;
	buffer.format = format;

	return id;
}

//-----------------------------------------------------------------------------
VertexBufferId GLRenderer::create_dynamic_vertex_buffer(size_t count, VertexFormat format, const void* vertices)
{
	const VertexBufferId id = m_vertex_buffers_id_table.create();

	VertexBuffer& buffer = m_vertex_buffers[id.index];

	gl_check(glGenBuffers(1, &buffer.gl_object));

	gl_check(glBindBuffer(GL_ARRAY_BUFFER, buffer.gl_object));
	gl_check(glBufferData(GL_ARRAY_BUFFER, count * Vertex::bytes_per_vertex(format), vertices, GL_STREAM_DRAW));

	buffer.count = count;
	buffer.format = format;

	return id;
}

//-----------------------------------------------------------------------------
void GLRenderer::update_vertex_buffer(VertexBufferId id, size_t offset, size_t count, const void* vertices)
{
	ce_assert(m_vertex_buffers_id_table.has(id), "Vertex buffer does not exist");

	VertexBuffer& buffer = m_vertex_buffers[id.index];

	gl_check(glBindBuffer(GL_ARRAY_BUFFER, buffer.gl_object));
	gl_check(glBufferSubData(GL_ARRAY_BUFFER, offset * Vertex::bytes_per_vertex(buffer.format),
					count * Vertex::bytes_per_vertex(buffer.format), vertices));
}

//-----------------------------------------------------------------------------
void GLRenderer::destroy_vertex_buffer(VertexBufferId id)
{
	ce_assert(m_vertex_buffers_id_table.has(id), "Vertex buffer does not exist");

	VertexBuffer& buffer = m_vertex_buffers[id.index];

	gl_check(glDeleteBuffers(1, &buffer.gl_object));

	m_vertex_buffers_id_table.destroy(id);
}

//-----------------------------------------------------------------------------
IndexBufferId GLRenderer::create_index_buffer(size_t count, const void* indices)
{
	const IndexBufferId id = m_index_buffers_id_table.create();

	IndexBuffer& buffer = m_index_buffers[id.index];

	gl_check(glGenBuffers(1, &buffer.gl_object));

	gl_check(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.gl_object));
	gl_check(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLushort), indices, GL_STATIC_DRAW));

	buffer.index_count = count;

	return id;
}

//-----------------------------------------------------------------------------
void GLRenderer::destroy_index_buffer(IndexBufferId id)
{
	ce_assert(m_index_buffers_id_table.has(id), "Index buffer does not exist");

	IndexBuffer& buffer = m_index_buffers[id.index];

	gl_check(glDeleteBuffers(1, &buffer.gl_object));

	m_index_buffers_id_table.destroy(id);
}

//-----------------------------------------------------------------------------
TextureId GLRenderer::create_texture(uint32_t width, uint32_t height, PixelFormat format, const void* data)
{
	const TextureId id = m_textures_id_table.create();

	Texture& gl_texture = m_textures[id.index];

	gl_check(glGenTextures(1, &gl_texture.gl_object));

	gl_check(glBindTexture(GL_TEXTURE_2D, gl_texture.gl_object));

	gl_check(glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE));

	// FIXME
	gl_check(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
				 GL::pixel_format(format), GL_UNSIGNED_BYTE, data));

	gl_texture.format = format;

	return id;
}

//-----------------------------------------------------------------------------
void GLRenderer::update_texture(TextureId id, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void* data)
{
	ce_assert(m_textures_id_table.has(id), "Texture does not exist");

	Texture& gl_texture = m_textures[id.index];

	gl_check(glBindTexture(GL_TEXTURE_2D, gl_texture.gl_object));

	gl_check(glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, GL::pixel_format(gl_texture.format),
					GL_UNSIGNED_BYTE, data));
}

//-----------------------------------------------------------------------------
void GLRenderer::destroy_texture(TextureId id)
{
	ce_assert(m_textures_id_table.has(id), "Texture does not exist");

	Texture& gl_texture = m_textures[id.index];

	gl_check(glDeleteTextures(1, &gl_texture.gl_object));
}

//-----------------------------------------------------------------------------
VertexShaderId GLRenderer::create_vertex_shader(const char* program)
{
	ce_assert(program != NULL, "Program must be != NULL");

	const VertexShaderId& id = m_vertex_shaders_id_table.create();

	VertexShader& gl_shader = m_vertex_shaders[id.index];

	gl_shader.gl_object = gl_check(glCreateShader(GL_VERTEX_SHADER));

	gl_check(glShaderSource(gl_shader.gl_object, 1, &program, NULL));

	gl_check(glCompileShader(gl_shader.gl_object));

	GLint success;
	gl_check(glGetShaderiv(gl_shader.gl_object, GL_COMPILE_STATUS, &success));

	if (!success)
	{
		GLchar info_log[256];

		gl_check(glGetShaderInfoLog(gl_shader.gl_object, 256, NULL, info_log));

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

	gl_check(glDeleteShader(gl_shader.gl_object));
}

//-----------------------------------------------------------------------------
PixelShaderId GLRenderer::create_pixel_shader(const char* program)
{
	ce_assert(program != NULL, "Program must be != NULL");

	const PixelShaderId& id = m_pixel_shaders_id_table.create();

	PixelShader& gl_shader = m_pixel_shaders[id.index];

	gl_shader.gl_object = gl_check(glCreateShader(GL_FRAGMENT_SHADER));

	gl_check(glShaderSource(gl_shader.gl_object, 1, &program, NULL));

	gl_check(glCompileShader(gl_shader.gl_object));

	GLint success;
	gl_check(glGetShaderiv(gl_shader.gl_object, GL_COMPILE_STATUS, &success));

	if (!success)
	{
		GLchar info_log[256];

		gl_check(glGetShaderInfoLog(gl_shader.gl_object, 256, NULL, info_log));

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

	gl_check(glDeleteShader(gl_shader.gl_object));	
}

//-----------------------------------------------------------------------------
GPUProgramId GLRenderer::create_gpu_program(VertexShaderId vs, PixelShaderId ps)
{
	ce_assert(m_vertex_shaders_id_table.has(vs), "Vertex shader does not exist");
	ce_assert(m_pixel_shaders_id_table.has(ps), "Pixel shader does not exist");

	const GPUProgramId id = m_gpu_programs_id_table.create();

	GPUProgram& gl_program = m_gpu_programs[id.index];

	gl_program.gl_object = gl_check(glCreateProgram());

	gl_check(glAttachShader(gl_program.gl_object, m_vertex_shaders[id.index].gl_object));
	gl_check(glAttachShader(gl_program.gl_object, m_pixel_shaders[id.index].gl_object));

	gl_check(glBindAttribLocation(gl_program.gl_object, SA_VERTEX, "vertex"));
	gl_check(glBindAttribLocation(gl_program.gl_object, SA_COORDS, "coords"));
	gl_check(glBindAttribLocation(gl_program.gl_object, SA_NORMAL, "normal"));

	gl_check(glLinkProgram(gl_program.gl_object));

	GLint success;
	gl_check(glGetProgramiv(gl_program.gl_object, GL_LINK_STATUS, &success));

	if (!success)
	{
		GLchar info_log[256];
		gl_check(glGetProgramInfoLog(gl_program.gl_object, 256, NULL, info_log));
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

	gl_check(glDeleteProgram(gl_program.gl_object));
}

//-----------------------------------------------------------------------------
void GLRenderer::set_gpu_program_bool_uniform(GPUProgramId id, const char* name, bool value)
{
	ce_assert(m_gpu_programs_id_table.has(id), "GPU program does not exist");

	const GLint uniform = find_gpu_program_uniform(m_gpu_programs[id.index].gl_object, name);

	gl_check(glUniform1i(uniform, (GLint) value));
}

//-----------------------------------------------------------------------------
void GLRenderer::set_gpu_program_int_uniform(GPUProgramId id, const char* name, int value)
{
	ce_assert(m_gpu_programs_id_table.has(id), "GPU program does not exist");

	const GLint uniform = find_gpu_program_uniform(m_gpu_programs[id.index].gl_object, name);

	gl_check(glUniform1i(uniform, (GLint) value));
}

//-----------------------------------------------------------------------------
void GLRenderer::set_gpu_program_vec2_uniform(GPUProgramId id, const char* name, const Vec2& value)
{
	ce_assert(m_gpu_programs_id_table.has(id), "GPU program does not exist");

	const GLint uniform = find_gpu_program_uniform(m_gpu_programs[id.index].gl_object, name);

	gl_check(glUniform2fv(uniform, 1, value.to_float_ptr()));
}

//-----------------------------------------------------------------------------
void GLRenderer::set_gpu_program_vec3_uniform(GPUProgramId id, const char* name, const Vec3& value)
{
	ce_assert(m_gpu_programs_id_table.has(id), "GPU program does not exist");

	const GLint uniform = find_gpu_program_uniform(m_gpu_programs[id.index].gl_object, name);

	gl_check(glUniform3fv(uniform, 1, value.to_float_ptr()));
}

//-----------------------------------------------------------------------------
void GLRenderer::set_gpu_program_vec4_uniform(GPUProgramId id, const char* name, const Vec4& value)
{
	ce_assert(m_gpu_programs_id_table.has(id), "GPU program does not exist");

	const GLint uniform = find_gpu_program_uniform(m_gpu_programs[id.index].gl_object, name);

	gl_check(glUniform4fv(uniform, 1, value.to_float_ptr()));
}

//-----------------------------------------------------------------------------
void GLRenderer::set_gpu_porgram_mat3_uniform(GPUProgramId id, const char* name, const Mat3& value)
{
	ce_assert(m_gpu_programs_id_table.has(id), "GPU program does not exist");

	const GLint uniform = find_gpu_program_uniform(m_gpu_programs[id.index].gl_object, name);

	gl_check(glUniformMatrix3fv(uniform, 1, GL_FALSE, value.to_float_ptr()));
}

//-----------------------------------------------------------------------------
void GLRenderer::set_gpu_program_mat4_uniform(GPUProgramId id, const char* name, const Mat4& value)
{
	ce_assert(m_gpu_programs_id_table.has(id), "GPU program does not exist");

	const GLint uniform = find_gpu_program_uniform(m_gpu_programs[id.index].gl_object, name);

	gl_check(glUniformMatrix4fv(uniform, 1, GL_FALSE, value.to_float_ptr()));
}

//-----------------------------------------------------------------------------
void GLRenderer::set_gpu_program_sampler_uniform(GPUProgramId id, const char* name, uint32_t value)
{
	ce_assert(m_gpu_programs_id_table.has(id), "GPU program does not exist");

	const GLint uniform = find_gpu_program_uniform(m_gpu_programs[id.index].gl_object, name);

	gl_check(glUniform1i(uniform, (GLint) value));	
}

//-----------------------------------------------------------------------------
void GLRenderer::bind_gpu_program(GPUProgramId id) const
{
	ce_assert(m_gpu_programs_id_table.has(id), "GPU program does not exist");

	const GPUProgram& gl_program = m_gpu_programs[id.index];

	gl_check(glUseProgram(gl_program.gl_object));
}

//-----------------------------------------------------------------------------
// RenderBufferId GLRenderer::create_render_buffer(uint32_t width, uint32_t height, PixelFormat format)
// {
// 	const RenderBufferId id = m_render_buffers_id_table.create();

// 	GLRenderBuffer& buffer = m_render_buffers[id.index];

// 	if (GLEW_EXT_framebuffer_object)
// 	{
// 		gl_check(glGenFramebuffersEXT(1, &buffer.gl_check(gl_frame_buffer));
// 		gl_check(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, buffer.gl_check(gl_frame_buffer));

// 		gl_check(glGenRenderbuffersEXT(1, &buffer.gl_check(gl_render_buffer));
// 		gl_check(glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, buffer.gl_check(gl_render_buffer));

// 		gl_check(glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA8, width, height));

// 		gl_check(glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, buffer.gl_check(gl_render_buffer));

// 		gl_check(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0));
// 	}

// 	return id;
// }

//-----------------------------------------------------------------------------
// void GLRenderer::destroy_render_buffer(RenderBufferId id)
// {
// 	GLRenderBuffer& buffer = m_render_buffers[id.index];

// 	if (GLEW_EXT_framebuffer_object)
// 	{
// 		gl_check(glDeleteFramebuffersEXT(1, &buffer.gl_check(gl_frame_buffer));
// 		gl_check(glDeleteRenderbuffersEXT(1, &buffer.gl_check(gl_render_buffer));
// 	}

// 	m_render_buffers_id_table.destroy(id);
// }

//-----------------------------------------------------------------------------
void GLRenderer::set_clear_color(const Color4& color)
{
	gl_check(glClearColor(color.r, color.g, color.b, color.a));
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

	gl_check(glEnable(m_texture_unit_target[unit]));
	gl_check(glBindTexture(m_texture_unit_target[unit], m_texture_unit[unit]));
}

//-----------------------------------------------------------------------------
void GLRenderer::set_texturing(uint32_t unit, bool texturing)
{
	if (!activate_texture_unit(unit))
		return;

	if (texturing)
	{
		gl_check(glEnable(m_texture_unit_target[unit]));
	}
	else
	{
		gl_check(glDisable(m_texture_unit_target[unit]));
	}
}

//-----------------------------------------------------------------------------
void GLRenderer::set_texture_wrap(uint32_t unit, TextureWrap wrap)
{
	GLenum gl_wrap = GL::texture_wrap(wrap);

	gl_check(glTexParameteri(m_texture_unit_target[unit], GL_TEXTURE_WRAP_S, gl_wrap));
	gl_check(glTexParameteri(m_texture_unit_target[unit], GL_TEXTURE_WRAP_T, gl_wrap));
}

//-----------------------------------------------------------------------------
void GLRenderer::set_texture_filter(uint32_t unit, TextureFilter filter)
{
	if (!activate_texture_unit(unit))
		return;

	GLint min_filter;
	GLint mag_filter;

	GL::texture_filter(filter, min_filter, mag_filter);

	gl_check(glTexParameteri(m_texture_unit_target[unit], GL_TEXTURE_MIN_FILTER, min_filter));
	gl_check(glTexParameteri(m_texture_unit_target[unit], GL_TEXTURE_MAG_FILTER, mag_filter));
}

//-----------------------------------------------------------------------------
void GLRenderer::set_backface_culling(bool culling)
{
	if (culling)
	{
		gl_check(glEnable(GL_CULL_FACE));
	}
	else
	{
		gl_check(glDisable(GL_CULL_FACE));
	}
}

//-----------------------------------------------------------------------------
void GLRenderer::set_depth_test(bool test)
{
	if (test)
	{
		gl_check(glEnable(GL_DEPTH_TEST));
	}
	else
	{
		gl_check(glDisable(GL_DEPTH_TEST));
	}
}

//-----------------------------------------------------------------------------
void GLRenderer::set_depth_write(bool write)
{
	gl_check(glDepthMask((GLboolean) write));
}

//-----------------------------------------------------------------------------
void GLRenderer::set_depth_func(CompareFunction func)
{
	GLenum gl_func = GL::compare_function(func);

	gl_check(glDepthFunc(gl_func));
}

//-----------------------------------------------------------------------------
void GLRenderer::set_blending(bool blending)
{
	if (blending)
	{
		gl_check(glEnable(GL_BLEND));
	}
	else
	{
		gl_check(glDisable(GL_BLEND));
	}
}

//-----------------------------------------------------------------------------
void GLRenderer::set_blending_params(BlendEquation equation, BlendFunction src, BlendFunction dst, const Color4& color)
{
	GLenum gl_equation = GL::blend_equation(equation);

	gl_check(glBlendEquation(gl_equation));

	GLenum gl_src_factor = GL::blend_function(src);
	GLenum gl_dst_factor = GL::blend_function(dst);

	gl_check(glBlendFunc(gl_src_factor, gl_dst_factor));

	gl_check(glBlendColor(color.r, color.g, color.b, color.a));
}

//-----------------------------------------------------------------------------
void GLRenderer::set_color_write(bool write)
{
	if (write)
	{
		gl_check(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
	}
	else
	{
		gl_check(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));
	}
}

//-----------------------------------------------------------------------------
void GLRenderer::set_front_face(FrontFace face)
{
	const GLenum gl_face = (face == FF_CCW) ? GL_CCW : GL_CW;

	gl_check(glFrontFace(gl_face));
}

//-----------------------------------------------------------------------------
void GLRenderer::set_viewport_params(int32_t x, int32_t y, int32_t width, int32_t height)
{
	m_viewport[0] = x;
	m_viewport[1] = y;
	m_viewport[2] = width;
	m_viewport[3] = height;

	gl_check(glViewport(x, y, width, height));
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
		gl_check(glEnable(GL_SCISSOR_TEST));
	}
	else
	{
		gl_check(glDisable(GL_SCISSOR_TEST));
	}
}

//-----------------------------------------------------------------------------
void GLRenderer::set_scissor_params(int32_t x, int32_t y, int32_t width, int32_t height)
{
	m_scissor[0] = x;
	m_scissor[1] = y;
	m_scissor[2] = width;
	m_scissor[3] = height;

	gl_check(glScissor(x, y, width, height));
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
	gl_check(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	// Bind the default gpu program
	bind_gpu_program(m_default_gpu_program);

	set_gpu_program_mat4_uniform(m_default_gpu_program, "mvp_matrix", m_model_view_projection_matrix);
	set_gpu_program_vec3_uniform(m_default_gpu_program, "color", Vec3(0, 1, 0));

	gl_check(glFinish());
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

	gl_check(glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer.gl_object));

	switch (vertex_buffer.format)
	{
		case VF_XY_FLOAT_32:
		{
			gl_check(glEnableVertexAttribArray(SA_VERTEX));
			gl_check(glVertexAttribPointer(SA_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, 0));
			break;
		}
		case VF_XYZ_FLOAT_32:
		{
			gl_check(glEnableVertexAttribArray(SA_VERTEX));
			gl_check(glVertexAttribPointer(SA_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, 0));
			break;
		}
		case VF_UV_FLOAT_32:
		{
			gl_check(glEnableVertexAttribArray(SA_COORDS));
			gl_check(glVertexAttribPointer(SA_COORDS, 2, GL_FLOAT, GL_FALSE, 0, 0));
			break;
		}
		case VF_UVT_FLOAT_32:
		{
			gl_check(glEnableVertexAttribArray(SA_COORDS));
			gl_check(glVertexAttribPointer(SA_COORDS, 3, GL_FLOAT, GL_FALSE, 0, 0));
			break;
		}
		case VF_XYZ_NORMAL_FLOAT_32:
		{
			gl_check(glEnableVertexAttribArray(SA_NORMAL));
			gl_check(glVertexAttribPointer(SA_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, 0));
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

	gl_check(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer.gl_object));

	gl_check(glDrawElements(GL_TRIANGLES, index_buffer.index_count, GL_UNSIGNED_SHORT, 0));
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
	gl_check(glBindBuffer(GL_ARRAY_BUFFER, 0));
	gl_check(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

	gl_check(glEnableClientState(GL_VERTEX_ARRAY));
	gl_check(glEnableClientState(GL_COLOR_ARRAY));

	gl_check(glVertexPointer(3, GL_FLOAT, 0, vertices));
	gl_check(glColorPointer(4, GL_FLOAT, 0, colors));

	gl_check(glDrawArrays(GL_LINES, 0, count));

	gl_check(glDisableClientState(GL_COLOR_ARRAY));
	gl_check(glDisableClientState(GL_VERTEX_ARRAY));
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

	gl_check(glActiveTexture(GL_TEXTURE0 + unit));
	m_active_texture_unit = unit;

	return true;
}

//-----------------------------------------------------------------------------
GLint GLRenderer::find_gpu_program_uniform(GLuint program, const char* name) const
{
	GLint uniform = gl_check(glGetUniformLocation(program, name));

	ce_assert(uniform != -1, "Uniform does not exist");

	return uniform;
}

} // namespace crown

