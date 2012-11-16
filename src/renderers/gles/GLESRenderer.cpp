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

#include "GLESIndexBuffer.h"
#include "GLESRenderer.h"
#include "GLESSupport.h"
#include "GLESTexture.h"
#include "GLESTextureManager.h"
#include "GLESUtils.h"
#include "GLESVertexBuffer.h"
#include "Light.h"
#include "Log.h"
#include "Material.h"
#include "Types.h"
#include "Rect.h"
#include <GLES/gl.h>

namespace crown
{

GLESRenderer::GLESRenderer() :
	mGLESSupport(NULL),

	mMaxLights(0),
	mMaxTextureSize(0),
	mMaxTextureUnits(0),
	mMaxVertexIndices(0),
	mMaxVertexVertices(0),

	mMaxAnisotropy(0.0f),

	mActiveTextureUnit(0)
{
	mMinMaxPoint32_tSize[0] = 0.0f;
	mMinMaxPoint32_tSize[1] = 0.0f;

	for (uint32_t i = 0; i < MAX_TEXTURE_UNITS; i++)
	{
		mTextureUnit[i] = 0;
		mTextureUnitTarget[i] = GL_TEXTURE_2D;
	}

	// This code snippet initializes the matrices
	for (uint32_t i = 0; i < MT_COUNT; i++)
	{
		mMatrix[i].LoadIdentity();
	}

	mGLESSupport = GetGLESSupport();

	mGLESSupport->BuildExtensionList();

	glGetIntegerv(GL_MAX_LIGHTS, &mMaxLights);
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &mMaxTextureSize);
	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &mMaxTextureUnits);
	//glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &mMaxVertexIndices);
	//glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &mMaxVertexVertices);

	glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, &mMinMaxPoint32_tSize[0]);
	//glGetFloatv(GL_LINE_WIDTH_RANGE, &mMinMaxLineWidth[0]);

	Log::I("OpenGLES Vendor\t: %s", mGLSupport->GetVendor().c_str());
	Log::I("OpenGLES Renderer\t: %s", mGLSupport->GetRenderer().c_str());
	Log::I("OpenGLES Version\t: %s", mGLSupport->GetVersion().c_str());
	Log::I("Min Point32_t Size\t: %f", mMinMaxPoint32_tSize[0]);
	Log::I("Max Point32_t Size\t: %f", mMinMaxPoint32_tSize[1]);
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
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glFrontFace(GL_CCW);

	glEnable(GL_CULL_FACE);

	glShadeModel(GL_SMOOTH);

	// Set the global ambient light
	float amb[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);

	// Some hint32_ts
	glHint32_t(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	// Set the framebuffer clear color
	glClearColor(0.5f, 0.5f, 0.5f, 0.5f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// Enable scissor test
	glEnable(GL_SCISSOR_TEST);

	// Disable dithering
	glDisable(GL_DITHER);

	Log::I("OpenGLES Renderer initialized.");
}

GLESRenderer::~GLESRenderer()
{
	for (int32_t i = 0; i < mVertexBufferList.GetSize(); i++)
	{
		delete mVertexBufferList[i];
	}

	for (int32_t i = 0; i < mIndexBufferList.GetSize(); i++)
	{
		delete mIndexBufferList[i];
	}
}

void GLESRenderer::_SetViewport(const Rect& absArea)
{
	glViewport((int32_t)absArea.min.x, (int32_t)absArea.min.y, (int32_t)absArea.max.x, (int32_t)absArea.max.y);
	glScissor((int32_t)absArea.min.x, (int32_t)absArea.min.y, (int32_t)absArea.max.x, (int32_t)absArea.max.y);
}

void GLESRenderer::SetClearColor(const Color4& color)
{
	glClearColor(color.r, color.g, color.b, color.a);
}

void GLESRenderer::_SetMaterialParams(const Color4& ambient, const Color4& diffuse, const Color4& specular,
				const Color4& emission, int32_t shininess)
{
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, &ambient.r);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, &diffuse.r);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, &specular.r);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, &emission.r);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
}

void GLESRenderer::_SetLighting(bool lighting)
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

void GLESRenderer::_SetAmbientLight(const Color4& color)
{
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, color.ToFloatPtr());
}

void GLESRenderer::_SetTexturing(uint32_t unit, bool texturing)
{
	if (!ActivateTextureUnit(unit))
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

void GLESRenderer::_SetTexture(uint32_t unit, Texture* texture)
{
	if (!ActivateTextureUnit(unit))
		return;

	if (texture != NULL)
	{
		mTextureUnit[unit] = texture;
		mTextureUnitTarget[unit] = static_cast<const GLESTexture*>(texture)->GetGLTarget();

		glEnable(mTextureUnitTarget[unit]);
		glBindTexture(mTextureUnitTarget[unit], static_cast<const GLESTexture*>(texture)->GetGLObject());
	}
}

void GLESRenderer::_SetTextureMode(uint32_t unit, TextureMode mode, const Color4& /* blendColor */)
{
	/* No support for blend color, params will be ignored */
	if (!ActivateTextureUnit(unit))
		return;

	GLint envMode = GLES::GetTextureMode(mode);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, envMode);
}

