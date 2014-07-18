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
	#define GL_GLEXT_PROTOTYPES
	#include <GL/glcorearb.h>
#elif defined(ANDROID)
	#include <GLES2/gl2.h>
	#include <GLES2/gl2ext.h>
#else
	#error "Oops, wrong platform"
#endif

#include <algorithm>
#include "Assert.h"
#include "Device.h"
#include "GLContext.h"
#include "StringUtils.h"
#include "Log.h"
#include "Matrix3x3.h"
#include "Matrix4x4.h"
#include "Renderer.h"
#include "Resource.h"
#include "StringUtils.h"
#include "Types.h"
#include "Vector4.h"
#include "Memory.h"
#include "VertexFormat.h"
#include "Hash.h"
#include "MathUtils.h"
#include "PixelFormat.h"

#if defined(CROWN_DEBUG)

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

	#define GL_CHECK(function)\
		function;\
		do { GLenum error; CE_ASSERT((error = glGetError()) == GL_NO_ERROR,\
				"OpenGL error: %s", gl_error_to_string(error)); } while (0)
#else
	#define GL_CHECK(function)\
		function;
#endif

#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
	#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
	#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
	#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#endif

#ifdef ANDROID
	#define GL_DEPTH_STENCIL GL_DEPTH_STENCIL_OES
	#ifndef GL_DEPTH_STENCIL_ATTACHMENT
		#define GL_DEPTH_STENCIL_ATTACHMENT 0x821A
	#endif // GL_DEPTH_STENCIL_ATTACHMENT
	#define GL_DEPTH_COMPONENT24 GL_DEPTH_COMPONENT24_OES
	#define GL_DEPTH_COMPONENT32 GL_DEPTH_COMPONENT32_OES
	#define GL_DEPTH24_STENCIL8 GL_DEPTH24_STENCIL8_OES
#endif // ANDROID

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
struct TextureFormatInfo
{
	GLenum internal_format;
	GLenum format;
};

//-----------------------------------------------------------------------------
const TextureFormatInfo TEXTURE_FORMAT_TABLE[PixelFormat::COUNT] =
{
	{ GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT },
	{ GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT },
	{ GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT },
	{ GL_RGB,                           GL_RGB                           },
	{ GL_RGBA,                          GL_RGBA                          },
	{ GL_DEPTH_COMPONENT16,             GL_DEPTH_COMPONENT               },
	{ GL_DEPTH_COMPONENT24,             GL_DEPTH_COMPONENT               },
	{ GL_DEPTH_COMPONENT32,             GL_DEPTH_COMPONENT               },
	{ GL_DEPTH24_STENCIL8,              GL_DEPTH_STENCIL                 }
};

//-----------------------------------------------------------------------------
const GLenum DEPTH_FUNCTION_TABLE[] = 
{
	0, // Unused
	GL_NEVER,
	GL_LESS,
	GL_EQUAL,
	GL_LEQUAL,
	GL_GREATER,
	GL_NOTEQUAL,
	GL_GEQUAL,
	GL_ALWAYS
};

//-----------------------------------------------------------------------------
const GLenum BLEND_FUNCTION_TABLE[] =
{
	0, // Unused
	GL_ZERO,
	GL_ONE,
	GL_SRC_COLOR,
	GL_ONE_MINUS_SRC_COLOR,
	GL_DST_COLOR,
	GL_ONE_MINUS_DST_COLOR,
	GL_SRC_ALPHA,
	GL_ONE_MINUS_SRC_ALPHA,
	GL_DST_ALPHA,
	GL_ONE_MINUS_DST_ALPHA
};

