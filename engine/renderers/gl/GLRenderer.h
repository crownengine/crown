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

#include "Config.h"

#if defined(LINUX) || defined(WINDOWS)
	#include <GL/glew.h>
#elif defined(ANDROID)
	#include <GLES2/gl2.h>
#else
	#error "Oops, wrong platform"
#endif

#include "Renderer.h"
#include "IdTable.h"
#include "Resource.h"
#include "GLContext.h"
#include "HeapAllocator.h"
#include "VertexFormat.h"
#include "Allocator.h"

namespace crown
{

extern const GLenum TEXTURE_MIN_FILTER_TABLE[];
extern const GLenum TEXTURE_MAG_FILTER_TABLE[];
extern const GLenum TEXTURE_WRAP_TABLE[];
extern const char* const SHADER_ATTRIB_NAMES[ATTRIB_COUNT];
extern const char* const SHADER_UNIFORM_NAMES[];
extern const size_t UNIFORM_SIZE_TABLE[UNIFORM_END];
extern ShaderUniform name_to_stock_uniform(const char* uniform);

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
		GL_CHECK(glBufferData(GL_ARRAY_BUFFER, count * Vertex::bytes_per_vertex(format), vertices,
			(vertices == NULL) ? GL_STREAM_DRAW : GL_STATIC_DRAW));
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
		GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLushort), indices,
					(indices == NULL) ? GL_STREAM_DRAW : GL_STATIC_DRAW));
		GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

		m_index_count = count;
	}

	//-----------------------------------------------------------------------------
	void update(size_t offset, size_t count, const void* indices)
	{
		GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id));
		GL_CHECK(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset * sizeof(uint16_t),
									count * sizeof(uint16_t), indices));
		GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
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
struct Uniform
{
	Uniform()
	{
		string::strncpy(m_name, "", CROWN_MAX_UNIFORM_NAME_LENGTH);
	}

	void create(const char* name, UniformType type, uint8_t num)
	{
		string::strncpy(m_name, name, CROWN_MAX_UNIFORM_NAME_LENGTH);
		m_type = type;
		m_num = num;

		size_t size = UNIFORM_SIZE_TABLE[type] * num;
		m_data = default_allocator().allocate(size);
		memset(m_data, 0, size);

		// Log::d("Uniform created, name = %s, type = %d, num = %d, size = %d", m_name, type, num, size);
	}

	void update(size_t size, const void* data)
	{
		// Log::d("Uniform updated, new size = %d, new ptr = %d", size, *((int32_t*)data));
		memcpy(m_data, data, size);
	}

	void destroy()
	{
		default_allocator().deallocate(m_data);
	}

public:

	char m_name[CROWN_MAX_UNIFORM_NAME_LENGTH];
	UniformType m_type;
	uint8_t m_num;
	void* m_data;
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

