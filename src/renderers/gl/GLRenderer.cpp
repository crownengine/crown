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
#include <cassert>
#include <algorithm>
#include "Types.h"
#include "GLIndexBuffer.h"
#include "GLOcclusionQuery.h"
#include "GLRenderer.h"
#include "GLUtils.h"
#include "GLVertexBuffer.h"
#include "Log.h"
#include "Material.h"
#include "Rect.h"
#include "Allocator.h"

#include "TextureResource.h"

#if defined(WINDOWS)
	//Define the missing constants in vs' gl.h
	#define GL_TEXTURE_3D					0x806F
	#define GL_TEXTURE_CUBE_MAP				0x8513
	#define GL_LIGHT_MODEL_COLOR_CONTROL	0x81F8
	#define GL_SEPARATE_SPECULAR_COLOR		0x81FA
	#define GL_SINGLE_COLOR					0x81F9
	#define GL_GENERATE_MIPMAP				0x8191
#endif

namespace crown
{

//-----------------------------------------------------------------------------
GLRenderer::GLRenderer() :
	mModelMatrixStackIndex(0),

	mMaxLights(0),
	mMaxTextureSize(0),
	mMaxTextureUnits(0),
	mMaxVertexIndices(0),
	mMaxVertexVertices(0),
	mMaxAnisotropy(0.0f),

	m_texture_count(0),
	mActiveTextureUnit(0),

	mLinesCount(0)
{
	mMinMaxPointSize[0] = 0.0f;
	mMinMaxPointSize[1] = 0.0f;
	mMinMaxLineWidth[0] = 0.0f;
	mMinMaxLineWidth[1] = 0.0f;

	// Initialize texture units
	for (uint32_t i = 0; i < MAX_TEXTURE_UNITS; i++)
	{
		mTextureUnit[i] = 0;
		mTextureUnitTarget[i] = GL_TEXTURE_2D;
	}

	// Initialize the matrices
	for (uint32_t i = 0; i < MT_COUNT; i++)
	{
		mMatrix[i].load_identity();
	}

	GLenum err = glewInit();

	assert(err == GLEW_OK);

	Log::I("GLEW initialized.");

	glGetIntegerv(GL_MAX_LIGHTS, &mMaxLights);
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &mMaxTextureSize);
	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &mMaxTextureUnits);
	glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &mMaxVertexIndices);
	glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &mMaxVertexVertices);

	// Check for anisotropic filter support
	if (GLEW_EXT_texture_filter_anisotropic)
	{
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &mMaxAnisotropy);
	}

	glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, &mMinMaxPointSize[0]);
	glGetFloatv(GL_LINE_WIDTH_RANGE, &mMinMaxLineWidth[0]);

	const unsigned char* gl_vendor = glGetString(GL_VENDOR);
	const unsigned char* gl_renderer = glGetString(GL_RENDERER);
	const unsigned char* gl_version = glGetString(GL_VERSION);

	Log::I("OpenGL Vendor\t: %s", gl_vendor);
	Log::I("OpenGL Renderer\t: %s", gl_renderer);
	Log::I("OpenGL Version\t: %s", gl_version);
	Log::I("Min Point Size\t: %f", mMinMaxPointSize[0]);
	Log::I("Max Point Size\t: %f", mMinMaxPointSize[1]);
	Log::I("Min Line Width\t: %f", mMinMaxLineWidth[0]);
	Log::I("Max Line Width\t: %f", mMinMaxLineWidth[1]);
	Log::I("Max Texture Size\t: %dx%d", mMaxTextureSize, mMaxTextureSize);
	Log::I("Max Texture Units\t: %d", mMaxTextureUnits);
	Log::I("Max Lights\t\t: %d", mMaxLights);
	Log::I("Max Vertex Indices\t: %d", mMaxVertexIndices);
	Log::I("Max Vertex Vertices\t: %d", mMaxVertexVertices);
	Log::I("Max Anisotropy\t: %f", mMaxAnisotropy);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_TEXTURE_2D);

	glEnable(GL_LIGHTING);

	glEnable(GL_BLEND);
	//TODO: Use Premultiplied alpha
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	glFrontFace(GL_CCW);

	glEnable(GL_CULL_FACE);

	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);

	glShadeModel(GL_SMOOTH);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Set the global ambient light
	float amb[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);

	// Some hints
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// Set the framebuffer clear color
	glClearColor(0.5f, 0.5f, 0.5f, 0.5f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearDepth(1.0);

	// Enable scissor test
	glEnable(GL_SCISSOR_TEST);

	// Disable dithering
	glDisable(GL_DITHER);

	Log::I("OpenGL Renderer initialized.");
}