//-----------------------------------------------------------------------------
const GLenum BLEND_EQUATION_TABLE[] =
{
	0, // Unused
	GL_FUNC_ADD,
	GL_FUNC_SUBTRACT,
	GL_FUNC_REVERSE_SUBTRACT
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

static UniformType::Enum gl_enum_to_uniform_type(GLenum type)
{
	switch (type)
	{
		case GL_INT: return UniformType::INTEGER_1;
		case GL_INT_VEC2: return UniformType::INTEGER_2;
		case GL_INT_VEC3: return UniformType::INTEGER_3;
		case GL_INT_VEC4: return UniformType::INTEGER_4;
		case GL_FLOAT: return UniformType::FLOAT_1;
		case GL_FLOAT_VEC2: return UniformType::FLOAT_2;
		case GL_FLOAT_VEC3: return UniformType::FLOAT_3;
		case GL_FLOAT_VEC4: return UniformType::FLOAT_4;
		case GL_FLOAT_MAT3: return UniformType::FLOAT_3x3;
		case GL_FLOAT_MAT4: return UniformType::FLOAT_4x4;
		case GL_SAMPLER_2D: return UniformType::INTEGER_1;
		default: CE_FATAL("Oops, unknown uniform type"); return UniformType::END;
	}
}

//-----------------------------------------------------------------------------
struct VertexBuffer
{
	//-----------------------------------------------------------------------------
	void create(size_t size, const void* data, VertexFormat::Enum format)
	{
		GL_CHECK(glGenBuffers(1, &m_id));
		CE_ASSERT(m_id != 0, "Failed to create buffer");
		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_id));
		GL_CHECK(glBufferData(GL_ARRAY_BUFFER, size, data, (data == NULL) ? GL_STREAM_DRAW : GL_STATIC_DRAW));
		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));

		m_size = size;
		m_format = format;
	}

	//-----------------------------------------------------------------------------
	void update(size_t offset, size_t size, const void* data)
	{
		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_id));
		GL_CHECK(glBufferSubData(GL_ARRAY_BUFFER, offset, size, data));
		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	//-----------------------------------------------------------------------------
	void destroy()
	{
		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
		GL_CHECK(glDeleteBuffers(1, &m_id));
	}

public:

	GLuint m_id;
	size_t m_size;
	VertexFormat::Enum m_format;
};

//-----------------------------------------------------------------------------
struct IndexBuffer
{
	//-----------------------------------------------------------------------------
	void create(size_t size, const void* data)
	{
		GL_CHECK(glGenBuffers(1, &m_id));
		CE_ASSERT(m_id != 0, "Failed to create buffer");
		GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id));
		GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, (data == NULL) ? GL_STREAM_DRAW : GL_STATIC_DRAW));
		GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

		m_size = size;
	}

	//-----------------------------------------------------------------------------
	void update(size_t offset, size_t size, const void* data)
	{
		GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id));
		GL_CHECK(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size, data));
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
	uint32_t	m_size;
};

//-----------------------------------------------------------------------------
struct Shader
{
	//-----------------------------------------------------------------------------
	void create(ShaderType::Enum type, const char* text)
	{
		m_id = GL_CHECK(glCreateShader(type == ShaderType::VERTEX ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER));

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
	void create(uint32_t width, uint32_t height, uint8_t num_mips, PixelFormat::Enum format, const void* data)
	{
		GL_CHECK(glGenTextures(1, &m_id));
		CE_ASSERT(m_id != 0, "Failed to create texture");
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_id));

		const GLenum internal_fmt = TEXTURE_FORMAT_TABLE[format].internal_format;
		const GLenum fmt = TEXTURE_FORMAT_TABLE[format].format;

		if (pixel_format::is_compressed(format))
		{
			const char* src = (const char*) data;
			uint32_t w = width;
			uint32_t h = height;

			for (uint8_t i = 0; i < num_mips; i++)
			{
				GL_CHECK(glCompressedTexImage2D(GL_TEXTURE_2D,
							i,
							internal_fmt,
							w, h,
							0,
						 	pixel_format::size(format) * w * h,
						 	src));
			}
		}
		else
		{
			const char* src = (const char*) data;
			uint32_t w = width;
			uint32_t h = height;

			for (uint8_t i = 0; i < num_mips; i++)
			{
				GL_CHECK(glTexImage2D(GL_TEXTURE_2D,
							i,
							internal_fmt,
							w, h,
							0,
						 	fmt,
						 	GL_UNSIGNED_BYTE,
						 	src));

				w = math::max(1u, w >> 1);
				h = math::max(1u, h >> 1);
				src += pixel_format::size(format) * w * h;
			}
		}

		GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