void GLESRenderer::_SetTextureWrap(uint32_t unit, TextureWrap wrap)
{
	GLenum glWrap = GLES::GetTextureWrap(wrap);

	glTexParameteri(mTextureUnitTarget[unit], GL_TEXTURE_WRAP_S, glWrap);
	glTexParameteri(mTextureUnitTarget[unit], GL_TEXTURE_WRAP_T, glWrap);
}

void GLESRenderer::_SetTextureFilter(uint32_t unit, TextureFilter filter)
{
	if (!ActivateTextureUnit(unit))
		return;

	GLint minFilter;
	GLint magFilter;

	GLES::GetTextureFilter(filter, minFilter, magFilter);

	glTexParameteri(mTextureUnitTarget[unit], GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(mTextureUnitTarget[unit], GL_TEXTURE_MAG_FILTER, magFilter);
}

void GLESRenderer::_SetBackfaceCulling(bool culling)
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

void GLESRenderer::_SetSeparateSpecularColor(bool /* separate */)
{
	/* No support for separate specular color, params will be ignored */
	Log::W("Renderer does not support separate specular color");
}

void GLESRenderer::_SetDepthTest(bool test)
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

void GLESRenderer::_SetDepthWrite(bool write)
{
	glDepthMask((GLboolean) write);
}

void GLESRenderer::_SetDepthFunc(CompareFunction func)
{
	GLenum glFunc = GLES::GetCompareFunction(func);

	glDepthFunc(glFunc);
}

void GLESRenderer::_SetRescaleNormals(bool rescale)
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

void GLESRenderer::_SetBlending(bool blending)
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

void GLESRenderer::_SetBlendingParams(BlendEquation /* equation */, BlendFunction src, BlendFunction dst, const Color4& /* color */)
{
	/* No support for blend equation, params will be ignored */
	/* No support for blend color, params will be ignored */

	GLenum glSrcFactor = GLES::GetBlendFunction(src);
	GLenum glDstFactor = GLES::GetBlendFunction(dst);

	glBlendFunc(glSrcFactor, glDstFactor);
}

void GLESRenderer::_SetColorWrite(bool write)
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

void GLESRenderer::_SetFog(bool fog)
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

void GLESRenderer::_SetFogParams(FogMode mode, float density, float start, float end, const Color4& color)
{
	GLenum glMode = GLES::GetFogMode(mode);

	glFogf(GL_FOG_MODE, glMode);
	glFogf(GL_FOG_DENSITY, density);
	glFogf(GL_FOG_START, start);
	glFogf(GL_FOG_END, end);
	glFogfv(GL_FOG_COLOR, &color.r);
}

void GLESRenderer::_SetAlphaTest(bool test)
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

void GLESRenderer::_SetAlphaParams(CompareFunction func, float ref)
{
	GLenum glFunc = GLES::GetCompareFunction(func);

	glAlphaFunc(glFunc, ref);
}

void GLESRenderer::_SetShadingType(ShadingType type)
{
	GLenum glMode = GL_SMOOTH;

	if (type == ST_FLAT)
	{
		glMode = GL_FLAT;
	}

	glShadeModel(glMode);
}

void GLESRenderer::_SetPolygonMode(PolygonMode /* mode */)
{
	/* No support for polygon mode, params will be ignored */
	Log::W("Renderer does not support PolygonMode");
}

void GLESRenderer::_SetFrontFace(FrontFace face)
{
	GLenum glFace = GL_CCW;

	if (face == FF_CW)
	{
		glFace = GL_CW;
	}

	glFrontFace(glFace);
}

void GLESRenderer::_SetViewportParams(int32_t x, int32_t y, int32_t width, int32_t height)
{
	glViewport(x, y, width, height);
}

void GLESRenderer::_SetScissor(bool scissor)
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

void GLESRenderer::_SetScissorParams(int32_t x, int32_t y, int32_t width, int32_t height)
{
	glScissor(x, y, width, height);
}

void GLESRenderer::_SetPoint32_tSprite(bool sprite)
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

void GLESRenderer::_SetPoint32_tSize(float size)
{
	glPoint32_tSize(size);
}

void GLESRenderer::_SetPoint32_tParams(float min, float max)
{
	glPoint32_tParameterf(GL_POINT_SIZE_MIN, min);
	glPoint32_tParameterf(GL_POINT_SIZE_MAX, max);
}

void GLESRenderer::_BeginFrame()
{
	// Clear frame/depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GLESRenderer::_EndFrame()
{
	glFinish();

	CheckGLErrors();
}

Mat4 GLESRenderer::GetMatrix(MatrixType type) const
{
	return mMatrix[type];
}

void GLESRenderer::SetMatrix(MatrixType type, const Mat4& matrix)
{
	mMatrix[type] = matrix;

	switch (type)
	{
		case MT_VIEW:
		case MT_MODEL:
			glMatrixMode(GL_MODELVIEW);
			// Transformations must be listed in reverse order
			glLoadMatrixf((mMatrix[MT_VIEW] * mMatrix[MT_MODEL]).ToFloatPtr());
			break;
		case MT_PROJECTION:
			glMatrixMode(GL_PROJECTION);
			glLoadMatrixf(mMatrix[MT_PROJECTION].ToFloatPtr());
			break;
		case MT_TEXTURE:
			glMatrixMode(GL_TEXTURE);
			glLoadMatrixf(mMatrix[MT_TEXTURE].ToFloatPtr());
			break;
		case MT_COLOR:
			//glMatrixMode(GL_COLOR);
			//glLoadMatrixf(mMatrix[MT_COLOR].ToFloatPtr());
			break;
		default:
			break;
	}
}

void GLESRenderer::PushMatrix()
{
	glPushMatrix();
}

void GLESRenderer::PopMatrix()
{
	// Note: Is checking for push-pop count necessary? Maybe it should signal matrix stack underflow.
	glPopMatrix();
}

void GLESRenderer::SelectMatrix(MatrixType type)
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
			//glMatrixMode(GL_COLOR);
			break;
		default:
			break;
	}
}

