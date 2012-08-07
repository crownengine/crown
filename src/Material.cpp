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

namespace Crown
{

Material::Material() :
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
	for (uint i = 0; i < MAX_TEXTURE_LAYERS; i++)
	{
		mTextureLayer[i] = 0;
	}
}

Material::~Material()
{
}

const Color4& Material::GetAmbient() const
{
	return mAmbient;
}

void Material::SetAmbient(const Color4& ambient)
{
	mAmbient = ambient;
}

const Color4& Material::GetDiffuse() const
{
	return mDiffuse;
}

void Material::SetDiffuse(const Color4& diffuse)
{
	mDiffuse = diffuse;
}

const Color4& Material::GetSpecular() const
{
	return mSpecular;
}

void Material::SetSpecular(const Color4& specular)
{
	mSpecular = specular;
}

const Color4& Material::GetEmission() const
{
	return mEmission;
}

void Material::SetEmission(const Color4& emission)
{
	mEmission = emission;
}

int Material::GetShininess() const
{
	return mShininess;
}

void Material::SetShininess(int shininess)
{
	mShininess = shininess;
}

bool Material::GetLighting() const
{
	return mLighting;
}

void Material::SetLighting(bool lighting)
{
	mLighting = lighting;
}

bool Material::GetTexturing() const
{
	return mTexturing;
}

void Material::SetTexturing(bool texturing)
{
	mTexturing = texturing;
}

bool Material::GetBackfaceCulling() const
{
	return mBackfaceCulling;
}

void Material::SetBackfaceCulling(bool culling)
{
	mBackfaceCulling = culling;
}

bool Material::GetSeparateSpecularColor() const
{
	return mSeparateSpecularColor;
}

void Material::SetSeparateSpecularColor(bool separate)
{
	mSeparateSpecularColor = separate;
}

bool Material::GetDepthTest() const
{
	return mDepthTest;
}

void Material::SetDepthTest(bool test)
{
	mDepthTest = test;
}

bool Material::GetDepthWrite() const
{
	return mDepthWrite;
}

void Material::SetDepthWrite(bool write)
{
	mDepthWrite = write;
}

bool Material::GetRescaleNormals() const
{
	return mRescaleNormals;
}

void Material::SetRescaleNormals(bool rescale)
{
	mRescaleNormals = rescale;
}

bool Material::GetBlending() const
{
	return mBlending;
}

void Material::SetBlending(bool blending)
{
	mBlending = blending;
}

bool Material::GetColorWrite() const
{
	return mColorWrite;
}

void Material::SetColorWrite(bool write)
{
	mColorWrite = write;
}

bool Material::GetFog() const
{
	return mFog;
}

void Material::SetFog(bool fog)
{
	mFog = fog;
}

bool Material::GetAlphaTest() const
{
	return mAlphaTest;
}

void Material::SetAlphaTest(bool test)
{
	mAlphaTest = test;
}

bool Material::GetPointSprite() const
{
	return mPointSprite;
}

void Material::SetPointSprite(bool sprite)
{
	mPointSprite = sprite;
}

ShadingType Material::GetShadingType() const
{
	return mShadingType;
}

void Material::SetShadingType(ShadingType type)
{
	mShadingType = type;
}

PolygonMode Material::GetPolygonMode() const
{
	return mPolygonMode;
}

void Material::SetPolygonMode(PolygonMode mode)
{
	mPolygonMode = mode;
}

FrontFace Material::GetFrontFace() const
{
	return mFrontFace;
}

void Material::SetFrontFace(FrontFace front)
{
	mFrontFace = front;
}

CompareFunction Material::GetDepthFunc() const
{
	return mDepthFunc;
}

void Material::SetDepthFunc(CompareFunction func)
{
	mDepthFunc = func;
}

FogMode Material::GetFogMode() const
{
	return mFogMode;
}

void Material::SetFogMode(FogMode mode)
{
	mFogMode = mode;
}

float Material::GetFogDensity() const
{
	return mFogDensity;
}

void Material::SetFogDensity(float density)
{
	mFogDensity = density;
}

float Material::GetFogStart() const
{
	return mFogStart;
}

void Material::SetFogStart(float start)
{
	mFogStart = start;
}

float Material::GetFogEnd() const
{
	return mFogEnd;
}

void Material::SetFogEnd(float end)
{
	mFogEnd = end;
}

const Color4& Material::GetFogColor() const
{
	return mFogColor;
}

void Material::SetFogColor(const Color4& color)
{
	mFogColor = color;
}

CompareFunction Material::GetAlphaFunc() const
{
	return mAlphaFunc;
}

void Material::SetAlphaFunction(CompareFunction func)
{
	mAlphaFunc = func;
}

float Material::GetAlphaRef() const
{
	return mAlphaRef;
}

void Material::SetAlphaRef(float ref)
{
	mAlphaRef = ref;
}

float Material::GetPointSize() const
{
	return mPointSize;
}

void Material::SetPointSize(float size)
{
	mPointSize = size;
}

float Material::GetPointSizeMin() const
{
	return mPointSizeMin;
}

void Material::SetPointSizeMin(float min)
{
	mPointSizeMin = min;
}

float Material::GetPointSizeMax() const
{
	return mPointSizeMax;
}

void Material::SetPointSizeMax(float max)
{
	mPointSizeMax = max;
}

BlendFunction Material::GetSrcBlendFunc() const
{
	return mBlendSrc;
}

void Material::SetSrcBlendFunc(BlendFunction src)
{
	mBlendSrc = src;
}

BlendFunction Material::GetDstBlendFunc() const
{
	return mBlendDst;
}

void Material::SetDstBlendFunc(BlendFunction dst)
{
	mBlendDst = dst;
}

void Material::SetBlendFunc(BlendFunction src, BlendFunction dst)
{
	mBlendSrc = src;
	mBlendDst = dst;
}

Color4& Material::GetBlendColor()
{
	return mBlendColor;
}

void Material::SetBlendColor(const Color4& color)
{
	mBlendColor = color;
}

bool Material::SetTextureLayer(uint layer, Texture* texture)
{
	if (layer >= MAX_TEXTURE_LAYERS)
	{
		return false;
	}

	mTextureLayer[layer] = texture;
	return true;
}

Texture* Material::GetTextureLayer(uint layer) const
{
	if (layer >= MAX_TEXTURE_LAYERS)
	{
		return 0;
	}

	return mTextureLayer[layer];
}

void Material::SetTextureMode(TextureMode mode)
{
	for (uint i = 0; i < MAX_TEXTURE_LAYERS; i++)
	{
		if (mTextureLayer[i] == 0)
			continue;

		mTextureLayer[i]->SetMode(mode);
	}
}

void Material::SetTextureFilter(TextureFilter filter)
{
	for (uint i = 0; i < MAX_TEXTURE_LAYERS; i++)
	{
		if (mTextureLayer[i] == 0)
			continue;

		mTextureLayer[i]->SetFilter(filter);
	}
}

void Material::SetTextureWrap(TextureWrap wrap)
{
	for (uint i = 0; i < MAX_TEXTURE_LAYERS; i++)
	{
		if (mTextureLayer[i] == 0)
			continue;

		mTextureLayer[i]->SetWrap(wrap);
	}
}

} // namespace Crown