		m_target = GL_TEXTURE_2D;
		m_width = width;
		m_height = height;
		m_format = format;
		m_gl_fmt = fmt;
	}

	//-----------------------------------------------------------------------------
	void update(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void* data)
	{
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_id));

		if (pixel_format::is_compressed(m_format))
		{
			GL_CHECK(glCompressedTexSubImage2D(GL_TEXTURE_2D,
				0,
				x, y,
				width, height,
				m_gl_fmt,
				pixel_format::size(m_format) * width * height,
				data));
		}
		else
		{
			GL_CHECK(glTexSubImage2D(GL_TEXTURE_2D,
				0,
				x, y,
				width, height,
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				data));
		}

		GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
	}

	//-----------------------------------------------------------------------------
	void set_sampler_state(uint32_t flags)
	{
		const GLenum min_filter = TEXTURE_MIN_FILTER_TABLE[(flags & TEXTURE_FILTER_MASK) >> TEXTURE_FILTER_SHIFT];
		const GLenum mag_filter = TEXTURE_MAG_FILTER_TABLE[(flags & TEXTURE_FILTER_MASK) >> TEXTURE_FILTER_SHIFT];
		const GLenum wrap_s = TEXTURE_WRAP_TABLE[(flags & TEXTURE_WRAP_U_MASK) >> TEXTURE_WRAP_U_SHIFT];
		const GLenum wrap_t = TEXTURE_WRAP_TABLE[(flags & TEXTURE_WRAP_V_MASK) >> TEXTURE_WRAP_V_SHIFT];

		GL_CHECK(glTexParameteri(m_target, GL_TEXTURE_WRAP_S, wrap_s));
		GL_CHECK(glTexParameteri(m_target, GL_TEXTURE_WRAP_T, wrap_t));

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
		//GL_CHECK(glEnable(m_target));
		GL_CHECK(glBindTexture(m_target, m_id));

		set_sampler_state(flags);
	}

public:

	GLuint m_id;
	GLenum m_target;      // Always GL_TEXTURE_2D
	uint32_t m_width;
	uint32_t m_height;
	PixelFormat::Enum m_format;
	GLenum m_gl_fmt;
};

//-----------------------------------------------------------------------------
struct GPUProgram
{
	//-----------------------------------------------------------------------------
	void create(const Shader& vertex, const Shader& pixel, const Hash<void*>& registry)
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

		GL_CHECK(glValidateProgram(m_id));
		GLint valid;
		GL_CHECK(glGetProgramiv(m_id, GL_VALIDATE_STATUS, &valid));
		if (!valid)
		{
			GLchar info_log[2048];
			GL_CHECK(glGetProgramInfoLog(m_id, 2048, NULL, info_log));
			CE_ASSERT(false, "GPU program validation failed:\n%s", info_log);
		}

		// Find active attribs/uniforms
		GLint num_active_attribs;
		GLint num_active_uniforms;
		GL_CHECK(glGetProgramiv(m_id, GL_ACTIVE_ATTRIBUTES, &num_active_attribs));
		GL_CHECK(glGetProgramiv(m_id, GL_ACTIVE_UNIFORMS, &num_active_uniforms));

		// CE_LOGD("Found %d active attribs", num_active_attribs);
		// CE_LOGD("Found %d active uniforms", num_active_uniforms);

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