//-----------------------------------------------------------------------------
GLRenderer::~GLRenderer()
{
}

//-----------------------------------------------------------------------------
void GLRenderer::set_viewport(const Rect& absArea)
{
	glViewport((int32_t)absArea.min.x, (int32_t)absArea.min.y, (int32_t)absArea.max.x, (int32_t)absArea.max.y);
	glScissor((int32_t)absArea.min.x, (int32_t)absArea.min.y, (int32_t)absArea.max.x, (int32_t)absArea.max.y);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_clear_color(const Color4& color)
{
	glClearColor(color.r, color.g, color.b, color.a);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_material_params(const Color4& ambient, const Color4& diffuse, const Color4& specular,
				const Color4& emission, int32_t shininess)
{
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, &ambient.r);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, &diffuse.r);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, &specular.r);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, &emission.r);
	glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_ambient_light(const Color4& color)
{
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, color.to_float_ptr());
}

//-----------------------------------------------------------------------------
void GLRenderer::set_lighting(bool lighting)
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
void GLRenderer::set_texturing(uint32_t unit, bool texturing)
{
	if (!activate_texture_unit(unit))
		return;

	if (texturing)
	{
		glEnable(mTextureUnitTarget[unit]);
	}
	else
	{
		glDisable(mTextureUnitTarget[unit]);
	}
}