void GLESRenderer::RenderVertexIndexBuffer(const VertexBuffer* vertices, const IndexBuffer* indices)
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

void GLESRenderer::RenderPoint32_tBuffer(const VertexBuffer* buffer)
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

OcclusionQuery* GLESRenderer::CreateOcclusionQuery()
{
	Log::W("Renderer does not support OcclusionQuery");
	return 0;
}

VertexBuffer*  GLESRenderer::CreateVertexBuffer()
{
	VertexBuffer* vb = new GLESVertexBuffer();

	if (!vb)
		return 0;

	mVertexBufferList.Append(vb);
	return vb;
}

IndexBuffer*  GLESRenderer::CreateIndexBuffer()
{
	IndexBuffer* ib = new GLESIndexBuffer();

	if (!ib)
		return 0;

	mIndexBufferList.Append(ib);
	return ib;
}

void GLESRenderer::SetTexture(uint32_t layer, Texture* texture)
{
	if (texture == NULL)
		return;

	_SetTexturing(layer, true);
	_SetTexture(layer, texture);
	_SetTextureMode(layer, texture->GetMode(), texture->GetBlendColor());
	_SetTextureWrap(layer, texture->GetWrap());
	_SetTextureFilter(layer, texture->GetFilter());
}

void GLESRenderer::SetScissorBox(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	int32_t vals[4];
	glGetIntegerv(GL_VIEWPORT, vals);
	glScissor(x, vals[3] - y - height, width, height);
}

void GLESRenderer::GetScissorBox(uint32_t& x, uint32_t& y, uint32_t& width, uint32_t& height)
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

void GLESRenderer::DrawRectangle(const Point32_t2& position, const Point32_t2& dimensions, int32_t drawMode,
														 const Color4& borderColor, const Color4& fillColor)
{

}

bool GLESRenderer::ActivateTextureUnit(uint32_t unit)
{
	if (unit >= (uint32_t) mMaxTextureUnits)
		return false;

	glActiveTexture(GL_TEXTURE0 + unit);
	mActiveTextureUnit = unit;
	return true;
}

//-----------------------------------------------------------------------------
void GLESRenderer::_SetLight(uint32_t light, bool active)
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
void GLESRenderer::_SetLightParams(uint32_t light, LightType type, const Vec3& position)
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
void GLESRenderer::_SetLightColor(uint32_t light, const Color4& ambient, const Color4& diffuse, const Color4& specular)
{
	glLightfv(GL_LIGHT0 + light, GL_AMBIENT, ambient.ToFloatPtr());
	glLightfv(GL_LIGHT0 + light, GL_DIFFUSE, diffuse.ToFloatPtr());
	glLightfv(GL_LIGHT0 + light, GL_SPECULAR, specular.ToFloatPtr());
}

//-----------------------------------------------------------------------------
void GLESRenderer::_SetLightAttenuation(uint32_t light, float constant, float linear, float quadratic)
{
	glLightf(GL_LIGHT0 + light, GL_CONSTANT_ATTENUATION, constant);
	glLightf(GL_LIGHT0 + light, GL_LINEAR_ATTENUATION, linear);
	glLightf(GL_LIGHT0 + light, GL_QUADRATIC_ATTENUATION, quadratic);
}


void GLESRenderer::CheckGLErrors()
{
	GLenum error;

	while ((error = glGetError()))
	{
		switch (error)
		{
			case GL_INVALID_ENUM:
				Log::E("GLESRenderer: GL_INVALID_ENUM");
				break;
			case GL_INVALID_VALUE:
				Log::E("GLESRenderer: GL_INVALID_VALUE");
				break;
			case GL_INVALID_OPERATION:
				Log::E("GLESRenderer: GL_INVALID_OPERATION");
				break;
			case GL_STACK_OVERFLOW:
				Log::E("GLESRenderer: GL_STACK_OVERFLOW");
				break;
			case GL_STACK_UNDERFLOW:
				Log::E("GLESRenderer: GL_STACK_UNDERFLOW");
				break;
			case GL_OUT_OF_MEMORY:
				Log::E("GLESRenderer: GL_OUT_OF_MEMORY");
				break;
		}
	}
}

} // namespace crown