			/* GLint attrib_location = */GL_CHECK(glGetAttribLocation(m_id, attrib_name));
			// CE_LOGD("Attrib %d: name = '%s' location = '%d'", attrib, attrib_name, attrib_location);
		}

		m_num_active_attribs = 0;
		for (uint32_t attrib = 0; attrib < ShaderAttrib::COUNT; attrib++)
		{
			GLint loc = GL_CHECK(glGetAttribLocation(m_id, SHADER_ATTRIB_NAMES[attrib]));
			if (loc != -1)
			{
				m_active_attribs[m_num_active_attribs] = (ShaderAttrib::Enum) attrib;
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

			ShaderUniform::Enum stock_uniform = name_to_stock_uniform(uniform_name);
			if (stock_uniform != ShaderUniform::COUNT)
			{
				m_stock_uniforms[m_num_stock_uniforms] = stock_uniform;
				m_stock_uniform_locations[m_num_stock_uniforms] = uniform_location;
				m_num_stock_uniforms++;
			}
			else
			{
				void* data = hash::get(registry, string::murmur2_64(uniform_name, string::strlen(uniform_name)), (void*) NULL);

				if (data == NULL)
					continue;

				m_uniforms[m_num_uniforms] = gl_enum_to_uniform_type(uniform_type);
				m_uniform_info[m_num_uniforms].loc = uniform_location;
				m_uniform_info[m_num_uniforms].data = data;
				m_num_uniforms++;
			}

			// CE_LOGD("Uniform %d: name = '%s' location = '%d' stock = %s", uniform, uniform_name, uniform_location,
			// 			 (stock_uniform != ShaderUniform::COUNT) ? "yes" : "no");
		}
	}

	//-----------------------------------------------------------------------------
	void destroy()
	{
		GL_CHECK(glUseProgram(0));
		GL_CHECK(glDeleteProgram(m_id));
	}

	//-----------------------------------------------------------------------------
	void bind_attributes(VertexFormat::Enum format, uint32_t start_vertex) const
	{
		// Bind all active attributes
		for (uint8_t i = 0; i < m_num_active_attribs; i++)
		{
			ShaderAttrib::Enum attrib = m_active_attribs[i];
			GLint loc = m_attrib_locations[attrib];

			const VertexFormatInfo& info = Vertex::info(format);

			if (loc == -1)
				return;

			if (info.has_attrib(attrib))
			{
				GL_CHECK(glEnableVertexAttribArray(loc));
				uint32_t base_vertex = start_vertex * info.attrib_stride(attrib) + info.attrib_offset(attrib);
				GL_CHECK(glVertexAttribPointer(loc, info.num_components(attrib), GL_FLOAT, GL_FALSE, info.attrib_stride(attrib),
										(GLvoid*)(uintptr_t) base_vertex));
			}
			else
			{
				GL_CHECK(glDisableVertexAttribArray(loc));
			}
		}
	}

	//-----------------------------------------------------------------------------
	void commit() const
	{
		for (uint8_t i = 0; i < m_num_uniforms; i++)
		{
			const UniformType::Enum type = m_uniforms[i];
			const GLint loc = m_uniform_info[i].loc;
			const void* data = m_uniform_info[i].data;

			switch (type)
			{
				case UniformType::INTEGER_1:   GL_CHECK(glUniform1iv(loc, 1, (const GLint*)data)); break;
				case UniformType::INTEGER_2:   GL_CHECK(glUniform2iv(loc, 1, (const GLint*)data)); break;
				case UniformType::INTEGER_3:   GL_CHECK(glUniform3iv(loc, 1, (const GLint*)data)); break;				
				case UniformType::INTEGER_4:   GL_CHECK(glUniform4iv(loc, 1, (const GLint*)data)); break;
				case UniformType::FLOAT_1:     GL_CHECK(glUniform1fv(loc, 1, (const GLfloat*)data)); break;
				case UniformType::FLOAT_2:     GL_CHECK(glUniform2fv(loc, 1, (const GLfloat*)data)); break;
				case UniformType::FLOAT_3:     GL_CHECK(glUniform3fv(loc, 1, (const GLfloat*)data)); break;
				case UniformType::FLOAT_4:     GL_CHECK(glUniform4fv(loc, 1, (const GLfloat*)data)); break;
				case UniformType::FLOAT_3x3:   GL_CHECK(glUniformMatrix3fv(loc, 1, GL_FALSE, (const GLfloat*)data)); break;
				case UniformType::FLOAT_4x4:   GL_CHECK(glUniformMatrix4fv(loc, 1, GL_FALSE, (const GLfloat*)data)); break;
				default: CE_FATAL("Oops, unknown uniform type"); break;
			}
		}
	}

public:

	GLuint				m_id;

	uint8_t				m_num_active_attribs;
	ShaderAttrib::Enum	m_active_attribs[ShaderAttrib::COUNT];
	GLint				m_attrib_locations[ShaderAttrib::COUNT];

	uint8_t				m_num_stock_uniforms;
	ShaderUniform::Enum	m_stock_uniforms[ShaderUniform::COUNT];
	GLint				m_stock_uniform_locations[ShaderUniform::COUNT];

	struct LocAndData
	{
		GLint loc;
		void* data;
	};

	uint8_t				m_num_uniforms;
	UniformType::Enum	m_uniforms[16];
	LocAndData			m_uniform_info[16];
};

