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

#pragma once

#include <GL/glew.h>

#include "Config.h"
#include "Renderer.h"
#include "IdTable.h"
#include "Resource.h"
#include "GLContext.h"
#include "HeapAllocator.h"

namespace crown
{

extern const GLenum TEXTURE_MIN_FILTER_TABLE[];
extern const GLenum TEXTURE_MAG_FILTER_TABLE[];
extern const GLenum TEXTURE_WRAP_TABLE[];

enum ShaderAttrib
{
	ATTRIB_POSITION			= 0,
	ATTRIB_NORMAL			= 1,
	ATTRIB_COLOR			= 2,
	ATTRIB_TEX_COORD0		= 3,
	ATTRIB_TEX_COORD1		= 4,
	ATTRIB_TEX_COORD2		= 5,
	ATTRIB_TEX_COORD3		= 6,
	ATTRIB_COUNT
};

// Keep in sync with ShaderAttrib
const char* const SHADER_ATTRIB_NAMES[] =
{
	"a_position",
	"a_normal",
	"a_color",
	"a_tex_coord0",
	"a_tex_coord1",
	"a_tex_coord2",
	"a_tex_coord3"
};

enum ShaderUniform
{
	UNIFORM_VIEW					= 0,
	UNIFORM_MODEL					= 1,
	UNIFORM_MODEL_VIEW				= 2,
	UNIFORM_MODEL_VIEW_PROJECTION	= 3,
	UNIFORM_TIME_SINCE_START		= 4,
	UNIFORM_COUNT
};

const char* const SHADER_UNIFORM_NAMES[] =
{
	"u_view",
	"u_model",
	"u_model_view",
	"u_model_view_projection",
	"u_time_since_start"
};

static ShaderUniform name_to_stock_uniform(const char* uniform)
{
	for (uint8_t i = 0; i < UNIFORM_COUNT; i++)
	{
		if (string::strcmp(uniform, SHADER_UNIFORM_NAMES[i]) == 0)
		{
			return (ShaderUniform) i;
		}
	}

	return UNIFORM_COUNT;
}

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
#if defined(CROWN_DEBUG) || defined(CROWN_DEVELOPMENT)
	#define GL_CHECK(function)\
		function;\
		do { GLenum error; CE_ASSERT((error = glGetError()) == GL_NO_ERROR,\
				"OpenGL error: %s", gl_error_to_string(error)); } while (0)
#else
	#define GL_CHECK(function)\
		function;
#endif

//-----------------------------------------------------------------------------
struct VertexBuffer
{
	//-----------------------------------------------------------------------------
	void create(size_t count, VertexFormat format, const void* vertices)
	{
		GL_CHECK(glGenBuffers(1, &m_id));
		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_id));
		GL_CHECK(glBufferData(GL_ARRAY_BUFFER, count * Vertex::bytes_per_vertex(format), vertices, GL_STATIC_DRAW));
		// GL_STREAM_DRAW
		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));

		m_count = count;
		m_format = format;
	}

	//-----------------------------------------------------------------------------
	void update(size_t offset, size_t count, const void* vertices)
	{
		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_id));
		GL_CHECK(glBufferSubData(GL_ARRAY_BUFFER, offset * Vertex::bytes_per_vertex(m_format),
									count * Vertex::bytes_per_vertex(m_format), vertices));
		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	//-----------------------------------------------------------------------------
	void destroy()
	{
		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
		GL_CHECK(glDeleteBuffers(1, &m_id));
	}

public:

	GLuint			m_id;
	size_t			m_count;
	VertexFormat	m_format;
};

//-----------------------------------------------------------------------------
struct IndexBuffer
{
	//-----------------------------------------------------------------------------
	void create(size_t count, const void* indices)
	{
		GL_CHECK(glGenBuffers(1, &m_id));
		GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id));
		GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLushort), indices, GL_STATIC_DRAW));
		GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

		m_index_count = count;
	}

	//-----------------------------------------------------------------------------
	void destroy()
	{
		GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
		GL_CHECK(glDeleteBuffers(1, &m_id));
	}

public:

	GLuint		m_id;
	uint32_t	m_index_count;
};

