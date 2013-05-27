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

#include <GLES/gl.h>

#include "GLESIndexBuffer.h"
#include "GLESVertexBuffer.h"
#include "GLESRenderer.h"
#include "GLESUtils.h"
#include "Log.h"
#include "Material.h"
#include "Types.h"

#include "TextureResource.h"

namespace crown
{

//-----------------------------------------------------------------------------
GLESRenderer::GLESRenderer() :
	m_max_lights(0),
	m_max_texture_size(0),
	m_max_texture_units(0),
	m_max_vertex_indices(0),
	m_max_vertex_vertices(0),
	m_max_anisotropy(0.0f),

	m_texture_count(0),
	m_active_texture_unit(0)
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


	glGetIntegerv(GL_MAX_LIGHTS, &m_max_lights);
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_max_texture_size);
	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &m_max_texture_units);
	//glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &m_max_vertex_indices);
	//glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &m_max_vertex_vertices);

	glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, &m_min_max_point_size[0]);
	//glGetFloatv(GL_LINE_WIDTH_RANGE, &m_min_max_line_width[0]);

	const unsigned char* gl_vendor = glGetString(GL_VENDOR);
	const unsigned char* gl_renderer = glGetString(GL_RENDERER);
	const unsigned char* gl_version = glGetString(GL_VERSION);

	Log::i("OpenGL|ES Vendor\t: %s", gl_vendor);
	Log::i("OpenGL|ES Renderer\t: %s", gl_renderer);
	Log::i("OpenGL|ES Version\t: %s", gl_version);
	Log::i("Min Point Size\t: %f", m_min_max_point_size[0]);
	Log::i("Max Point Size\t: %f", m_min_max_point_size[1]);
	Log::i("Min Line Width\t: %f", m_min_max_line_width[0]);
	Log::i("Max Line Width\t: %f", m_min_max_line_width[1]);
	Log::i("Max Texture Size\t: %dx%d", m_max_texture_size, m_max_texture_size);
	Log::i("Max Texture Units\t: %d", m_max_texture_units);
	Log::i("Max Lights\t\t: %d", m_max_lights);
	Log::i("Max Vertex Indices\t: %d", m_max_vertex_indices);
	Log::i("Max Vertex Vertices\t: %d", m_max_vertex_vertices);
	Log::i("Max Anisotropy\t: %f", m_max_anisotropy);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_TEXTURE_2D);

	glEnable(GL_LIGHTING);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glFrontFace(GL_CCW);

	glEnable(GL_CULL_FACE);

	glShadeModel(GL_SMOOTH);

	// Set the global ambient light
	float amb[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);

	// Some hint32_ts
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// Set the framebuffer clear color
	glClearColor(0.5f, 0.5f, 0.5f, 0.5f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// Enable scissor test
	glEnable(GL_SCISSOR_TEST);

	// Disable dithering
	glDisable(GL_DITHER);

	Log::i("OpenGLES Renderer initialized.");
}