//-----------------------------------------------------------------------------
struct RenderTarget
{
	void create(uint16_t width, uint16_t height, PixelFormat::Enum format, uint32_t flags)
	{
		m_width = width;
		m_height = height;
		m_format = format;
		m_col_texture = 0;
		m_fbo = 0;
		m_rbo = 0;

		GL_CHECK(glGenFramebuffers(1, &m_fbo));
		CE_ASSERT(m_fbo != 0, "Failed to create frame buffer");
		GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));

		const bool no_texture = (flags & RENDER_TARGET_NO_TEXTURE) >> RENDER_TARGET_SHIFT;
		const TextureFormatInfo& tif = TEXTURE_FORMAT_TABLE[format];

		const GLenum attachment = pixel_format::is_depth(format) ? GL_DEPTH_ATTACHMENT :
									pixel_format::is_color(format) ? GL_COLOR_ATTACHMENT0 : GL_DEPTH_STENCIL_ATTACHMENT;

		if (!no_texture)
		{
			GL_CHECK(glGenTextures(1, &m_col_texture));
			CE_ASSERT(m_col_texture != 0, "Failed to create texture");
			GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_col_texture));
			GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, tif.internal_format, width, height, 0, tif.format, GL_UNSIGNED_BYTE, 0));

			GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	
			GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER,
											attachment,
											GL_TEXTURE_2D,
											m_col_texture,
											0));

			GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
		}
		else
		{
			GL_CHECK(glGenRenderbuffers(1, &m_rbo));
			CE_ASSERT(m_rbo != 0, "Failed to create renderbuffer");
			GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, m_rbo));
			GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, tif.internal_format, width, height));
			GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, m_rbo));
			GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, 0));
		}

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		CE_ASSERT(status == GL_FRAMEBUFFER_COMPLETE, "Oops, framebuffer incomplete!");
		CE_UNUSED(status);

		GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}

	void destroy()
	{
		GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		GL_CHECK(glDeleteTextures(1, &m_col_texture));
		GL_CHECK(glDeleteFramebuffers(1, &m_fbo));
		GL_CHECK(glDeleteRenderbuffers(1, &m_rbo));
	}

	uint16_t m_width;
	uint16_t m_height;
	PixelFormat::Enum m_format;
	GLuint m_col_texture;
	GLuint m_fbo;
	GLuint m_rbo;
};

/// OpenGL renderer
class RendererImplementation
{
public:

	//-----------------------------------------------------------------------------
	RendererImplementation(Renderer* renderer)
		: m_renderer(renderer)
		, m_max_texture_size(0)
		, m_max_texture_units(0)
		, m_max_vertex_indices(0)
		, m_max_vertex_vertices(0)
		, m_max_anisotropy(0.0f)
		, m_uniform_registry(default_allocator())
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

