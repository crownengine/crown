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

#include "Material.h"
#include "Types.h"

namespace crown
{

MaterialResource::MaterialResource() :
	mAmbient(0.5f, 0.5f, 0.5f, 1.0f),
	mDiffuse(0.5f, 0.5f, 0.5f, 1.0f),
	mSpecular(0.5f, 0.5f, 0.5f, 1.0f),
	mEmission(0.0f, 0.0f, 0.0f, 1.0f),
	mShininess(0),
	mLighting(true),
	mTexturing(false),
	mBackfaceCulling(true),
	mSeparateSpecularColor(true),
	mDepthTest(true),
	mDepthWrite(true),
	mRescaleNormals(false),
	mBlending(false),
	mColorWrite(true),
	mFog(false),
	mAlphaTest(false),
	mPointSprite(false),
	mShadingType(ST_SMOOTH),
	mPolygonMode(PM_FILL),
	mFrontFace(FF_CCW),
	mDepthFunc(CF_LEQUAL),
	mFogMode(FM_LINEAR),
	mFogDensity(1.0f),
	mFogStart(0.0f),
	mFogEnd(1.0f),
	mFogColor(0.0f, 0.0f, 0.0f, 0.0f),
	mAlphaFunc(CF_ALWAYS),
	mAlphaRef(0.0f),
	mPointSize(1.0f),
	mPointSizeMin(0.0f),
	mPointSizeMax(1.0f),
	mBlendEquation(BE_FUNC_ADD),
	mBlendSrc(BF_SRC_ALPHA),
	mBlendDst(BF_ONE_MINUS_SRC_ALPHA),
	mBlendColor(0.0f, 0.0f, 0.0f, 0.0f)
{
//	for (uint32_t i = 0; i < MAX_TEXTURE_LAYERS; i++)
//	{
//		mTextureLayer[i] = 0;
//	}
}

MaterialResource::~MaterialResource()
{
}

const Color4& MaterialResource::GetAmbient() const
{
	return mAmbient;
}

void MaterialResource::SetAmbient(const Color4& ambient)
{
	mAmbient = ambient;
}

const Color4& MaterialResource::GetDiffuse() const
{
	return mDiffuse;
}

void MaterialResource::SetDiffuse(const Color4& diffuse)
{
	mDiffuse = diffuse;
}

const Color4& MaterialResource::GetSpecular() const
{
	return mSpecular;
}

void MaterialResource::SetSpecular(const Color4& specular)
{
	mSpecular = specular;
}

const Color4& MaterialResource::GetEmission() const
{
	return mEmission;
}

void MaterialResource::SetEmission(const Color4& emission)
{
	mEmission = emission;
}

int32_t MaterialResource::GetShininess() const
{
	return mShininess;
}

void MaterialResource::SetShininess(int32_t shininess)
{
	mShininess = shininess;
}

bool MaterialResource::GetLighting() const
{
	return mLighting;
}

void MaterialResource::SetLighting(bool lighting)
{
	mLighting = lighting;
}

bool MaterialResource::GetTexturing() const
{
	return mTexturing;
}

void MaterialResource::SetTexturing(bool texturing)
{
	mTexturing = texturing;
}

bool MaterialResource::GetBackfaceCulling() const
{
	return mBackfaceCulling;
}

void MaterialResource::SetBackfaceCulling(bool culling)
{
	mBackfaceCulling = culling;
}

bool MaterialResource::GetSeparateSpecularColor() const
{
	return mSeparateSpecularColor;
}

void MaterialResource::SetSeparateSpecularColor(bool separate)
{
	mSeparateSpecularColor = separate;
}

bool MaterialResource::GetDepthTest() const
{
	return mDepthTest;
}

void MaterialResource::SetDepthTest(bool test)
{
	mDepthTest = test;
}

bool MaterialResource::GetDepthWrite() const
{
	return mDepthWrite;
}

void MaterialResource::SetDepthWrite(bool write)
{
	mDepthWrite = write;
}

bool MaterialResource::GetRescaleNormals() const
{
	return mRescaleNormals;
}

void MaterialResource::SetRescaleNormals(bool rescale)
{
	mRescaleNormals = rescale;
}

bool MaterialResource::GetBlending() const
{
	return mBlending;
}

void MaterialResource::SetBlending(bool blending)
{
	mBlending = blending;
}

bool MaterialResource::GetColorWrite() const
{
	return mColorWrite;
}

void MaterialResource::SetColorWrite(bool write)
{
	mColorWrite = write;
}

bool MaterialResource::GetFog() const
{
	return mFog;
}

void MaterialResource::SetFog(bool fog)
{
	mFog = fog;
}

bool MaterialResource::GetAlphaTest() const
{
	return mAlphaTest;
}

void MaterialResource::SetAlphaTest(bool test)
{
	mAlphaTest = test;
}

bool MaterialResource::GetPointSprite() const
{
	return mPointSprite;
}

void MaterialResource::SetPointSprite(bool sprite)
{
	mPointSprite = sprite;
}

ShadingType MaterialResource::GetShadingType() const
{
	return mShadingType;
}

void MaterialResource::SetShadingType(ShadingType type)
{
	mShadingType = type;
}

PolygonMode MaterialResource::GetPolygonMode() const
{
	return mPolygonMode;
}

void MaterialResource::SetPolygonMode(PolygonMode mode)
{
	mPolygonMode = mode;
}

FrontFace MaterialResource::GetFrontFace() const
{
	return mFrontFace;
}

void MaterialResource::SetFrontFace(FrontFace front)
{
	mFrontFace = front;
}

CompareFunction MaterialResource::GetDepthFunc() const
{
	return mDepthFunc;
}

void MaterialResource::SetDepthFunc(CompareFunction func)
{
	mDepthFunc = func;
}

FogMode MaterialResource::GetFogMode() const
{
	return mFogMode;
}

void MaterialResource::SetFogMode(FogMode mode)
{
	mFogMode = mode;
}

float MaterialResource::GetFogDensity() const
{
	return mFogDensity;
}

void MaterialResource::SetFogDensity(float density)
{
	mFogDensity = density;
}

float MaterialResource::GetFogStart() const
{
	return mFogStart;
}

void MaterialResource::SetFogStart(float start)
{
	mFogStart = start;
}

float MaterialResource::GetFogEnd() const
{
	return mFogEnd;
}

void MaterialResource::SetFogEnd(float end)
{
	mFogEnd = end;
}

const Color4& MaterialResource::GetFogColor() const
{
	return mFogColor;
}

void MaterialResource::SetFogColor(const Color4& color)
{
	mFogColor = color;
}

CompareFunction MaterialResource::GetAlphaFunc() const
{
	return mAlphaFunc;
}

void MaterialResource::SetAlphaFunction(CompareFunction func)
{
	mAlphaFunc = func;
}

float MaterialResource::GetAlphaRef() const
{
	return mAlphaRef;
}

void MaterialResource::SetAlphaRef(float ref)
{
	mAlphaRef = ref;
}

float MaterialResource::GetPointSize() const
{
	return mPointSize;
}

void MaterialResource::SetPointSize(float size)
{
	mPointSize = size;
}

float MaterialResource::GetPointSizeMin() const
{
	return mPointSizeMin;
}

void MaterialResource::SetPointSizeMin(float min)
{
	mPointSizeMin = min;
}

float MaterialResource::GetPointSizeMax() const
{
	return mPointSizeMax;
}

void MaterialResource::SetPointSizeMax(float max)
{
	mPointSizeMax = max;
}

BlendFunction MaterialResource::GetSrcBlendFunc() const
{
	return mBlendSrc;
}

void MaterialResource::SetSrcBlendFunc(BlendFunction src)
{
	mBlendSrc = src;
}

BlendFunction MaterialResource::GetDstBlendFunc() const
{
	return mBlendDst;
}

void MaterialResource::SetDstBlendFunc(BlendFunction dst)
{
	mBlendDst = dst;
}

void MaterialResource::SetBlendFunc(BlendFunction src, BlendFunction dst)
{
	mBlendSrc = src;
	mBlendDst = dst;
}

Color4& MaterialResource::GetBlendColor()
{
	return mBlendColor;
}

void MaterialResource::SetBlendColor(const Color4& color)
{
	mBlendColor = color;
}

bool MaterialResource::SetTextureLayer(uint32_t layer, ResourceId texture)
{
	if (layer >= MAX_TEXTURE_LAYERS)
	{
		return false;
	}

	mTextureLayer[layer] = texture;
	return true;
}

ResourceId MaterialResource::GetTextureLayer(uint32_t layer) const
{
//	if (layer >= MAX_TEXTURE_LAYERS)
//	{
//		return 0;
//	}

	return mTextureLayer[layer];
}

void MaterialResource::SetTextureMode(TextureMode mode)
{
//	for (uint32_t i = 0; i < MAX_TEXTURE_LAYERS; i++)
//	{
//		if (mTextureLayer[i] == 0)
//			continue;

//		mTextureLayer[i]->SetMode(mode);
//	}
}

void MaterialResource::SetTextureFilter(TextureFilter filter)
{
//	for (uint32_t i = 0; i < MAX_TEXTURE_LAYERS; i++)
//	{
//		if (mTextureLayer[i] == 0)
//			continue;

//		mTextureLayer[i]->SetFilter(filter);
//	}
}

void MaterialResource::SetTextureWrap(TextureWrap wrap)
{
//	for (uint32_t i = 0; i < MAX_TEXTURE_LAYERS; i++)
//	{
//		if (mTextureLayer[i] == 0)
//			continue;

//		mTextureLayer[i]->SetWrap(wrap);
//	}
}

} // namespace crown