//-----------------------------------------------------------------------------
struct Shader
{
	//-----------------------------------------------------------------------------
	void create(ShaderType type, const char* text)
	{
		m_id = GL_CHECK(glCreateShader(type == SHADER_VERTEX ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER));

		GL_CHECK(glShaderSource(m_id, 1, &text, NULL));
		GL_CHECK(glCompileShader(m_id));

		GLint success;
		GL_CHECK(glGetShaderiv(m_id, GL_COMPILE_STATUS, &success));

		if (!success)
		{
			GLchar info_log[2048];
			GL_CHECK(glGetShaderInfoLog(m_id, 2048, NULL, info_log));
			CE_ASSERT(false, "Shader compilation failed\n\n%s", info_log);
		}
	}

	//-----------------------------------------------------------------------------
	void destroy()
	{
		GL_CHECK(glDeleteShader(m_id));
	}

public:

	GLuint m_id;
};

//-----------------------------------------------------------------------------
struct Texture
{
	//-----------------------------------------------------------------------------
	void create(uint32_t width, uint32_t height, PixelFormat format, const void* data)
	{
		GL_CHECK(glGenTextures(1, &m_id));
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_id));
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE));

		// FIXME
		GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
					 			GL_RGBA, GL_UNSIGNED_BYTE, data));
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

		m_format = format;
	}

	//-----------------------------------------------------------------------------
	void update(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void* data)
	{
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_id));
		GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, GL_RGBA,
									GL_UNSIGNED_BYTE, data));
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
	}

	//-----------------------------------------------------------------------------
	void set_sampler_state(uint32_t flags)
	{
		GLenum min_filter = TEXTURE_MIN_FILTER_TABLE[(flags & TEXTURE_FILTER_MASK) >> TEXTURE_FILTER_SHIFT];
		GLenum mag_filter = TEXTURE_MIN_FILTER_TABLE[(flags & TEXTURE_FILTER_MASK) >> TEXTURE_FILTER_SHIFT];
		GLenum wrap = TEXTURE_WRAP_TABLE[(flags & TEXTURE_WRAP_MASK) >> TEXTURE_WRAP_SHIFT];

		GL_CHECK(glTexParameteri(m_target, GL_TEXTURE_WRAP_S, wrap));
		GL_CHECK(glTexParameteri(m_target, GL_TEXTURE_WRAP_T, wrap));

		GL_CHECK(glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, min_filter));
		GL_CHECK(glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, mag_filter));
	}

	//-----------------------------------------------------------------------------
	void destroy()
	{
		GL_CHECK(glBindTexture(m_target, 0));
		GL_CHECK(glDeleteTextures(1, &m_id));
	}

	//-----------------------------------------------------------------------------
	void commit(uint8_t unit, uint32_t flags)
	{
		GL_CHECK(glActiveTexture(GL_TEXTURE0 + unit));
		GL_CHECK(glEnable(m_target));
		GL_CHECK(glBindTexture(m_target, m_id));

		set_sampler_state(flags);
	}

public:

	GLuint			m_id;
	GLenum			m_target;      // Always GL_TEXTURE_2D
	uint32_t		m_width;
	uint32_t		m_height;
	PixelFormat		m_format;
};