//-----------------------------------------------------------------------------
void GLRenderer::set_texture(uint32_t unit, TextureId texture)
{
	if (!activate_texture_unit(unit))
	{
		return;
	}

	mTextureUnitTarget[unit] = GL_TEXTURE_2D;
	mTextureUnit[unit] = m_textures[texture.index].texture_object;

	glEnable(mTextureUnitTarget[unit]);
	glBindTexture(mTextureUnitTarget[unit], mTextureUnit[unit]);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_texture_mode(uint32_t unit, TextureMode mode, const Color4& blendColor)
{
	if (!activate_texture_unit(unit))
		return;

	GLint envMode = GL::GetTextureMode(mode);

	if (envMode == GL_BLEND)
	{
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, &blendColor.r);
	}

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, envMode);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_texture_wrap(uint32_t unit, TextureWrap wrap)
{
	GLenum glWrap = GL::GetTextureWrap(wrap);

	glTexParameteri(mTextureUnitTarget[unit], GL_TEXTURE_WRAP_S, glWrap);
	glTexParameteri(mTextureUnitTarget[unit], GL_TEXTURE_WRAP_T, glWrap);
	glTexParameteri(mTextureUnitTarget[unit], GL_TEXTURE_WRAP_R, glWrap);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_texture_filter(uint32_t unit, TextureFilter filter)
{
	if (!activate_texture_unit(unit))
		return;

	GLint minFilter;
	GLint magFilter;

	GL::GetTextureFilter(filter, minFilter, magFilter);

	glTexParameteri(mTextureUnitTarget[unit], GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(mTextureUnitTarget[unit], GL_TEXTURE_MAG_FILTER, magFilter);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_backface_culling(bool culling)
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
void GLRenderer::set_separate_specular_color(bool separate)
{
	if (separate)
	{
		glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
	}
	else
	{
		glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SINGLE_COLOR);
	}
}

//-----------------------------------------------------------------------------
void GLRenderer::set_depth_test(bool test)
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
void GLRenderer::set_depth_write(bool write)
{
	glDepthMask((GLboolean) write);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_depth_func(CompareFunction func)
{
	GLenum glFunc = GL::GetCompareFunction(func);

	glDepthFunc(glFunc);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_rescale_normals(bool rescale)
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
void GLRenderer::set_blending(bool blending)
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
void GLRenderer::set_blending_params(BlendEquation equation, BlendFunction src, BlendFunction dst, const Color4& color)
{
	GLenum glEquation = GL::GetBlendEquation(equation);

	glBlendEquation(glEquation);

	GLenum glSrcFactor = GL::GetBlendFunction(src);
	GLenum glDstFactor = GL::GetBlendFunction(dst);

	glBlendFunc(glSrcFactor, glDstFactor);

	glBlendColor(color.r, color.g, color.b, color.a);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_color_write(bool write)
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
void GLRenderer::set_fog(bool fog)
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
void GLRenderer::set_fog_params(FogMode mode, float density, float start, float end, const Color4& color)
{
	GLenum glMode = GL::GetFogMode(mode);

	glFogi(GL_FOG_MODE, glMode);
	glFogf(GL_FOG_DENSITY, density);
	glFogf(GL_FOG_START, start);
	glFogf(GL_FOG_END, end);
	glFogfv(GL_FOG_COLOR, &color.r);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_alpha_test(bool test)
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
void GLRenderer::set_alpha_params(CompareFunction func, float ref)
{
	GLenum glFunc = GL::GetCompareFunction(func);

	glAlphaFunc(glFunc, ref);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_shading_type(ShadingType type)
{
	GLenum glMode = GL_SMOOTH;

	if (type == ST_FLAT)
	{
		glMode = GL_FLAT;
	}

	glShadeModel(glMode);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_polygon_mode(PolygonMode mode)
{
	GLenum glMode = GL::GetPolygonMode(mode);

	glPolygonMode(GL_FRONT_AND_BACK, glMode);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_front_face(FrontFace face)
{
	GLenum glFace = GL_CCW;

	if (face == FF_CW)
	{
		glFace = GL_CW;
	}

	glFrontFace(glFace);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_viewport_params(int32_t x, int32_t y, int32_t width, int32_t height)
{
	glViewport(x, y, width, height);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_scissor(bool scissor)
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
void GLRenderer::set_scissor_params(int32_t x, int32_t y, int32_t width, int32_t height)
{
	glScissor(x, y, width, height);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_point_sprite(bool sprite)
{
	if (sprite)
	{
		glEnable(GL_POINT_SPRITE);
		glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
	}
	else
	{
		glDisable(GL_POINT_SPRITE);
		glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_FALSE);
	}
}

//-----------------------------------------------------------------------------
void GLRenderer::set_point_size(float size)
{
	glPointSize(size);
}

//-----------------------------------------------------------------------------
void GLRenderer::set_point_params(float min, float max)
{
	glPointParameterf(GL_POINT_SIZE_MIN, min);
	glPointParameterf(GL_POINT_SIZE_MAX, max);
}

//-----------------------------------------------------------------------------
void GLRenderer::begin_frame()
{
	// Clear frame/depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

//-----------------------------------------------------------------------------
void GLRenderer::end_frame()
{
	glFinish();

	check_gl_errors();
}

//-----------------------------------------------------------------------------
Mat4 GLRenderer::get_matrix(MatrixType type) const
{
	return mMatrix[type];
}

//-----------------------------------------------------------------------------
void GLRenderer::set_matrix(MatrixType type, const Mat4& matrix)
{
	mMatrix[type] = matrix;

	switch (type)
	{
		case MT_VIEW:
		case MT_MODEL:
			glMatrixMode(GL_MODELVIEW);
			// Transformations must be listed in reverse order
			glLoadMatrixf((mMatrix[MT_VIEW] * mMatrix[MT_MODEL]).to_float_ptr());
			break;
		case MT_PROJECTION:
			glMatrixMode(GL_PROJECTION);
			glLoadMatrixf(mMatrix[MT_PROJECTION].to_float_ptr());
			break;
		case MT_TEXTURE:
			glMatrixMode(GL_TEXTURE);
			glLoadMatrixf(mMatrix[MT_TEXTURE].to_float_ptr());
			break;
		case MT_COLOR:
			//glMatrixMode(GL_COLOR);
			//glLoadMatrixf(mMatrix[MT_COLOR].to_float_ptr());
			break;
		default:
			break;
	}
}

//-----------------------------------------------------------------------------
void GLRenderer::push_matrix()
{
	assert(mModelMatrixStackIndex != MAX_MODEL_MATRIX_STACK_DEPTH);

	//Copy the current matrix into the stack, and move to the next location
	glGetFloatv(GL_MODELVIEW_MATRIX, mModelMatrixStack[mModelMatrixStackIndex].to_float_ptr());
	mModelMatrixStackIndex++;
}

//-----------------------------------------------------------------------------
void GLRenderer::pop_matrix()
{
	// Note: Is checking for push-pop count necessary? Maybe it should signal matrix stack underflow.
	//glPopMatrix();
	assert(mModelMatrixStackIndex > 0);

	mModelMatrixStackIndex--;
	set_matrix(MT_MODEL, mModelMatrixStack[mModelMatrixStackIndex]);
}

//-----------------------------------------------------------------------------
void GLRenderer::select_matrix(MatrixType type)
{
	switch (type)
	{
		case MT_VIEW:
		case MT_MODEL:
			glMatrixMode(GL_MODELVIEW);
			break;
		case MT_PROJECTION:
			glMatrixMode(GL_PROJECTION);
			break;
		case MT_TEXTURE:
			glMatrixMode(GL_TEXTURE);
			break;
		case MT_COLOR:
			glMatrixMode(GL_COLOR);
			break;
		default:
			break;
	}
}

//-----------------------------------------------------------------------------
void GLRenderer::render_vertex_index_buffer(const VertexBuffer* vertices, const IndexBuffer* indices)
{
	assert(vertices != NULL);
	assert(indices != NULL);

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
void GLRenderer::render_point_buffer(const VertexBuffer* buffer)
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
void GLRenderer::set_scissor_box(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	int32_t vals[4];
	glGetIntegerv(GL_VIEWPORT, vals);
	glScissor(x, vals[3] - y - height, width, height);
}

//-----------------------------------------------------------------------------
void GLRenderer::get_scissor_box(uint32_t& x, uint32_t& y, uint32_t& width, uint32_t& height)
{
	int32_t vals[4];
	glGetIntegerv(GL_SCISSOR_BOX, vals);
	int32_t valsViewport[4];
	glGetIntegerv(GL_VIEWPORT, valsViewport);
	x = vals[0];
	width = vals[2];
	height = vals[3];
	y = valsViewport[3] - vals[1] - height;
}

//-----------------------------------------------------------------------------
void GLRenderer::draw_rectangle(const Point2& position, const Point2& dimensions, int32_t drawMode,
															 const Color4& borderColor, const Color4& fillColor)
{
	if (drawMode & DM_FILL)
	{
		glBegin(GL_QUADS);

		glColor4f(fillColor.r, fillColor.g, fillColor.b, fillColor.a);

		glVertex3i(position.x								, position.y							 , 0);
		glVertex3i(position.x + dimensions.x, position.y							 , 0);
		glVertex3i(position.x + dimensions.x, position.y + dimensions.y, 0);
		glVertex3i(position.x								, position.y + dimensions.y, 0);
		
		glEnd();
	}

	if (drawMode & DM_BORDER)
	{
		glBegin(GL_LINE_LOOP);

		glColor4f(borderColor.r, borderColor.g, borderColor.b, borderColor.a);

		glVertex3i(position.x										, position.y									 , 0);
		glVertex3i(position.x										, position.y + dimensions.y - 1, 0);
		glVertex3i(position.x + dimensions.x - 1, position.y + dimensions.y - 1, 0);
		glVertex3i(position.x + dimensions.x - 1, position.y									 , 0);

		glEnd();
	}
}

//-----------------------------------------------------------------------------
bool GLRenderer::activate_texture_unit(uint32_t unit)
{
	if (unit >= (uint32_t) mMaxTextureUnits)
	{
		return false;
	}

	glActiveTexture(GL_TEXTURE0 + unit);
	mActiveTextureUnit = unit;

	return true;
}

//-----------------------------------------------------------------------------
void GLRenderer::set_light(uint32_t light, bool active)
{
	if (light >= (uint32_t) mMaxLights)
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
void GLRenderer::set_light_params(uint32_t light, LightType type, const Vec3& position)
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
void GLRenderer::set_light_color(uint32_t light, const Color4& ambient, const Color4& diffuse, const Color4& specular)
{
	glLightfv(GL_LIGHT0 + light, GL_AMBIENT, ambient.to_float_ptr());
	glLightfv(GL_LIGHT0 + light, GL_DIFFUSE, diffuse.to_float_ptr());
	glLightfv(GL_LIGHT0 + light, GL_SPECULAR, specular.to_float_ptr());
}

//-----------------------------------------------------------------------------
void GLRenderer::set_light_attenuation(uint32_t light, float constant, float linear, float quadratic)
{
	glLightf(GL_LIGHT0 + light, GL_CONSTANT_ATTENUATION, constant);
	glLightf(GL_LIGHT0 + light, GL_LINEAR_ATTENUATION, linear);
	glLightf(GL_LIGHT0 + light, GL_QUADRATIC_ATTENUATION, quadratic);
}

//-----------------------------------------------------------------------------
TextureId GLRenderer::load_texture(TextureResource* texture)
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

	GLint gl_texture_format = GL::GetPixelFormat(texture->format());

	// FIXME FIXME FIXME
	//if (mGenerateMipMaps)
	//{
	//	glTexParameteri(target, GL_GENERATE_MIPMAP, GL_TRUE);
	//}

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
void GLRenderer::unload_texture(TextureResource* texture)
{

}

//-----------------------------------------------------------------------------
TextureId GLRenderer::reload_texture(TextureResource* old_texture, TextureResource* new_texture)
{

}

//-----------------------------------------------------------------------------
void GLRenderer::check_gl_errors()
{
	GLenum error;

	while ((error = glGetError()))
	{
		switch (error)
		{
			case GL_INVALID_ENUM:
				Log::E("GLRenderer: GL_INVALID_ENUM");
				break;
			case GL_INVALID_VALUE:
				Log::E("GLRenderer: GL_INVALID_VALUE");
				break;
			case GL_INVALID_OPERATION:
				Log::E("GLRenderer: GL_INVALID_OPERATION");
				break;
			case GL_STACK_OVERFLOW:
				Log::E("GLRenderer: GL_STACK_OVERFLOW");
				break;
			case GL_STACK_UNDERFLOW:
				Log::E("GLRenderer: GL_STACK_UNDERFLOW");
				break;
			case GL_OUT_OF_MEMORY:
				Log::E("GLRenderer: GL_OUT_OF_MEMORY");
				break;
			case GL_TABLE_TOO_LARGE:
				Log::E("GLRenderer: GL_OUT_OF_MEMORY");
				break;
		}
	}
}

void GLRenderer::add_debug_line(const Vec3& start, const Vec3& end, const Color4& color)
{
	if (mLinesCount < 256)
	{
		mLinesData[mLinesCount].start		= start;
		mLinesData[mLinesCount].c1			= color;
		mLinesData[mLinesCount].end			= end;
		mLinesData[mLinesCount].c2			= color;
		mLinesCount++;
	}
}

void GLRenderer::draw_debug_lines()
{
	if (mLinesCount == 0)
	{
		return;
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

		glVertexPointer(3, GL_FLOAT, sizeof(float) * 7, &mLinesData[0].start);
		glColorPointer(4, GL_FLOAT, sizeof(float) * 7, &mLinesData[0].c1);

		glDrawArrays(GL_LINES, 0, mLinesCount * 4);

	glDisableClientState(GL_VERTEX_ARRAY);

	mLinesCount = 0;
}

} // namespace crown