		GL_CHECK(glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_max_texture_size));
		GL_CHECK(glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &m_max_texture_units));
		// GL_CHECK(glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &m_max_vertex_indices));
		// GL_CHECK(glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &m_max_vertex_vertices));

		GL_CHECK(glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, &m_min_max_point_size[0]));
		// GL_CHECK(glGetFloatv(GL_LINE_WIDTH_RANGE, &m_min_max_line_width[0]));

		CE_LOGI("OpenGL Vendor        : %s", glGetString(GL_VENDOR));
		CE_LOGI("OpenGL Renderer      : %s", glGetString(GL_RENDERER));
		CE_LOGI("OpenGL Version       : %s", glGetString(GL_VERSION));
		CE_LOGI("GLSL Version         : %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

		CE_LOGD("Min Point Size       : %f", m_min_max_point_size[0]);
		CE_LOGD("Max Point Size       : %f", m_min_max_point_size[1]);
		CE_LOGD("Min Line Width       : %f", m_min_max_line_width[0]);
		CE_LOGD("Max Line Width       : %f", m_min_max_line_width[1]);
		CE_LOGD("Max Texture Size     : %dx%d", m_max_texture_size, m_max_texture_size);
		CE_LOGD("Max Texture Units    : %d", m_max_texture_units);
		CE_LOGD("Max Vertex Indices   : %d", m_max_vertex_indices);
		CE_LOGD("Max Vertex Vertices  : %d", m_max_vertex_vertices);

		CE_LOGI("OpenGL Renderer initialized.");
	}

	//-----------------------------------------------------------------------------
	void shutdown()
	{
		m_gl_context.destroy_context();
	}

	//-----------------------------------------------------------------------------
	void render(RenderContext& context)
	{
		RenderTargetId cur_rt;
		cur_rt.index = 0xFFFF;
		cur_rt.id = INVALID_ID;
		uint8_t layer = 0xFF;

		// Sort render keys
		context.sort();

		// Update transient buffers
		if (context.m_tvb_offset)
		{
			m_vertex_buffers[context.m_transient_vb->vb.index].update(0, context.m_tvb_offset, context.m_transient_vb->data);
		}
		if (context.m_tib_offset)
		{
			m_index_buffers[context.m_transient_ib->ib.index].update(0, context.m_tib_offset, context.m_transient_ib->data);
		}

		// Bind default framebuffer
		GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));

		for (uint32_t s = 0; s < context.m_num_states; s++)
		{
			const uint64_t key_s = context.m_keys[s].key;
			RenderKey key;
			key.decode(key_s);

			const RenderState& cur_state = context.m_states[context.m_keys[s].state];
			const uint64_t flags = cur_state.m_flags;
			//const RenderTargetId& cur_rt = context.m_targets[layer];

			// Check if layer changed
			if (key.m_layer != layer)
			{
				layer = key.m_layer;

				// Switch render target if necessary
				if (cur_rt != context.m_targets[layer])
				{
					cur_rt = context.m_targets[layer];
					glBindFramebuffer(GL_FRAMEBUFFER, m_render_targets[cur_rt.index].m_fbo);
				}

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
			}

			// Depth test
			if (flags & STATE_DEPTH_TEST_MASK)
			{
				uint32_t depthf = (flags & STATE_DEPTH_TEST_MASK) >> STATE_DEPTH_TEST_SHIFT;

				GL_CHECK(glEnable(GL_DEPTH_TEST));
				GL_CHECK(glDepthFunc(DEPTH_FUNCTION_TABLE[depthf]));
			}
			else
			{
				GL_CHECK(glDisable(GL_DEPTH_TEST));
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

			// Blending
			if (flags & (STATE_BLEND_FUNC_MASK | STATE_BLEND_EQUATION_MASK))
			{
				uint32_t function = (flags & STATE_BLEND_FUNC_MASK) >> STATE_BLEND_FUNC_SHIFT;
				uint32_t equation = (flags & STATE_BLEND_EQUATION_MASK) >> STATE_BLEND_EQUATION_SHIFT;
				uint32_t src = (function >> 4) & 0xF;
				uint32_t dst = function & 0xF;

				GL_CHECK(glEnable(GL_BLEND));
				GL_CHECK(glBlendFunc(BLEND_FUNCTION_TABLE[src], BLEND_FUNCTION_TABLE[dst]));
				GL_CHECK(glBlendEquation(BLEND_EQUATION_TABLE[equation]));
			}
			else
			{
				GL_CHECK(glDisable(GL_BLEND));
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
							case SAMPLER_RENDER_TARGET:
							{
								RenderTarget& rt = m_render_targets[sampler.sampler_id.index];
								GL_CHECK(glActiveTexture(GL_TEXTURE0 + unit));
								GL_CHECK(glBindTexture(GL_TEXTURE_2D, rt.m_col_texture));
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

			m_renderer->update_uniforms(context.m_constants, cur_state.begin_uniform, cur_state.end_uniform);

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
							GL_CHECK(glUniformMatrix4fv(uniform_location, 1, GL_FALSE, matrix4x4::to_float_ptr(view)));
							break;
						}
						case ShaderUniform::MODEL:
						{
							GL_CHECK(glUniformMatrix4fv(uniform_location, 1, GL_FALSE, matrix4x4::to_float_ptr(cur_state.pose)));
							break;
						}
						case ShaderUniform::MODEL_VIEW:
						{
							GL_CHECK(glUniformMatrix4fv(uniform_location, 1, GL_FALSE, matrix4x4::to_float_ptr(view * cur_state.pose)));
							break;
						}
						case ShaderUniform::MODEL_VIEW_PROJECTION:
						{
							GL_CHECK(glUniformMatrix4fv(uniform_location, 1, GL_FALSE, matrix4x4::to_float_ptr(projection * view * cur_state.pose)));
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
				const VertexFormat::Enum format = vertex_buffer.m_format == VertexFormat::COUNT ? cur_state.vertex_format : vertex_buffer.m_format;
				gpu_program.bind_attributes(format, cur_state.start_vertex);
			}
			else
			{
				GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
			}

			const IndexBufferId& ib = cur_state.ib;
			if (ib.id != INVALID_ID)
			{
				const IndexBuffer& index_buffer = m_index_buffers[ib.index];
				const uint32_t prim_type = (flags & STATE_PRIMITIVE_MASK) >> STATE_PRIMITIVE_SHIFT;
				const GLenum gl_prim_type = PRIMITIVE_TYPE_TABLE[prim_type];
				const uint32_t num_indices = cur_state.num_indices == 0xFFFFFFFF ? index_buffer.m_size : cur_state.num_indices;

				GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer.m_id));
				GL_CHECK(glDrawElements(gl_prim_type, num_indices, GL_UNSIGNED_SHORT, (void*) (uintptr_t) (cur_state.start_index * sizeof(uint16_t))));
			}
			else
			{
				GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
			}
		}

		GL_CHECK(glFinish());
		m_gl_context.swap_buffers();
	}

