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

#include "Types.h"
#include "TextureResource.h"
#include "Resource.h"
#include "Vec3.h"

namespace crown
{

//! Max texture layers supported by the material.
//! Note that the maximum number of usable layers
//! depends on the graphic card.
const uint32_t MAX_TEXTURE_LAYERS = 8;

enum FogMode
{
	FM_LINEAR	= 0,
	FM_EXP		= 1,
	FM_EXP2		= 2,
	FM_COUNT
};

enum CompareFunction
{
	CF_NEVER	= 0,
	CF_LESS		= 1,
	CF_EQUAL	= 2,
	CF_LEQUAL	= 3,
	CF_GREATER	= 4,
	CF_NOTEQUAL	= 5,
	CF_GEQUAL	= 6,
	CF_ALWAYS	= 7,
	CF_COUNT
};

enum ShadingType
{
	ST_FLAT		= 0,
	ST_SMOOTH	= 1,
	ST_COUNT
};

enum PolygonMode
{
	PM_POINT	= 0,
	PM_LINE		= 1,
	PM_FILL		= 2,
	PM_COUNT
};

enum FrontFace
{
	FF_CW		= 0,
	FF_CCW		= 1,
	FF_COUNT
};

enum BlendEquation
{
	BE_FUNC_ADD					= 0,
	BE_FUNC_SUBTRACT			= 1,
	BE_FUNC_REVERSE_SUBTRACT	= 2,
	BE_MIN						= 3,
	BE_MAX						= 4,
	BE_COUNT
};

enum BlendFunction
{
	BF_ZERO						= 0,
	BF_ONE						= 1,
	BF_SRC_COLOR				= 2,
	BF_ONE_MINUS_SRC_COLOR		= 3,
	BF_DST_COLOR				= 4,
	BF_ONE_MINUS_DST_COLOR		= 5,
	BF_SRC_ALPHA				= 6,
	BF_ONE_MINUS_SRC_ALPHA		= 7,
	BF_DST_ALPHA				= 8,
	BF_ONE_MINUS_DST_ALPHA		= 9,
	BF_CONSTANT_COLOR			= 10,
	BF_ONE_MINUS_CONSTANT_COLOR	= 11,
	BF_CONSTANT_ALPHA			= 12,
	BF_ONE_MINUS_CONSTANT_ALPHA	= 13,
	BF_SRC_ALPHA_SATURATE		= 14,
	BF_COUNT
};

class MaterialResource
{

public:

	//! Constructor
	MaterialResource();

	//! Destructor
	~MaterialResource();

	//! Returns the ambient reflectance
	const Color4& GetAmbient() const;

	//! Sets the ambient reflectance
	void SetAmbient(const Color4& ambient);

	//! Returns the diffuse reflectance
	const Color4& GetDiffuse() const;

	//! Sets the diffuse reflectance
	void SetDiffuse(const Color4& diffuse);

	//! Returns the specular reflectance
	const Color4& GetSpecular() const;

	//! Sets the specular reflectance
	void SetSpecular(const Color4& specular);

	//! Returns the emitted light int32_tensity
	const Color4& GetEmission() const;

	//! Sets the emitted light int32_tensity
	void SetEmission(const Color4& emission);

	//! Returns the shininess
	int32_t GetShininess() const;

	//! Sets the shininess
	void SetShininess(int32_t shininess);

	//! Returns whether lighting is enabled
	bool GetLighting() const;

	//! Sets whether lighting is enabled
	void SetLighting(bool lighting);

	//! Returns whether texturing is enabled
	bool GetTexturing() const;

	//! Sets whether texturing is enabled
	void SetTexturing(bool texturing);

	//! Returns whether backface culling is enabled
	bool GetBackfaceCulling() const;

	//! Sets whether backface culling is enabled
	void SetBackfaceCulling(bool culling);

	//! Returns whether separate specular color is enabled
	bool GetSeparateSpecularColor() const;

	//! Sets whether separate specular color is enabled
	void SetSeparateSpecularColor(bool separate);

	//! Returns whether depth test is enabled
	bool GetDepthTest() const;

	//! Sets whether depth test is enabled
	void SetDepthTest(bool test);

	//! Returns whether depth writing is enabled
	bool GetDepthWrite() const;

	//! Returns whether depth writing is enabled
	void SetDepthWrite(bool write);

	//! Returns whether auto normal rescaling is enabled
	bool GetRescaleNormals() const;

	//! Sets whether auto normal rescaling is enabled
	void SetRescaleNormals(bool rescale);

	//! Returns whether blending is enabled
	bool GetBlending() const;

	//! Sets whether blending is enabled
	void SetBlending(bool blending);

	//! Returns whether writing int32_to the color buffer is enabled
	bool GetColorWrite() const;

	//! Sets whether writing int32_to the color buffer is enabled
	void SetColorWrite(bool write);

	//! Returns whether fog is enabled
	bool GetFog() const;

	//! Sets whether fog is enabled
	void SetFog(bool fog);

	//! Returns whether alpha test is enabled
	bool GetAlphaTest() const;

	//! Sets whether alpha test is enabled
	void SetAlphaTest(bool test);

	//! Returns whether point32_t sprite is enabled
	bool GetPointSprite() const;

	//! Sets whether point32_t sprite is enabled
	void SetPointSprite(bool sprite);

