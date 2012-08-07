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

#pragma once

#include <GLES/gl.h>
#include "List.h"
#include "Renderer.h"

#define MAX_TEXTURE_UNITS 8

namespace Crown
{

class GLESSupport;

/**
 * OpenGL|ES 1.1 renderer.
 */
class GLESRenderer : public Renderer
{
	typedef List<VertexBuffer*> VertexBufferList;
	typedef List<IndexBuffer*> IndexBufferList;

public:

						GLESRenderer();
						~GLESRenderer();

	void				_BeginFrame();
	void				_EndFrame();

	void				SetClearColor(const Color4& color);
	void				_SetViewport(const Rect& absArea);
	void				_SetMaterialParams(const Color4& ambient, const Color4& diffuse, const Color4& specular, const Color4& emission, int shininess);
	void				_SetLighting(bool lighting);
	void				_SetAmbientLight(const Color4& color);

	void				_SetTexturing(uint unit, bool texturing);
	void				_SetTexture(uint unit, Texture* texture);
	void				_SetTextureMode(uint unit, TextureMode mode, const Color4& blendColor);
	void				_SetTextureWrap(uint unit, TextureWrap wrap);
	void				_SetTextureFilter(uint unit, TextureFilter filter);

	void				_SetLight(uint light, bool active);
	void				_SetLightParams(uint light, LightType type, const Vec3& position);
	void				_SetLightColor(uint light, const Color4& ambient, const Color4& diffuse, const Color4& specular);
	void				_SetLightAttenuation(uint light, float constant, float linear, float quadratic);

	void				_SetBackfaceCulling(bool culling);

	void				_SetSeparateSpecularColor(bool separate);

	void				_SetDepthTest(bool test);
	void				_SetDepthWrite(bool write);
	void				_SetDepthFunc(CompareFunction func);

	void				_SetRescaleNormals(bool rescale);

	void				_SetBlending(bool blending);
	void				_SetBlendingParams(BlendEquation equation, BlendFunction src, BlendFunction dst, const Color4& color);
	void				_SetColorWrite(bool write);

	void				_SetFog(bool fog);
	void				_SetFogParams(FogMode mode, float density, float start, float end, const Color4& color);

	void				_SetAlphaTest(bool test);
	void				_SetAlphaParams(CompareFunction func, float ref);

	void				_SetShadingType(ShadingType type);
	void				_SetPolygonMode(PolygonMode mode);
	void				_SetFrontFace(FrontFace face);

	void				_SetViewportParams(int x, int y, int width, int height);

	void				_SetScissor(bool scissor);
	void				_SetScissorParams(int x, int y, int width, int height);

	void				_SetPointSprite(bool sprite);
	void				_SetPointSize(float size);
	void				_SetPointParams(float min, float max);

	VertexBuffer*		CreateVertexBuffer();
	IndexBuffer*		CreateIndexBuffer();

	OcclusionQuery*		CreateOcclusionQuery();

	void				SetTexture(uint layer, Texture* texture);

	Mat4				GetMatrix(MatrixType type) const;
	void				SetMatrix(MatrixType type, const Mat4& matrix);

	void				PushMatrix();
	void				PopMatrix();

	void				SelectMatrix(MatrixType type);

	void				RenderVertexIndexBuffer(const VertexBuffer* vertices, const IndexBuffer* indices);
	void				RenderPointBuffer(const VertexBuffer* buffer);

	void				SetScissorBox(uint x, uint y, uint width, uint height);
	void				GetScissorBox(uint& x, uint& y, uint& width, uint& height);
	void				DrawRectangle(const Point2& position, const Point2& dimensions, int drawMode,
														 const Color4& borderColor, const Color4& fillColor);

	void				AddDebugLine(const Vec3& start, const Vec3& end, const Color4& color) {}
	void				DrawDebugLines() {}

private:

	bool				ActivateTextureUnit(uint unit);		//!< Activates a texture unit and returns true if succes

	void				CheckGLErrors();

	GLESSupport*		mGLESSupport;

	Mat4				mMatrix[MT_COUNT];

	//Mat4				mModelMatrixStack[MAX_MODEL_MATRIX_STACK_DEPTH];
	//uint				mModelMatrixStackIndex;

	int					mMaxLights;
	int					mMaxTextureSize;
	int					mMaxTextureUnits;
	int					mMaxVertexIndices;
	int					mMaxVertexVertices;

	float				mMaxAnisotropy;
	float				mMinMaxPointSize[2];
	float				mMinMaxLineWidth[2];

	VertexBufferList	mVertexBufferList;
	IndexBufferList		mIndexBufferList;

	int					mActiveTextureUnit;
	Texture*			mTextureUnit[MAX_TEXTURE_UNITS];
	GLenum				mTextureUnitTarget[MAX_TEXTURE_UNITS];
};

} // namespace Crown