//-----------------------------------------------------------------------------
GLESRenderer::~GLESRenderer()
{
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_clear_color(const Color4& color)
{
	glClearColor(color.r, color.g, color.b, color.a);
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_material_params(const Color4& ambient, const Color4& diffuse, const Color4& specular,
				const Color4& emission, int32_t shininess)
{
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, &ambient.r);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, &diffuse.r);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, &specular.r);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, &emission.r);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_lighting(bool lighting)
{
	if (lighting)
	{
		glEnable(GL_LIGHTING);
	}
	else
	{
		glDisable(GL_LIGHTING);
	}
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_ambient_light(const Color4& color)
{
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, color.to_float_ptr());
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
void GLESRenderer::set_texture(uint32_t unit, TextureId texture)
{
	if (!activate_texture_unit(unit))
	{
		return;
	}

	m_texture_unit_target[unit] = GL_TEXTURE_2D;
	m_texture_unit[unit] = m_textures[texture.index].texture_object;

	glEnable(m_texture_unit_target[unit]);
	glBindTexture(m_texture_unit_target[unit], m_texture_unit[unit]);
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_texture_mode(uint32_t unit, TextureMode mode, const Color4& /* blendColor */)
{
	/* No support for blend color, params will be ignored */
	if (!activate_texture_unit(unit))
		return;

	GLint envMode = GLES::GetTextureMode(mode);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, envMode);
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_texture_wrap(uint32_t unit, TextureWrap wrap)
{
	GLenum glWrap = GLES::GetTextureWrap(wrap);

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

	GLES::GetTextureFilter(filter, minFilter, magFilter);

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
void GLESRenderer::set_separate_specular_color(bool /* separate */)
{
	/* No support for separate specular color, params will be ignored */
	Log::w("Renderer does not support separate specular color");
}

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
	GLenum glFunc = GLES::GetCompareFunction(func);

	glDepthFunc(glFunc);
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_rescale_normals(bool rescale)
{
	if (rescale)
	{
		glEnable(GL_RESCALE_NORMAL);
	}
	else
	{
		glDisable(GL_RESCALE_NORMAL);
	}
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
void GLESRenderer::set_blending_params(BlendEquation /* equation */, BlendFunction src, BlendFunction dst, const Color4& /* color */)
{
	/* No support for blend equation, params will be ignored */
	/* No support for blend color, params will be ignored */

	GLenum glSrcFactor = GLES::GetBlendFunction(src);
	GLenum glDstFactor = GLES::GetBlendFunction(dst);

	glBlendFunc(glSrcFactor, glDstFactor);
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
void GLESRenderer::set_fog(bool fog)
{
	if (fog)
	{
		glEnable(GL_FOG);
	}
	else
	{
		glDisable(GL_FOG);
	}
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_fog_params(FogMode mode, float density, float start, float end, const Color4& color)
{
	GLenum glMode = GLES::GetFogMode(mode);

	glFogf(GL_FOG_MODE, glMode);
	glFogf(GL_FOG_DENSITY, density);
	glFogf(GL_FOG_START, start);
	glFogf(GL_FOG_END, end);
	glFogfv(GL_FOG_COLOR, &color.r);
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_alpha_test(bool test)
{
	if (test)
	{
		glEnable(GL_ALPHA_TEST);
	}
	else
	{
		glDisable(GL_ALPHA_TEST);
	}
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_alpha_params(CompareFunction func, float ref)
{
	GLenum glFunc = GLES::GetCompareFunction(func);

	glAlphaFunc(glFunc, ref);
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_shading_type(ShadingType type)
{
	GLenum glMode = GL_SMOOTH;

	if (type == ST_FLAT)
	{
		glMode = GL_FLAT;
	}

	glShadeModel(glMode);
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_polygon_mode(PolygonMode /* mode */)
{
	/* No support for polygon mode, params will be ignored */
	Log::w("Renderer does not support PolygonMode");
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
void GLESRenderer::set_point_sprite(bool sprite)
{
	if (sprite)
	{
		glEnable(GL_POINT_SPRITE_OES);
		glTexEnvi(GL_POINT_SPRITE_OES, GL_COORD_REPLACE_OES, GL_TRUE);
	}
	else
	{
		glDisable(GL_POINT_SPRITE_OES);
		glTexEnvi(GL_POINT_SPRITE_OES, GL_COORD_REPLACE_OES, GL_FALSE);
	}
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_point_size(float size)
{
	glPointSize(size);
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_point_params(float min, float max)
{
	glPointParameterf(GL_POINT_SIZE_MIN, min);
	glPointParameterf(GL_POINT_SIZE_MAX, max);
}

//-----------------------------------------------------------------------------
void GLESRenderer::begin_frame()
{
	// Clear frame/depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

//-----------------------------------------------------------------------------
void GLESRenderer::end_frame()
{
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
			glMatrixMode(GL_MODELVIEW);
			// Transformations must be listed in reverse order
			glLoadMatrixf((m_matrix[MT_VIEW] * m_matrix[MT_MODEL]).to_float_ptr());
			break;
		case MT_PROJECTION:
			glMatrixMode(GL_PROJECTION);
			glLoadMatrixf(m_matrix[MT_PROJECTION].to_float_ptr());
			break;
		case MT_TEXTURE:
			glMatrixMode(GL_TEXTURE);
			glLoadMatrixf(m_matrix[MT_TEXTURE].to_float_ptr());
			break;
		case MT_COLOR:
			//glMatrixMode(GL_COLOR);
			//glLoadMatrixf(m_matrix[MT_COLOR].to_float_ptr());
			break;
		default:
			break;
	}
}

//-----------------------------------------------------------------------------
void GLESRenderer::draw_vertex_index_buffer(const VertexBuffer* vertices, const IndexBuffer* indices)
{
	CE_ASSERT(vertices != NULL);
	CE_ASSERT(indices != NULL);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	vertices->Bind();
	indices->Bind();

	glDrawElements(GL_TRIANGLES, indices->GetIndexCount(), GL_UNSIGNED_SHORT, 0);

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

//-----------------------------------------------------------------------------
void GLESRenderer::draw_point_buffer(const VertexBuffer* buffer)
{
	if (buffer == NULL)
		return;
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	buffer->Bind();
	glDrawArrays(GL_POINTS, 0, buffer->GetVertexCount());

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

//-----------------------------------------------------------------------------
bool GLESRenderer::activate_texture_unit(uint32_t unit)
{
	if (unit >= (uint32_t) m_max_texture_units)
		return false;

	glActiveTexture(GL_TEXTURE0 + unit);
	m_active_texture_unit = unit;
	return true;
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_light(uint32_t light, bool active)
{
	if (light >= (uint32_t) m_max_lights)
	{
		return;
	}

	if (active)
	{
		glEnable(GL_LIGHT0 + light);
	}
	else
	{
		glDisable(GL_LIGHT0 + light);
	}
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_light_params(uint32_t light, LightType type, const Vec3& position)
{
	static float pos[4] =
	{
		position.x,
		position.y,
		position.z,
		1.0f
	};

	if (type == LT_DIRECTION)
	{
		pos[3] = 0.0f;
	}

	glLightfv(GL_LIGHT0 + light, GL_POSITION, pos);
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_light_color(uint32_t light, const Color4& ambient, const Color4& diffuse, const Color4& specular)
{
	glLightfv(GL_LIGHT0 + light, GL_AMBIENT, ambient.to_float_ptr());
	glLightfv(GL_LIGHT0 + light, GL_DIFFUSE, diffuse.to_float_ptr());
	glLightfv(GL_LIGHT0 + light, GL_SPECULAR, specular.to_float_ptr());
}

//-----------------------------------------------------------------------------
void GLESRenderer::set_light_attenuation(uint32_t light, float constant, float linear, float quadratic)
{
	glLightf(GL_LIGHT0 + light, GL_CONSTANT_ATTENUATION, constant);
	glLightf(GL_LIGHT0 + light, GL_LINEAR_ATTENUATION, linear);
	glLightf(GL_LIGHT0 + light, GL_QUADRATIC_ATTENUATION, quadratic);
}

//-----------------------------------------------------------------------------
void GLESRenderer::draw_lines(const float* vertices, const float* colors, uint32_t count)
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glColorPointer(4, GL_FLOAT, 0, colors);

	glDrawArrays(GL_LINES, 0, count);

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

//-----------------------------------------------------------------------------
void GLESRenderer::draw_triangles(const float* vertices, const float* normals, const float* uvs, const uint16_t* indices, uint32_t count)
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glNormalPointer(GL_FLOAT, 0, normals);
	glTexCoordPointer(2, GL_FLOAT, 0, uvs);

	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, indices);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

//-----------------------------------------------------------------------------
TextureId GLESRenderer::load_texture(TextureResource* texture)
{
	// Search for an already existent texture
	for (uint32_t i = 0; i < MAX_TEXTURES; i++)
	{
		if (m_textures[i].texture_resource == texture)
		{
			return m_textures[i].id;
		}
	}

	// If texture not found, create a new one
	GLuint gl_texture_object;

	glGenTextures(1, &gl_texture_object);

	glBindTexture(GL_TEXTURE_2D, gl_texture_object);

	GLint gl_texture_format = GLES::GetPixelFormat(texture->format());

	//FIXME FIXME FIXME
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width(), texture->height(), 0,
				 gl_texture_format, GL_UNSIGNED_BYTE, texture->data());

	TextureId id;
	id.index = m_texture_count;
	id.id = 0;

	m_textures[id.index].texture_object = gl_texture_object;
	m_textures[id.index].texture_resource = texture;
	m_textures[id.index].id = id;

	m_texture_count++;

	return id;
}

//-----------------------------------------------------------------------------
void GLESRenderer::unload_texture(TextureResource* texture)
{
	// FIXME
	(void)texture;
}

//-----------------------------------------------------------------------------
TextureId GLESRenderer::reload_texture(TextureResource* old_texture, TextureResource* new_texture)
{
	// FIXME
	(void)old_texture;
	(void)new_texture;
	return TextureId();
}

//-----------------------------------------------------------------------------
void GLESRenderer::check_gl_errors()
{
	GLenum error;

	while ((error = glGetError()))
	{
		switch (error)
		{
			case GL_INVALID_ENUM:
				Log::e("GLESRenderer: GL_INVALID_ENUM");
				break;
			case GL_INVALID_VALUE:
				Log::e("GLESRenderer: GL_INVALID_VALUE");
				break;
			case GL_INVALID_OPERATION:
				Log::e("GLESRenderer: GL_INVALID_OPERATION");
				break;
			case GL_STACK_OVERFLOW:
				Log::e("GLESRenderer: GL_STACK_OVERFLOW");
				break;
			case GL_STACK_UNDERFLOW:
				Log::e("GLESRenderer: GL_STACK_UNDERFLOW");
				break;
			case GL_OUT_OF_MEMORY:
				Log::e("GLESRenderer: GL_OUT_OF_MEMORY");
				break;
		}
	}
}

} // namespace crown