//-----------------------------------------------------------------------------
struct GPUProgram
{
	//-----------------------------------------------------------------------------
	void create(const Shader& vertex, const Shader& pixel)
	{
		m_id = GL_CHECK(glCreateProgram());

		GL_CHECK(glAttachShader(m_id, vertex.m_id));
		GL_CHECK(glAttachShader(m_id, pixel.m_id));

		GL_CHECK(glBindAttribLocation(m_id, ATTRIB_POSITION, SHADER_ATTRIB_NAMES[ATTRIB_POSITION]));
		GL_CHECK(glBindAttribLocation(m_id, ATTRIB_NORMAL, SHADER_ATTRIB_NAMES[ATTRIB_NORMAL]));

		GL_CHECK(glLinkProgram(m_id));

		GLint success;
		GL_CHECK(glGetProgramiv(m_id, GL_LINK_STATUS, &success));

		if (!success)
		{
			GLchar info_log[2048];
			GL_CHECK(glGetProgramInfoLog(m_id, 2048, NULL, info_log));
			CE_ASSERT(false, "GPU program compilation failed:\n%s", info_log);
		}

		// Find active attribs/uniforms
		GLint num_active_attribs;
		GLint num_active_uniforms;
		GL_CHECK(glGetProgramiv(m_id, GL_ACTIVE_ATTRIBUTES, &num_active_attribs));
		GL_CHECK(glGetProgramiv(m_id, GL_ACTIVE_UNIFORMS, &num_active_uniforms));

		Log::d("Found %d active attribs", num_active_attribs);
		Log::d("Found %d active uniforms", num_active_uniforms);

		// Find active attribs/uniforms max length
		GLint max_attrib_length;
		GLint max_uniform_length;
		GL_CHECK(glGetProgramiv(m_id, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &max_attrib_length));
		GL_CHECK(glGetProgramiv(m_id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_uniform_length));

		for (GLint attrib = 0; attrib < num_active_attribs; attrib++)
		{
			GLint attrib_size;
			GLenum attrib_type;
			char attrib_name[1024];
			GL_CHECK(glGetActiveAttrib(m_id, attrib, max_attrib_length, NULL, &attrib_size, &attrib_type, attrib_name));

			Log::d("Attrib %d: name = '%s' location = '%d'", attrib, attrib_name,
					glGetAttribLocation(m_id, attrib_name));
		}

		for (GLint uniform = 0; uniform < num_active_uniforms; uniform++)
		{
			GLint uniform_size;
			GLenum uniform_type;
			char uniform_name[1024];
			GL_CHECK(glGetActiveUniform(m_id, uniform, max_uniform_length, NULL, &uniform_size, &uniform_type, uniform_name));

			GLint uniform_location = glGetUniformLocation(m_id, uniform_name);
			Log::d("Uniform %d: name = '%s' location = '%d'", uniform, uniform_name,
					uniform_location);

			ShaderUniform stock_uniform = name_to_stock_uniform(uniform_name);
			if (stock_uniform != UNIFORM_COUNT)
			{
				m_stock_uniforms[m_num_stock_uniforms] = stock_uniform;
				m_stock_uniform_locations[m_num_stock_uniforms] = uniform_location;
				m_num_stock_uniforms++;

				Log::d("Found stock uniform: %s", uniform_name);
			}
		}
	}

	//-----------------------------------------------------------------------------
	void destroy()
	{
		GL_CHECK(glUseProgram(0));
		GL_CHECK(glDeleteProgram(m_id));
	}

public:

	GLuint				m_id;
	uint8_t				m_num_stock_uniforms;
	ShaderUniform		m_stock_uniforms[UNIFORM_COUNT];
	GLint				m_stock_uniform_locations[UNIFORM_COUNT];
};

//-----------------------------------------------------------------------------
struct RenderTarget
{
	void create(uint16_t /*width*/, uint16_t /*height*/, RenderTargetFormat /*format*/)
	{
		// // Create and bind FBO
		// GL_CHECK(glGenFramebuffersEXT(1, &m_gl_fbo));
		// GL_CHECK(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_gl_fbo));

		// GLuint renderedTexture;
		// glGenTextures(1, &renderedTexture);
		 
		// // "Bind" the newly created texture : all future texture functions will modify this texture
		// glBindTexture(GL_TEXTURE_2D, renderedTexture);
		 
		// // Give an empty image to OpenGL ( the last "0" )
		// glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, 1024, 768, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);
		 
		// // Poor filtering. Needed !
		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


		// // Create color/depth attachments
		// switch (format)
		// {
		// 	case RTF_RGBA_8:
		// 	case RTF_D24:
		// 	{
		// 		if (format == RTF_RGBA_8)
		// 		{
		// 			GL_CHECK(glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
  //                      GL_COLOR_ATTACHMENT0_EXT,
  //                      GL_TEXTURE_2D,
  //                      renderedTexture,
  //                      0));
		// 			break;
		// 		}
		// 		else if (format == RTF_D24)
		// 		{
		// 			GL_CHECK(glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, width, height));
		// 			GL_CHECK(glFramebufferRenderbufferEXT(GL_DRAW_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, m_gl_rbo));
		// 		}

		// 		break;
		// 	}
		// 	default:
		// 	{
		// 		CE_ASSERT(false, "Oops, render target format not supported!");
		// 		break;
		// 	}
		// }

		// GLenum status = glCheckFramebufferStatusEXT(GL_DRAW_FRAMEBUFFER_EXT);
		// CE_ASSERT(status == GL_FRAMEBUFFER_COMPLETE_EXT, "Oops, framebuffer incomplete!");

		// GL_CHECK(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0));

		// m_width = width;
		// m_height = height;
		// m_format = format;
	}