		#if defined(LINUX) || defined(WINDOWS)
			GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE));
		#endif

		// FIXME
		GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
					 			GL_RGB, GL_UNSIGNED_BYTE, data));
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

		m_target = GL_TEXTURE_2D;
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
		GLenum mag_filter = TEXTURE_MAG_FILTER_TABLE[(flags & TEXTURE_FILTER_MASK) >> TEXTURE_FILTER_SHIFT];
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
	void create(const Shader& vertex, const Shader& pixel, uint32_t num_uniforms, Uniform* uniforms)
	{
		m_id = GL_CHECK(glCreateProgram());

		GL_CHECK(glAttachShader(m_id, vertex.m_id));
		GL_CHECK(glAttachShader(m_id, pixel.m_id));

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

			GLint attrib_location = GL_CHECK(glGetAttribLocation(m_id, attrib_name));
			Log::d("Attrib %d: name = '%s' location = '%d'", attrib, attrib_name, attrib_location);
		}

		m_num_active_attribs = 0;
		for (uint32_t attrib = 0; attrib < ATTRIB_COUNT; attrib++)
		{
			GLint loc = GL_CHECK(glGetAttribLocation(m_id, SHADER_ATTRIB_NAMES[attrib]));
			if (loc != -1)
			{
				m_active_attribs[m_num_active_attribs] = (ShaderAttrib) attrib;
				m_num_active_attribs++;
				m_attrib_locations[attrib] = loc;
			}
		}

		m_num_stock_uniforms = 0;
		m_num_uniforms = 0;
		for (GLint uniform = 0; uniform < num_active_uniforms; uniform++)
		{
			GLint uniform_size;
			GLenum uniform_type;
			char uniform_name[1024];
			GL_CHECK(glGetActiveUniform(m_id, uniform, max_uniform_length, NULL, &uniform_size, &uniform_type, uniform_name));
			GLint uniform_location = GL_CHECK(glGetUniformLocation(m_id, uniform_name));

			ShaderUniform stock_uniform = name_to_stock_uniform(uniform_name);
			if (stock_uniform != UNIFORM_COUNT)
			{
				m_stock_uniforms[m_num_stock_uniforms] = stock_uniform;
				m_stock_uniform_locations[m_num_stock_uniforms] = uniform_location;
				m_num_stock_uniforms++;
			}
			else
			{
				for (uint32_t i = 0; i < num_uniforms; i++)
				{
					if (string::strcmp(uniforms[i].m_name, uniform_name) == 0)
					{
						m_uniforms[m_num_uniforms] = uniforms[i].m_type;
						m_uniform_info[m_num_uniforms].loc = uniform_location;
						m_uniform_info[m_num_uniforms].data = uniforms[i].m_data;
						m_num_uniforms++;
					}
				}
			}

			Log::d("Uniform %d: name = '%s' location = '%d' stock = %s", uniform, uniform_name, uniform_location,
						(stock_uniform != UNIFORM_COUNT) ? "yes" : "no");
		}
	}

	//-----------------------------------------------------------------------------
	void destroy()
	{
		GL_CHECK(glUseProgram(0));
		GL_CHECK(glDeleteProgram(m_id));
	}

	//-----------------------------------------------------------------------------
	void bind_attributes(VertexFormat format) const
	{
		// Bind all active attributes
		for (uint8_t i = 0; i < m_num_active_attribs; i++)
		{
			ShaderAttrib attrib = m_active_attribs[i];
			GLint loc = m_attrib_locations[attrib];

			const VertexFormatInfo& info = Vertex::info(format);

			if (loc != -1 && info.has_attrib(attrib))
			{
				GL_CHECK(glEnableVertexAttribArray(loc));
				GL_CHECK(glVertexAttribPointer(loc, info.num_components(attrib), GL_FLOAT, GL_FALSE, info.attrib_stride(attrib),
										(GLvoid*)(uintptr_t) info.attrib_offset(attrib)));
			}
		}
	}

	//-----------------------------------------------------------------------------
	void commit() const
	{
		for (uint8_t i = 0; i < m_num_uniforms; i++)
		{
			const UniformType type = m_uniforms[i];
			const GLint loc = m_uniform_info[i].loc;
			const void* data = m_uniform_info[i].data;

			switch (type)
			{
				case UNIFORM_INTEGER_1:   GL_CHECK(glUniform1iv(loc, 1, (const GLint*)data)); break;
				case UNIFORM_INTEGER_2:   GL_CHECK(glUniform2iv(loc, 2, (const GLint*)data)); break;
				case UNIFORM_INTEGER_3:   GL_CHECK(glUniform3iv(loc, 3, (const GLint*)data)); break;				
				case UNIFORM_INTEGER_4:   GL_CHECK(glUniform4iv(loc, 4, (const GLint*)data)); break;
				case UNIFORM_FLOAT_1:     GL_CHECK(glUniform1fv(loc, 1, (const GLfloat*)data)); break;
				case UNIFORM_FLOAT_2:     GL_CHECK(glUniform2fv(loc, 2, (const GLfloat*)data)); break;
				case UNIFORM_FLOAT_3:     GL_CHECK(glUniform3fv(loc, 3, (const GLfloat*)data)); break;
				case UNIFORM_FLOAT_4:     GL_CHECK(glUniform4fv(loc, 4, (const GLfloat*)data)); break;
				case UNIFORM_FLOAT_3_X_3: GL_CHECK(glUniformMatrix3fv(loc, 9, GL_FALSE, (const GLfloat*)data)); break;
				case UNIFORM_FLOAT_4_X_4: GL_CHECK(glUniformMatrix4fv(loc, 16, GL_FALSE, (const GLfloat*)data)); break;
				default: CE_ASSERT(false, "Oops, unknown uniform type"); break;
			}
		}
	}

public:

	GLuint				m_id;

	uint8_t				m_num_active_attribs;
	ShaderAttrib		m_active_attribs[ATTRIB_COUNT];
	GLint				m_attrib_locations[ATTRIB_COUNT];

	uint8_t				m_num_stock_uniforms;
	ShaderUniform		m_stock_uniforms[UNIFORM_COUNT];
	GLint				m_stock_uniform_locations[UNIFORM_COUNT];

	struct LocAndData
	{
		GLint loc;
		void* data;
	};

	uint8_t				m_num_uniforms;
	UniformType			m_uniforms[16];
	LocAndData			m_uniform_info[16];
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

} // namespace crown