	//! Returns the shading type
	ShadingType GetShadingType() const;

	//! Sets the shading type
	void SetShadingType(ShadingType type);

	//! Returns the polygon mode
	PolygonMode GetPolygonMode() const;

	//! Sets the polygon mode
	void SetPolygonMode(PolygonMode mode);

	//! Returns the front face
	FrontFace GetFrontFace() const;

	//! Sets the front face
	void SetFrontFace(FrontFace front);

	//! Returns the depth function
	CompareFunction GetDepthFunc() const;

	//! Sets the depth function
	void SetDepthFunc(CompareFunction func);

	//! Returns the fog mode
	FogMode GetFogMode() const;

	//! Sets the fog mode
	void SetFogMode(FogMode mode);

	//! Returns the fog density
	float GetFogDensity() const;

	//! Sets the fog density
	void SetFogDensity(float density);

	//! Returns the fog start
	float GetFogStart() const;

	//! Sets the fog start
	void SetFogStart(float start);

	//! Returns the fog end
	float GetFogEnd() const;

	//! Sets the fog end
	void SetFogEnd(float end);

	//! Returns the fog color
	const Color4& GetFogColor() const;

	//! Sets the fog color
	void SetFogColor(const Color4& color);

	//! Returns the alpha function
	CompareFunction GetAlphaFunc() const;

	//! Sets the alpha function
	void SetAlphaFunction(CompareFunction func);

	//! Returns the alpha reference
	float GetAlphaRef() const;

	//! Sets the alpha reference
	void SetAlphaRef(float ref);

	//! Returns the point size
	float GetPointSize() const;

	//! Sets the point size
	void SetPointSize(float size);

	//! Returns the minimum point size
	float GetPointSizeMin() const;

	//! Sets the minimum point size
	void SetPointSizeMin(float min);

	//! Returns the maximum point size
	float GetPointSizeMax() const;

	//! Sets the maximum point' size
	void SetPointSizeMax(float max);

	//! Returns the blending equation
	BlendEquation GetBlendEquation() const;

	//! Sets the blending equation
	void SetBlendEquation(BlendEquation equation);

	//! Returns the source blending function
	BlendFunction GetSrcBlendFunc() const;

	//! Sets the source blending function
	void SetSrcBlendFunc(BlendFunction src);

	//! Returns the destination blending function
	BlendFunction GetDstBlendFunc() const;

	//! Sets the destination blending function
	void SetDstBlendFunc(BlendFunction dst);

	//! Sets the blending function for both source and destination
	void SetBlendFunc(BlendFunction src, BlendFunction dst);

	//! Returns the blending color
	Color4& GetBlendColor();

	//! Sets the blending color
	void SetBlendColor(const Color4& color);

	//! Sets "texture" to layer "layer"
	//! Returns true if success
	bool SetTextureLayer(uint32_t layer, ResourceId texture);

	//! Returns the texture at layer "layer"
	ResourceId GetTextureLayer(uint32_t layer) const;

	//! Sets the texture mode for all layers
	void SetTextureMode(TextureMode mode);

	//! Sets the texture filter for all layers
	void SetTextureFilter(TextureFilter filter);

	//! Sets the texture wrap mode for all layers
	void SetTextureWrap(TextureWrap wrap);

	virtual void	Load(const char* name);
	virtual void	Unload(const char* name, bool reload);

//private:

	Color4			mAmbient;
	Color4			mDiffuse;
	Color4			mSpecular;
	Color4			mEmission;
	int32_t			mShininess;

	bool			mLighting				: 1; // Whether lighting is enabled
	bool			mTexturing				: 1; // Whether texturing is enabled
	bool			mBackfaceCulling		: 1; // Whether backface-culling is enabled
	bool			mSeparateSpecularColor : 1; // Whether separate specular color is enabled
	bool			mDepthTest				: 1; // Whether depth test is enabled
	bool			mDepthWrite			: 1; // Whether depth write is enabled
	bool			mRescaleNormals		: 1; // Whether auto normal rescaling is enabled
	bool			mBlending				: 1; // Whether blending is enabled
	bool			mColorWrite			: 1; // Whether writing int32_to the color buffer is enabled
	bool			mFog					: 1; // Whether fog is enabled
	bool			mAlphaTest				: 1; // Whether alpha test is enabled
	bool			mPointSprite			: 1; // Whether point sprite is enabled

	ShadingType		mShadingType;
	PolygonMode		mPolygonMode;
	FrontFace		mFrontFace;

	CompareFunction	mDepthFunc;

	FogMode 		mFogMode;
	float			mFogDensity;
	float			mFogStart;
	float			mFogEnd;
	Color4			mFogColor;

	CompareFunction	mAlphaFunc;
	float			mAlphaRef;

	float			mPointSize;
	float			mPointSizeMin;
	float			mPointSizeMax;

	BlendEquation	mBlendEquation;
	BlendFunction	mBlendSrc;
	BlendFunction	mBlendDst;
	Color4			mBlendColor;

	// A material can contain up to MAX_TEXTURE_LAYERS texture layers.
	// However, the maximum number of texture layers really usable is renderer-dependent.
	ResourceId		mTextureLayer[MAX_TEXTURE_LAYERS];
};

} // namespace crown