	void destroy()
	{
		// GL_CHECK(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0));
		// GL_CHECK(glDeleteFramebuffersEXT(1, &m_gl_fbo));

		// GL_CHECK(glDeleteRenderbuffersEXT(1, &m_gl_rbo));
	}

	uint16_t m_width;
	uint16_t m_height;
	RenderTargetFormat m_format;
	GLuint m_gl_fbo;
	GLuint m_gl_rbo;
};

/// OpenGL renderer
class GLRenderer : public Renderer
{
public:

						GLRenderer();
						~GLRenderer();

	void				init();
	void				shutdown();

	// Vertex buffers
	VertexBufferId		create_vertex_buffer(size_t count, VertexFormat format, const void* vertices);
	void				update_vertex_buffer(VertexBufferId id, size_t offset, size_t count, const void* vertices);
	void				destroy_vertex_buffer(VertexBufferId id);

	// Index buffers
	IndexBufferId		create_index_buffer(size_t count, const void* indices);
	void				destroy_index_buffer(IndexBufferId id);

	// Textures
	TextureId			create_texture(uint32_t width, uint32_t height, PixelFormat format, const void* data);
	void				update_texture(TextureId id, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void* data);
	void				destroy_texture(TextureId id);

	// Vertex shaders
	ShaderId			create_shader(ShaderType type, const char* text);
	void				destroy_shader(ShaderId id);

	// GPU programs
	GPUProgramId		create_gpu_program(ShaderId vertex, ShaderId pixel);
	void				destroy_gpu_program(GPUProgramId id);

	UniformId			create_uniform(const char* name, UniformType type);
	void				destroy_uniform(UniformId id);

	// Render Targets
	RenderTargetId		create_render_target(uint16_t width, uint16_t height, RenderTargetFormat format);
	void				destroy_render_target(RenderTargetId id);

	// Draws a complete frame
	void				frame();

	void				draw_lines(const float* vertices, const float* colors, uint32_t count);

private:

	HeapAllocator		m_allocator;

	// Limits
	int32_t				m_max_texture_size;
	int32_t				m_max_texture_units;
	int32_t				m_max_vertex_indices;
	int32_t				m_max_vertex_vertices;

	float				m_max_anisotropy;
	float				m_min_max_point_size[2];
	float				m_min_max_line_width[2];

	// Texture management
	IdTable 			m_textures_id_table;
	Texture				m_textures[CROWN_MAX_TEXTURES];

	uint32_t			m_active_texture_unit;
	GLuint				m_texture_unit[CROWN_MAX_TEXTURE_UNITS];
	GLenum				m_texture_unit_target[CROWN_MAX_TEXTURE_UNITS];

	// Vertex/Index buffer management
	IdTable				m_vertex_buffers_id_table;
	VertexBuffer		m_vertex_buffers[CROWN_MAX_VERTEX_BUFFERS];

	IdTable				m_index_buffers_id_table;
	IndexBuffer			m_index_buffers[CROWN_MAX_INDEX_BUFFERS];

	// Vertex shader management
	IdTable 			m_shaders_id_table;
	Shader				m_shaders[CROWN_MAX_SHADERS];

	// GPU program management
	IdTable 			m_gpu_programs_id_table;
	GPUProgram			m_gpu_programs[CROWN_MAX_GPU_PROGRAMS];

	IdTable				m_uniforms_id_table;
	Uniform				m_uniforms[CROWN_MAX_UNIFORMS];

	// Render buffer management
	IdTable				m_render_targets_id_table;
	RenderTarget		m_render_targets[CROWN_MAX_RENDER_TARGETS];

	// Context management
	GLContext			m_context;
};

} // namespace crown

