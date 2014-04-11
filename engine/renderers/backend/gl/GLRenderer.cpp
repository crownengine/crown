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
struct GLTextureFormatInfo
{
	GLenum internal_format;
	GLenum format;
};

//-----------------------------------------------------------------------------
const GLTextureFormatInfo TEXTURE_FORMAT_TABLE[PixelFormat::COUNT] =
{
	{ GL_RGB, GL_RGB },
	{ GL_RGBA, GL_RGBA}
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
struct Uniform
{
	Uniform()
	{
		string::strncpy(m_name, "", CE_MAX_UNIFORM_NAME_LENGTH);
	}

	void create(const char* name, UniformType::Enum type, uint8_t num)
	{
		string::strncpy(m_name, name, CE_MAX_UNIFORM_NAME_LENGTH);
		m_type = type;
		m_num = num;

		size_t size = UNIFORM_SIZE_TABLE[type] * num;
		m_data = default_allocator().allocate(size);
		memset(m_data, 0, size);

		// Log::d("Uniform created, name = %s, type = %d, num = %d, size = %ld, ptr = %p", m_name, type, num, size, m_data);
	}

	void update(size_t size, const void* data)
	{
		// Log::d("Uniform updated, new size = %ld, new ptr = %d", size, *((int32_t*)data));
		memcpy(m_data, data, size);
	}

	void destroy()
	{
		default_allocator().deallocate(m_data);
		string::strncpy(m_name, "", CE_MAX_UNIFORM_NAME_LENGTH); // <- this is a temporary fix
		// Log::d("Uniform destroyed, name = %s, type = %d, num = %d, ptr = %p", m_name, m_type, m_num, m_data);
	}

public:

	char m_name[CE_MAX_UNIFORM_NAME_LENGTH];
	UniformType::Enum m_type;
	uint8_t m_num;
	void* m_data;
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
	void create(uint32_t width, uint32_t height, PixelFormat::Enum format, const void* data)
	{
		GL_CHECK(glGenTextures(1, &m_id));
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_id));

		#if defined(LINUX) || defined(WINDOWS)
			GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE));
		#endif

		GLenum internal_fmt = TEXTURE_FORMAT_TABLE[format].internal_format;
		GLenum fmt = TEXTURE_FORMAT_TABLE[format].format;

		GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, internal_fmt, width, height, 0,
					 			fmt, GL_UNSIGNED_BYTE, data));
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

	GLuint				m_id;
	GLenum				m_target;      // Always GL_TEXTURE_2D
	uint32_t			m_width;
	uint32_t			m_height;
	PixelFormat::Enum	m_format;
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

		// Log::d("Found %d active attribs", num_active_attribs);
		// Log::d("Found %d active uniforms", num_active_uniforms);

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
			// Log::d("Attrib %d: name = '%s' location = '%d'", attrib, attrib_name, attrib_location);
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

			// Log::d("Uniform %d: name = '%s' location = '%d' stock = %s", uniform, uniform_name, uniform_location,
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

			if (loc != -1 && info.has_attrib(attrib))
			{
				GL_CHECK(glEnableVertexAttribArray(loc));
				uint32_t base_vertex = start_vertex * info.attrib_stride(attrib) + info.attrib_offset(attrib);
				GL_CHECK(glVertexAttribPointer(loc, info.num_components(attrib), GL_FLOAT, GL_FALSE, info.attrib_stride(attrib),
										(GLvoid*)(uintptr_t) base_vertex));
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
				case UniformType::INTEGER_2:   GL_CHECK(glUniform2iv(loc, 2, (const GLint*)data)); break;
				case UniformType::INTEGER_3:   GL_CHECK(glUniform3iv(loc, 3, (const GLint*)data)); break;				
				case UniformType::INTEGER_4:   GL_CHECK(glUniform4iv(loc, 4, (const GLint*)data)); break;
				case UniformType::FLOAT_1:     GL_CHECK(glUniform1fv(loc, 1, (const GLfloat*)data)); break;
				case UniformType::FLOAT_2:     GL_CHECK(glUniform2fv(loc, 2, (const GLfloat*)data)); break;
				case UniformType::FLOAT_3:     GL_CHECK(glUniform3fv(loc, 3, (const GLfloat*)data)); break;
				case UniformType::FLOAT_4:     GL_CHECK(glUniform4fv(loc, 4, (const GLfloat*)data)); break;
				case UniformType::FLOAT_3x3:   GL_CHECK(glUniformMatrix3fv(loc, 9, GL_FALSE, (const GLfloat*)data)); break;
				case UniformType::FLOAT_4x4:   GL_CHECK(glUniformMatrix4fv(loc, 16, GL_FALSE, (const GLfloat*)data)); break;
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
class RendererImplementation
{
public:

	//-----------------------------------------------------------------------------
	RendererImplementation()
		: m_max_texture_size(0)
		, m_max_texture_units(0)
		, m_max_vertex_indices(0)
		, m_max_vertex_vertices(0)
		, m_max_anisotropy(0.0f)
		, m_num_uniforms(0)
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
	uint32_t			m_num_uniforms;
	Uniform				m_uniforms[CE_MAX_UNIFORMS];
	RenderTarget		m_render_targets[CE_MAX_RENDER_TARGETS];

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
	CE_ASSERT(m_vertex_buffers.size() == 0, "%d vertex buffers not freed", m_vertex_buffers.size());
	CE_ASSERT(m_index_buffers.size() == 0, "%d index buffers not freed", m_index_buffers.size());
	CE_ASSERT(m_textures.size() == 0, "%d textures not freed", m_textures.size());
	CE_ASSERT(m_shaders.size() == 0, "%d shaders not freed", m_shaders.size());
	CE_ASSERT(m_gpu_programs.size() == 0, "%d GPU programs not freed", m_gpu_programs.size());
	CE_ASSERT(m_uniforms.size() == 0, "%d uniforms not freed", m_uniforms.size());
	CE_ASSERT(m_render_targets.size() == 0, "%d render targets not freed", m_render_targets.size());

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
	m_vertex_buffers.destroy(id);
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
	m_index_buffers.destroy(id);
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
	m_textures.destroy(id);
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
	m_shaders.destroy(id);
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
	m_gpu_programs.destroy(id);
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
	m_uniforms.destroy(id);
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