private:

	Renderer*			m_renderer;
	GLContext			m_gl_context;

	// Limits
	int32_t				m_max_texture_size;
	int32_t				m_max_texture_units;
	int32_t				m_max_vertex_indices;
	int32_t				m_max_vertex_vertices;

	float				m_max_anisotropy;
	float				m_min_max_point_size[2];
	float				m_min_max_line_width[2];

	VertexBuffer		m_vertex_buffers[CE_MAX_VERTEX_BUFFERS];
	IndexBuffer			m_index_buffers[CE_MAX_INDEX_BUFFERS];
	Texture				m_textures[CE_MAX_TEXTURES];
	Shader				m_shaders[CE_MAX_SHADERS];
	GPUProgram			m_gpu_programs[CE_MAX_GPU_PROGRAMS];
	Hash<void*>			m_uniform_registry;
	void*				m_uniforms[CE_MAX_UNIFORMS];
	RenderTarget		m_render_targets[CE_MAX_RENDER_TARGETS];

private:

	friend class		Renderer;
};

//-----------------------------------------------------------------------------
Renderer::Renderer(Allocator& a)
	: m_allocator(a), m_impl(NULL), m_thread("render-thread"), m_submit(&m_contexts[0]), m_draw(&m_contexts[1]),
		m_is_initialized(false), m_should_run(false)
{
	m_impl = CE_NEW(a, RendererImplementation)(this);
}

//-----------------------------------------------------------------------------
Renderer::~Renderer()
{
	CE_ASSERT(id_table::size(m_vertex_buffers) == 0, "%d vertex buffers not freed", id_table::size(m_vertex_buffers));
	CE_ASSERT(id_table::size(m_index_buffers) == 0, "%d index buffers not freed", id_table::size(m_index_buffers));
	CE_ASSERT(id_table::size(m_textures) == 0, "%d textures not freed", id_table::size(m_textures));
	CE_ASSERT(id_table::size(m_shaders) == 0, "%d shaders not freed", id_table::size(m_shaders));
	CE_ASSERT(id_table::size(m_gpu_programs) == 0, "%d GPU programs not freed", id_table::size(m_gpu_programs));
	CE_ASSERT(id_table::size(m_render_targets) == 0, "%d render targets not freed", id_table::size(m_render_targets));

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
void Renderer::create_vertex_buffer_impl(VertexBufferId id, size_t size, const void* data, VertexFormat::Enum format)
{
	m_impl->m_vertex_buffers[id.index].create(size, data, format);
}

//-----------------------------------------------------------------------------
void Renderer::create_dynamic_vertex_buffer_impl(VertexBufferId id, size_t size)
{
	m_impl->m_vertex_buffers[id.index].create(size, NULL, VertexFormat::COUNT);
}

//-----------------------------------------------------------------------------
void Renderer::update_vertex_buffer_impl(VertexBufferId id, size_t offset, size_t size, const void* data)
{
	m_impl->m_vertex_buffers[id.index].update(offset, size, data);
}

//-----------------------------------------------------------------------------
void Renderer::destroy_vertex_buffer_impl(VertexBufferId id)
{
	m_impl->m_vertex_buffers[id.index].destroy();
	id_table::destroy(m_vertex_buffers, id);
}

//-----------------------------------------------------------------------------
void Renderer::create_index_buffer_impl(IndexBufferId id, size_t size, const void* data)
{
	m_impl->m_index_buffers[id.index].create(size, data);
}

//-----------------------------------------------------------------------------
void Renderer::create_dynamic_index_buffer_impl(IndexBufferId id, size_t size)
{
	m_impl->m_index_buffers[id.index].create(size, NULL);
}

//-----------------------------------------------------------------------------
void Renderer::update_index_buffer_impl(IndexBufferId id, size_t offset, size_t size, const void* data)
{
	m_impl->m_index_buffers[id.index].update(offset, size, data);
}

//-----------------------------------------------------------------------------
void Renderer::destroy_index_buffer_impl(IndexBufferId id)
{
	m_impl->m_index_buffers[id.index].destroy();
	id_table::destroy(m_index_buffers, id);
}

//-----------------------------------------------------------------------------
void Renderer::create_texture_impl(TextureId id, uint32_t width, uint32_t height, uint8_t num_mips, PixelFormat::Enum format, const void* data)
{
	m_impl->m_textures[id.index].create(width, height, num_mips, format, data);
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
	id_table::destroy(m_textures, id);
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
	id_table::destroy(m_shaders, id);
}

//-----------------------------------------------------------------------------
void Renderer::create_gpu_program_impl(GPUProgramId id, ShaderId vertex, ShaderId pixel)
{
	Shader& vs = m_impl->m_shaders[vertex.index];
	Shader& ps = m_impl->m_shaders[pixel.index];
	m_impl->m_gpu_programs[id.index].create(vs, ps, m_impl->m_uniform_registry);
}

//-----------------------------------------------------------------------------
void Renderer::destroy_gpu_program_impl(GPUProgramId id)
{
	m_impl->m_gpu_programs[id.index].destroy();
	id_table::destroy(m_gpu_programs, id);
}

//-----------------------------------------------------------------------------
void Renderer::create_uniform_impl(UniformId id, const char* name, UniformType::Enum type, uint8_t num)
{
	const size_t size = UNIFORM_SIZE_TABLE[type] * num;
	m_impl->m_uniforms[id.index] = default_allocator().allocate(size);
	hash::set(m_impl->m_uniform_registry, string::murmur2_64(name, string::strlen(name)), m_impl->m_uniforms[id.index]);
	memset(m_impl->m_uniforms[id.index], 0, size);
}

//-----------------------------------------------------------------------------
void Renderer::update_uniform_impl(UniformId id, size_t size, const void* data)
{
	memcpy(m_impl->m_uniforms[id.index], data, size);
}

//-----------------------------------------------------------------------------
void Renderer::destroy_uniform_impl(UniformId id)
{
	default_allocator().deallocate(m_impl->m_uniforms[id.index]);
	id_table::destroy(m_uniforms, id);
}

//-----------------------------------------------------------------------------
void Renderer::create_render_target_impl(RenderTargetId id, uint16_t width, uint16_t height, PixelFormat::Enum format, uint32_t flags)
{
	m_impl->m_render_targets[id.index].create(width, height, format, flags);
}

//-----------------------------------------------------------------------------
void Renderer::destroy_render_target_impl(RenderTargetId id)
{
	m_impl->m_render_targets[id.index].destroy();
	id_table::destroy(m_render_targets, id);
}

} // namespace crown
