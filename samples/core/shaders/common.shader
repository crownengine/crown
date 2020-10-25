sampler_states = {
	clamp_point = {
		wrap_u = "clamp"
		wrap_v = "clamp"
		wrap_w = "clamp"
		filter_min = "point"
		filter_mag = "point"
	}

	clamp_anisotropic = {
		wrap_u = "clamp"
		wrap_v = "clamp"
		wrap_w = "clamp"
		filter_min = "anisotropic"
		filter_mag = "anisotropic"
	}

	mirror_point = {
		wrap_u = "mirror"
		wrap_v = "mirror"
		wrap_w = "mirror"
		filter_min = "point"
		filter_mag = "point"
	}

	mirror_anisotropic = {
		wrap_u = "mirror"
		wrap_v = "mirror"
		wrap_w = "mirror"
		filter_min = "anisotropic"
		filter_mag = "anisotropic"
	}
}

bgfx_shaders = {
	common = {
		code = """
			/*
			 * Copyright 2011-2020 Branimir Karadzic. All rights reserved.
			 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
			 */

			#ifndef BGFX_SHADER_H_HEADER_GUARD
			#define BGFX_SHADER_H_HEADER_GUARD

			#if !defined(BGFX_CONFIG_MAX_BONES)
			#	define BGFX_CONFIG_MAX_BONES 32
			#endif // !defined(BGFX_CONFIG_MAX_BONES)

			#ifndef __cplusplus

			#if BGFX_SHADER_LANGUAGE_HLSL > 3
			#	define BRANCH [branch]
			#	define LOOP   [loop]
			#	define UNROLL [unroll]
			#else
			#	define BRANCH
			#	define LOOP
			#	define UNROLL
			#endif // BGFX_SHADER_LANGUAGE_HLSL > 3

			#if BGFX_SHADER_LANGUAGE_HLSL > 3 && BGFX_SHADER_TYPE_FRAGMENT
			#	define EARLY_DEPTH_STENCIL [earlydepthstencil]
			#else
			#	define EARLY_DEPTH_STENCIL
			#endif // BGFX_SHADER_LANGUAGE_HLSL > 3 && BGFX_SHADER_TYPE_FRAGMENT

			#if BGFX_SHADER_LANGUAGE_GLSL
			#	define ARRAY_BEGIN(_type, _name, _count) _type _name[_count] = _type[](
			#	define ARRAY_END() )
			#else
			#	define ARRAY_BEGIN(_type, _name, _count) _type _name[_count] = {
			#	define ARRAY_END() }
			#endif // BGFX_SHADER_LANGUAGE_GLSL

			#if BGFX_SHADER_LANGUAGE_HLSL \
			 || BGFX_SHADER_LANGUAGE_PSSL \
			 || BGFX_SHADER_LANGUAGE_SPIRV \
			 || BGFX_SHADER_LANGUAGE_METAL
			#	define CONST(_x) static const _x
			#	define dFdx(_x) ddx(_x)
			#	define dFdy(_y) ddy(-_y)
			#	define inversesqrt(_x) rsqrt(_x)
			#	define fract(_x) frac(_x)

			#	define bvec2 bool2
			#	define bvec3 bool3
			#	define bvec4 bool4

			// To be able to patch the uav registers on the DXBC SPDB Chunk (D3D11 renderer) the whitespaces around
			// '_type[_reg]' are necessary. This only affects shaders with debug info (i.e., those that have the SPDB Chunk).
			#	if BGFX_SHADER_LANGUAGE_HLSL > 4
			#		define REGISTER(_type, _reg) register( _type[_reg] )
			#	else
			#		define REGISTER(_type, _reg) register(_type ## _reg)
			#	endif // BGFX_SHADER_LANGUAGE_HLSL

			#	if BGFX_SHADER_LANGUAGE_HLSL > 3 || BGFX_SHADER_LANGUAGE_PSSL || BGFX_SHADER_LANGUAGE_SPIRV || BGFX_SHADER_LANGUAGE_METAL
			#		if BGFX_SHADER_LANGUAGE_HLSL > 4 || BGFX_SHADER_LANGUAGE_PSSL || BGFX_SHADER_LANGUAGE_SPIRV || BGFX_SHADER_LANGUAGE_METAL
			#			define dFdxCoarse(_x) ddx_coarse(_x)
			#			define dFdxFine(_x)   ddx_fine(_x)
			#			define dFdyCoarse(_y) ddy_coarse(-_y)
			#			define dFdyFine(_y)   ddy_fine(-_y)
			#		endif // BGFX_SHADER_LANGUAGE_HLSL > 4

			#		if BGFX_SHADER_LANGUAGE_HLSL || BGFX_SHADER_LANGUAGE_SPIRV || BGFX_SHADER_LANGUAGE_METAL
			float intBitsToFloat(int   _x) { return asfloat(_x); }
			vec2  intBitsToFloat(uint2 _x) { return asfloat(_x); }
			vec3  intBitsToFloat(uint3 _x) { return asfloat(_x); }
			vec4  intBitsToFloat(uint4 _x) { return asfloat(_x); }
			#		endif // BGFX_SHADER_LANGUAGE_HLSL || BGFX_SHADER_LANGUAGE_SPIRV || BGFX_SHADER_LANGUAGE_METAL

			float uintBitsToFloat(uint  _x) { return asfloat(_x); }
			vec2  uintBitsToFloat(uint2 _x) { return asfloat(_x); }
			vec3  uintBitsToFloat(uint3 _x) { return asfloat(_x); }
			vec4  uintBitsToFloat(uint4 _x) { return asfloat(_x); }

			uint  floatBitsToUint(float _x) { return asuint(_x); }
			uvec2 floatBitsToUint(vec2  _x) { return asuint(_x); }
			uvec3 floatBitsToUint(vec3  _x) { return asuint(_x); }
			uvec4 floatBitsToUint(vec4  _x) { return asuint(_x); }

			int   floatBitsToInt(float _x) { return asint(_x); }
			ivec2 floatBitsToInt(vec2  _x) { return asint(_x); }
			ivec3 floatBitsToInt(vec3  _x) { return asint(_x); }
			ivec4 floatBitsToInt(vec4  _x) { return asint(_x); }

			uint  bitfieldReverse(uint  _x) { return reversebits(_x); }
			uint2 bitfieldReverse(uint2 _x) { return reversebits(_x); }
			uint3 bitfieldReverse(uint3 _x) { return reversebits(_x); }
			uint4 bitfieldReverse(uint4 _x) { return reversebits(_x); }

			#		if !BGFX_SHADER_LANGUAGE_SPIRV
			uint packHalf2x16(vec2 _x)
			{
				return (f32tof16(_x.y)<<16) | f32tof16(_x.x);
			}

			vec2 unpackHalf2x16(uint _x)
			{
				return vec2(f16tof32(_x & 0xffff), f16tof32(_x >> 16) );
			}
			#		endif // !BGFX_SHADER_LANGUAGE_SPIRV

			struct BgfxSampler2D
			{
				SamplerState m_sampler;
				Texture2D m_texture;
			};

			struct BgfxISampler2D
			{
				Texture2D<ivec4> m_texture;
			};

			struct BgfxUSampler2D
			{
				Texture2D<uvec4> m_texture;
			};

			struct BgfxSampler2DArray
			{
				SamplerState m_sampler;
				Texture2DArray m_texture;
			};

			struct BgfxSampler2DShadow
			{
				SamplerComparisonState m_sampler;
				Texture2D m_texture;
			};

			struct BgfxSampler2DArrayShadow
			{
				SamplerComparisonState m_sampler;
				Texture2DArray m_texture;
			};

			struct BgfxSampler3D
			{
				SamplerState m_sampler;
				Texture3D m_texture;
			};

			struct BgfxISampler3D
			{
				Texture3D<ivec4> m_texture;
			};

			struct BgfxUSampler3D
			{
				Texture3D<uvec4> m_texture;
			};

			struct BgfxSamplerCube
			{
				SamplerState m_sampler;
				TextureCube m_texture;
			};

			struct BgfxSamplerCubeShadow
			{
				SamplerComparisonState m_sampler;
				TextureCube m_texture;
			};

			struct BgfxSampler2DMS
			{
				Texture2DMS<vec4> m_texture;
			};

			vec4 bgfxTexture2D(BgfxSampler2D _sampler, vec2 _coord)
			{
				return _sampler.m_texture.Sample(_sampler.m_sampler, _coord);
			}

			vec4 bgfxTexture2DBias(BgfxSampler2D _sampler, vec2 _coord, float _bias)
			{
				return _sampler.m_texture.SampleBias(_sampler.m_sampler, _coord, _bias);
			}

			vec4 bgfxTexture2DLod(BgfxSampler2D _sampler, vec2 _coord, float _level)
			{
				return _sampler.m_texture.SampleLevel(_sampler.m_sampler, _coord, _level);
			}

			vec4 bgfxTexture2DLodOffset(BgfxSampler2D _sampler, vec2 _coord, float _level, ivec2 _offset)
			{
				return _sampler.m_texture.SampleLevel(_sampler.m_sampler, _coord, _level, _offset);
			}

			vec4 bgfxTexture2DProj(BgfxSampler2D _sampler, vec3 _coord)
			{
				vec2 coord = _coord.xy * rcp(_coord.z);
				return _sampler.m_texture.Sample(_sampler.m_sampler, coord);
			}

			vec4 bgfxTexture2DProj(BgfxSampler2D _sampler, vec4 _coord)
			{
				vec2 coord = _coord.xy * rcp(_coord.w);
				return _sampler.m_texture.Sample(_sampler.m_sampler, coord);
			}

			vec4 bgfxTexture2DGrad(BgfxSampler2D _sampler, vec2 _coord, vec2 _dPdx, vec2 _dPdy)
			{
				return _sampler.m_texture.SampleGrad(_sampler.m_sampler, _coord, _dPdx, _dPdy);
			}

			vec4 bgfxTexture2DArray(BgfxSampler2DArray _sampler, vec3 _coord)
			{
				return _sampler.m_texture.Sample(_sampler.m_sampler, _coord);
			}

			vec4 bgfxTexture2DArrayLod(BgfxSampler2DArray _sampler, vec3 _coord, float _lod)
			{
				return _sampler.m_texture.SampleLevel(_sampler.m_sampler, _coord, _lod);
			}

			vec4 bgfxTexture2DArrayLodOffset(BgfxSampler2DArray _sampler, vec3 _coord, float _level, ivec2 _offset)
			{
				return _sampler.m_texture.SampleLevel(_sampler.m_sampler, _coord, _level, _offset);
			}

			float bgfxShadow2D(BgfxSampler2DShadow _sampler, vec3 _coord)
			{
				return _sampler.m_texture.SampleCmpLevelZero(_sampler.m_sampler, _coord.xy, _coord.z);
			}

			float bgfxShadow2DProj(BgfxSampler2DShadow _sampler, vec4 _coord)
			{
				vec3 coord = _coord.xyz * rcp(_coord.w);
				return _sampler.m_texture.SampleCmpLevelZero(_sampler.m_sampler, coord.xy, coord.z);
			}

			vec4 bgfxShadow2DArray(BgfxSampler2DArrayShadow _sampler, vec4 _coord)
			{
				return _sampler.m_texture.SampleCmpLevelZero(_sampler.m_sampler, _coord.xyz, _coord.w);
			}

			vec4 bgfxTexture3D(BgfxSampler3D _sampler, vec3 _coord)
			{
				return _sampler.m_texture.Sample(_sampler.m_sampler, _coord);
			}

			vec4 bgfxTexture3DLod(BgfxSampler3D _sampler, vec3 _coord, float _level)
			{
				return _sampler.m_texture.SampleLevel(_sampler.m_sampler, _coord, _level);
			}

			ivec4 bgfxTexture3D(BgfxISampler3D _sampler, vec3 _coord)
			{
				uvec3 size;
				_sampler.m_texture.GetDimensions(size.x, size.y, size.z);
				return _sampler.m_texture.Load(ivec4(_coord * size, 0) );
			}

			uvec4 bgfxTexture3D(BgfxUSampler3D _sampler, vec3 _coord)
			{
				uvec3 size;
				_sampler.m_texture.GetDimensions(size.x, size.y, size.z);
				return _sampler.m_texture.Load(ivec4(_coord * size, 0) );
			}

			vec4 bgfxTextureCube(BgfxSamplerCube _sampler, vec3 _coord)
			{
				return _sampler.m_texture.Sample(_sampler.m_sampler, _coord);
			}

			vec4 bgfxTextureCubeBias(BgfxSamplerCube _sampler, vec3 _coord, float _bias)
			{
				return _sampler.m_texture.SampleBias(_sampler.m_sampler, _coord, _bias);
			}

			vec4 bgfxTextureCubeLod(BgfxSamplerCube _sampler, vec3 _coord, float _level)
			{
				return _sampler.m_texture.SampleLevel(_sampler.m_sampler, _coord, _level);
			}

			float bgfxShadowCube(BgfxSamplerCubeShadow _sampler, vec4 _coord)
			{
				return _sampler.m_texture.SampleCmpLevelZero(_sampler.m_sampler, _coord.xyz, _coord.w);
			}

			vec4 bgfxTexelFetch(BgfxSampler2D _sampler, ivec2 _coord, int _lod)
			{
				return _sampler.m_texture.Load(ivec3(_coord, _lod) );
			}

			vec4 bgfxTexelFetchOffset(BgfxSampler2D _sampler, ivec2 _coord, int _lod, ivec2 _offset)
			{
				return _sampler.m_texture.Load(ivec3(_coord, _lod), _offset );
			}

			vec2 bgfxTextureSize(BgfxSampler2D _sampler, int _lod)
			{
				vec2 result;
				_sampler.m_texture.GetDimensions(result.x, result.y);
				return result;
			}

			vec4 bgfxTextureGather(BgfxSampler2D _sampler, vec2 _coord)
			{
				return _sampler.m_texture.GatherRed(_sampler.m_sampler, _coord );
			}
			vec4 bgfxTextureGatherOffset(BgfxSampler2D _sampler, vec2 _coord, ivec2 _offset)
			{
				return _sampler.m_texture.GatherRed(_sampler.m_sampler, _coord, _offset );
			}
			vec4 bgfxTextureGather(BgfxSampler2DArray _sampler, vec3 _coord)
			{
				return _sampler.m_texture.GatherRed(_sampler.m_sampler, _coord );
			}

			ivec4 bgfxTexelFetch(BgfxISampler2D _sampler, ivec2 _coord, int _lod)
			{
				return _sampler.m_texture.Load(ivec3(_coord, _lod) );
			}

			uvec4 bgfxTexelFetch(BgfxUSampler2D _sampler, ivec2 _coord, int _lod)
			{
				return _sampler.m_texture.Load(ivec3(_coord, _lod) );
			}

			vec4 bgfxTexelFetch(BgfxSampler2DMS _sampler, ivec2 _coord, int _sampleIdx)
			{
				return _sampler.m_texture.Load(_coord, _sampleIdx);
			}

			vec4 bgfxTexelFetch(BgfxSampler2DArray _sampler, ivec3 _coord, int _lod)
			{
				return _sampler.m_texture.Load(ivec4(_coord, _lod) );
			}

			vec4 bgfxTexelFetch(BgfxSampler3D _sampler, ivec3 _coord, int _lod)
			{
				return _sampler.m_texture.Load(ivec4(_coord, _lod) );
			}

			vec3 bgfxTextureSize(BgfxSampler3D _sampler, int _lod)
			{
				vec3 result;
				_sampler.m_texture.GetDimensions(result.x, result.y, result.z);
				return result;
			}

			#		define SAMPLER2D(_name, _reg) \
						uniform SamplerState _name ## Sampler : REGISTER(s, _reg); \
						uniform Texture2D _name ## Texture : REGISTER(t, _reg); \
						static BgfxSampler2D _name = { _name ## Sampler, _name ## Texture }
			#		define ISAMPLER2D(_name, _reg) \
						uniform Texture2D<ivec4> _name ## Texture : REGISTER(t, _reg); \
						static BgfxISampler2D _name = { _name ## Texture }
			#		define USAMPLER2D(_name, _reg) \
						uniform Texture2D<uvec4> _name ## Texture : REGISTER(t, _reg); \
						static BgfxUSampler2D _name = { _name ## Texture }
			#		define sampler2D BgfxSampler2D
			#		define texture2D(_sampler, _coord) bgfxTexture2D(_sampler, _coord)
			#		define texture2DBias(_sampler, _coord, _bias) bgfxTexture2DBias(_sampler, _coord, _bias)
			#		define texture2DLod(_sampler, _coord, _level) bgfxTexture2DLod(_sampler, _coord, _level)
			#		define texture2DLodOffset(_sampler, _coord, _level, _offset) bgfxTexture2DLodOffset(_sampler, _coord, _level, _offset)
			#		define texture2DProj(_sampler, _coord) bgfxTexture2DProj(_sampler, _coord)
			#		define texture2DGrad(_sampler, _coord, _dPdx, _dPdy) bgfxTexture2DGrad(_sampler, _coord, _dPdx, _dPdy)

			#		define SAMPLER2DARRAY(_name, _reg) \
						uniform SamplerState _name ## Sampler : REGISTER(s, _reg); \
						uniform Texture2DArray _name ## Texture : REGISTER(t, _reg); \
						static BgfxSampler2DArray _name = { _name ## Sampler, _name ## Texture }
			#		define sampler2DArray BgfxSampler2DArray
			#		define texture2DArray(_sampler, _coord) bgfxTexture2DArray(_sampler, _coord)
			#		define texture2DArrayLod(_sampler, _coord, _lod) bgfxTexture2DArrayLod(_sampler, _coord, _lod)
			#		define texture2DArrayLodOffset(_sampler, _coord, _level, _offset) bgfxTexture2DArrayLodOffset(_sampler, _coord, _level, _offset)

			#		define SAMPLER2DMS(_name, _reg) \
						uniform Texture2DMS<vec4> _name ## Texture : REGISTER(t, _reg); \
						static BgfxSampler2DMS _name = { _name ## Texture }
			#		define sampler2DMS BgfxSampler2DMS

			#		define SAMPLER2DSHADOW(_name, _reg) \
						uniform SamplerComparisonState _name ## SamplerComparison : REGISTER(s, _reg); \
						uniform Texture2D _name ## Texture : REGISTER(t, _reg); \
						static BgfxSampler2DShadow _name = { _name ## SamplerComparison, _name ## Texture }
			#		define sampler2DShadow BgfxSampler2DShadow
			#		define shadow2D(_sampler, _coord) bgfxShadow2D(_sampler, _coord)
			#		define shadow2DProj(_sampler, _coord) bgfxShadow2DProj(_sampler, _coord)

			#		define SAMPLER2DARRAYSHADOW(_name, _reg) \
						uniform SamplerComparisonState _name ## SamplerComparison : REGISTER(s, _reg); \
						uniform Texture2DArray _name ## Texture : REGISTER(t, _reg); \
						static BgfxSampler2DArrayShadow _name = { _name ## SamplerComparison, _name ## Texture }
			#		define sampler2DArrayShadow BgfxSampler2DArrayShadow
			#		define shadow2DArray(_sampler, _coord) bgfxShadow2DArray(_sampler, _coord)

			#		define SAMPLER3D(_name, _reg) \
						uniform SamplerState _name ## Sampler : REGISTER(s, _reg); \
						uniform Texture3D _name ## Texture : REGISTER(t, _reg); \
						static BgfxSampler3D _name = { _name ## Sampler, _name ## Texture }
			#		define ISAMPLER3D(_name, _reg) \
						uniform Texture3D<ivec4> _name ## Texture : REGISTER(t, _reg); \
						static BgfxISampler3D _name = { _name ## Texture }
			#		define USAMPLER3D(_name, _reg) \
						uniform Texture3D<uvec4> _name ## Texture : REGISTER(t, _reg); \
						static BgfxUSampler3D _name = { _name ## Texture }
			#		define sampler3D BgfxSampler3D
			#		define texture3D(_sampler, _coord) bgfxTexture3D(_sampler, _coord)
			#		define texture3DLod(_sampler, _coord, _level) bgfxTexture3DLod(_sampler, _coord, _level)

			#		define SAMPLERCUBE(_name, _reg) \
						uniform SamplerState _name ## Sampler : REGISTER(s, _reg); \
						uniform TextureCube _name ## Texture : REGISTER(t, _reg); \
						static BgfxSamplerCube _name = { _name ## Sampler, _name ## Texture }
			#		define samplerCube BgfxSamplerCube
			#		define textureCube(_sampler, _coord) bgfxTextureCube(_sampler, _coord)
			#		define textureCubeBias(_sampler, _coord, _bias) bgfxTextureCubeBias(_sampler, _coord, _bias)
			#		define textureCubeLod(_sampler, _coord, _level) bgfxTextureCubeLod(_sampler, _coord, _level)

			#		define SAMPLERCUBESHADOW(_name, _reg) \
						uniform SamplerComparisonState _name ## SamplerComparison : REGISTER(s, _reg); \
						uniform TextureCube _name ## Texture : REGISTER(t, _reg); \
						static BgfxSamplerCubeShadow _name = { _name ## SamplerComparison, _name ## Texture }
			#		define samplerCubeShadow BgfxSamplerCubeShadow
			#		define shadowCube(_sampler, _coord) bgfxShadowCube(_sampler, _coord)

			#		define texelFetch(_sampler, _coord, _lod) bgfxTexelFetch(_sampler, _coord, _lod)
			#		define texelFetchOffset(_sampler, _coord, _lod, _offset) bgfxTexelFetchOffset(_sampler, _coord, _lod, _offset)
			#		define textureSize(_sampler, _lod) bgfxTextureSize(_sampler, _lod)
			#		define textureGather(_sampler, _coord) bgfxTextureGather(_sampler, _coord)
			#		define textureGatherOffset(_sampler, _coord, _offset) bgfxTextureGatherOffset(_sampler, _coord, _offset)
			#	else

			#		define sampler2DShadow sampler2D

			vec4 bgfxTexture2DProj(sampler2D _sampler, vec3 _coord)
			{
				return tex2Dproj(_sampler, vec4(_coord.xy, 0.0, _coord.z) );
			}

			vec4 bgfxTexture2DProj(sampler2D _sampler, vec4 _coord)
			{
				return tex2Dproj(_sampler, _coord);
			}

			float bgfxShadow2D(sampler2DShadow _sampler, vec3 _coord)
			{
			#if 0
				float occluder = tex2D(_sampler, _coord.xy).x;
				return step(_coord.z, occluder);
			#else
				return tex2Dproj(_sampler, vec4(_coord.xy, _coord.z, 1.0) ).x;
			#endif // 0
			}

			float bgfxShadow2DProj(sampler2DShadow _sampler, vec4 _coord)
			{
			#if 0
				vec3 coord = _coord.xyz * rcp(_coord.w);
				float occluder = tex2D(_sampler, coord.xy).x;
				return step(coord.z, occluder);
			#else
				return tex2Dproj(_sampler, _coord).x;
			#endif // 0
			}

			#		define SAMPLER2D(_name, _reg) uniform sampler2D _name : REGISTER(s, _reg)
			#		define SAMPLER2DMS(_name, _reg) uniform sampler2DMS _name : REGISTER(s, _reg)
			#		define texture2D(_sampler, _coord) tex2D(_sampler, _coord)
			#		define texture2DProj(_sampler, _coord) bgfxTexture2DProj(_sampler, _coord)

			#		define SAMPLER2DARRAY(_name, _reg) SAMPLER2D(_name, _reg)
			#		define texture2DArray(_sampler, _coord) texture2D(_sampler, (_coord).xy)
			#		define texture2DArrayLod(_sampler, _coord, _lod) texture2DLod(_sampler, _coord, _lod)

			#		define SAMPLER2DSHADOW(_name, _reg) uniform sampler2DShadow _name : REGISTER(s, _reg)
			#		define shadow2D(_sampler, _coord) bgfxShadow2D(_sampler, _coord)
			#		define shadow2DProj(_sampler, _coord) bgfxShadow2DProj(_sampler, _coord)

			#		define SAMPLER3D(_name, _reg) uniform sampler3D _name : REGISTER(s, _reg)
			#		define texture3D(_sampler, _coord) tex3D(_sampler, _coord)

			#		define SAMPLERCUBE(_name, _reg) uniform samplerCUBE _name : REGISTER(s, _reg)
			#		define textureCube(_sampler, _coord) texCUBE(_sampler, _coord)

			#		if BGFX_SHADER_LANGUAGE_HLSL == 2
			#			define texture2DLod(_sampler, _coord, _level) tex2D(_sampler, (_coord).xy)
			#			define texture2DGrad(_sampler, _coord, _dPdx, _dPdy) tex2D(_sampler, _coord)
			#			define texture3DLod(_sampler, _coord, _level) tex3D(_sampler, (_coord).xyz)
			#			define textureCubeLod(_sampler, _coord, _level) texCUBE(_sampler, (_coord).xyz)
			#		else
			#			define texture2DLod(_sampler, _coord, _level) tex2Dlod(_sampler, vec4( (_coord).xy, 0.0, _level) )
			#			define texture2DGrad(_sampler, _coord, _dPdx, _dPdy) tex2Dgrad(_sampler, _coord, _dPdx, _dPdy)
			#			define texture3DLod(_sampler, _coord, _level) tex3Dlod(_sampler, vec4( (_coord).xyz, _level) )
			#			define textureCubeLod(_sampler, _coord, _level) texCUBElod(_sampler, vec4( (_coord).xyz, _level) )
			#		endif // BGFX_SHADER_LANGUAGE_HLSL == 2

			#	endif // BGFX_SHADER_LANGUAGE_HLSL > 3

			vec3 instMul(vec3 _vec, mat3 _mtx) { return mul(_mtx, _vec); }
			vec3 instMul(mat3 _mtx, vec3 _vec) { return mul(_vec, _mtx); }
			vec4 instMul(vec4 _vec, mat4 _mtx) { return mul(_mtx, _vec); }
			vec4 instMul(mat4 _mtx, vec4 _vec) { return mul(_vec, _mtx); }

			bvec2 lessThan(vec2 _a, vec2 _b) { return _a < _b; }
			bvec3 lessThan(vec3 _a, vec3 _b) { return _a < _b; }
			bvec4 lessThan(vec4 _a, vec4 _b) { return _a < _b; }

			bvec2 lessThanEqual(vec2 _a, vec2 _b) { return _a <= _b; }
			bvec3 lessThanEqual(vec3 _a, vec3 _b) { return _a <= _b; }
			bvec4 lessThanEqual(vec4 _a, vec4 _b) { return _a <= _b; }

			bvec2 greaterThan(vec2 _a, vec2 _b) { return _a > _b; }
			bvec3 greaterThan(vec3 _a, vec3 _b) { return _a > _b; }
			bvec4 greaterThan(vec4 _a, vec4 _b) { return _a > _b; }

			bvec2 greaterThanEqual(vec2 _a, vec2 _b) { return _a >= _b; }
			bvec3 greaterThanEqual(vec3 _a, vec3 _b) { return _a >= _b; }
			bvec4 greaterThanEqual(vec4 _a, vec4 _b) { return _a >= _b; }

			bvec2 notEqual(vec2 _a, vec2 _b) { return _a != _b; }
			bvec3 notEqual(vec3 _a, vec3 _b) { return _a != _b; }
			bvec4 notEqual(vec4 _a, vec4 _b) { return _a != _b; }

			bvec2 equal(vec2 _a, vec2 _b) { return _a == _b; }
			bvec3 equal(vec3 _a, vec3 _b) { return _a == _b; }
			bvec4 equal(vec4 _a, vec4 _b) { return _a == _b; }

			float mix(float _a, float _b, float _t) { return lerp(_a, _b, _t); }
			vec2  mix(vec2  _a, vec2  _b, vec2  _t) { return lerp(_a, _b, _t); }
			vec3  mix(vec3  _a, vec3  _b, vec3  _t) { return lerp(_a, _b, _t); }
			vec4  mix(vec4  _a, vec4  _b, vec4  _t) { return lerp(_a, _b, _t); }

			float mod(float _a, float _b) { return _a - _b * floor(_a / _b); }
			vec2  mod(vec2  _a, vec2  _b) { return _a - _b * floor(_a / _b); }
			vec3  mod(vec3  _a, vec3  _b) { return _a - _b * floor(_a / _b); }
			vec4  mod(vec4  _a, vec4  _b) { return _a - _b * floor(_a / _b); }

			#else
			#	define CONST(_x) const _x
			#	define atan2(_x, _y) atan(_x, _y)
			#	define mul(_a, _b) ( (_a) * (_b) )
			#	define saturate(_x) clamp(_x, 0.0, 1.0)
			#	define SAMPLER2D(_name, _reg)       uniform sampler2D _name
			#	define SAMPLER2DMS(_name, _reg)     uniform sampler2DMS _name
			#	define SAMPLER3D(_name, _reg)       uniform sampler3D _name
			#	define SAMPLERCUBE(_name, _reg)     uniform samplerCube _name
			#	define SAMPLER2DSHADOW(_name, _reg) uniform sampler2DShadow _name

			#	define SAMPLER2DARRAY(_name, _reg)       uniform sampler2DArray _name
			#	define SAMPLER2DMSARRAY(_name, _reg)     uniform sampler2DMSArray _name
			#	define SAMPLERCUBEARRAY(_name, _reg)     uniform samplerCubeArray _name
			#	define SAMPLER2DARRAYSHADOW(_name, _reg) uniform sampler2DArrayShadow _name

			#	define ISAMPLER2D(_name, _reg) uniform isampler2D _name
			#	define USAMPLER2D(_name, _reg) uniform usampler2D _name
			#	define ISAMPLER3D(_name, _reg) uniform isampler3D _name
			#	define USAMPLER3D(_name, _reg) uniform usampler3D _name

			#	define texture2DBias(_sampler, _coord, _bias)      texture2D(_sampler, _coord, _bias)
			#	define textureCubeBias(_sampler, _coord, _bias)    textureCube(_sampler, _coord, _bias)

			#	if BGFX_SHADER_LANGUAGE_GLSL >= 130
			#		define texture2D(_sampler, _coord)      texture(_sampler, _coord)
			#		define texture2DArray(_sampler, _coord) texture(_sampler, _coord)
			#		define texture3D(_sampler, _coord)      texture(_sampler, _coord)
			#	endif // BGFX_SHADER_LANGUAGE_GLSL >= 130

			vec3 instMul(vec3 _vec, mat3 _mtx) { return mul(_vec, _mtx); }
			vec3 instMul(mat3 _mtx, vec3 _vec) { return mul(_mtx, _vec); }
			vec4 instMul(vec4 _vec, mat4 _mtx) { return mul(_vec, _mtx); }
			vec4 instMul(mat4 _mtx, vec4 _vec) { return mul(_mtx, _vec); }

			float rcp(float _a) { return 1.0/_a; }
			vec2  rcp(vec2  _a) { return vec2(1.0)/_a; }
			vec3  rcp(vec3  _a) { return vec3(1.0)/_a; }
			vec4  rcp(vec4  _a) { return vec4(1.0)/_a; }
			#endif // BGFX_SHADER_LANGUAGE_*

			vec2 vec2_splat(float _x) { return vec2(_x, _x); }
			vec3 vec3_splat(float _x) { return vec3(_x, _x, _x); }
			vec4 vec4_splat(float _x) { return vec4(_x, _x, _x, _x); }

			#if BGFX_SHADER_LANGUAGE_GLSL >= 130 || BGFX_SHADER_LANGUAGE_HLSL || BGFX_SHADER_LANGUAGE_PSSL || BGFX_SHADER_LANGUAGE_SPIRV || BGFX_SHADER_LANGUAGE_METAL
			uvec2 uvec2_splat(uint _x) { return uvec2(_x, _x); }
			uvec3 uvec3_splat(uint _x) { return uvec3(_x, _x, _x); }
			uvec4 uvec4_splat(uint _x) { return uvec4(_x, _x, _x, _x); }
			#endif // BGFX_SHADER_LANGUAGE_GLSL >= 130 || BGFX_SHADER_LANGUAGE_HLSL || BGFX_SHADER_LANGUAGE_PSSL || BGFX_SHADER_LANGUAGE_SPIRV || BGFX_SHADER_LANGUAGE_METAL

			mat4 mtxFromRows(vec4 _0, vec4 _1, vec4 _2, vec4 _3)
			{
			#if BGFX_SHADER_LANGUAGE_GLSL
				return transpose(mat4(_0, _1, _2, _3) );
			#else
				return mat4(_0, _1, _2, _3);
			#endif // BGFX_SHADER_LANGUAGE_GLSL
			}
			mat4 mtxFromCols(vec4 _0, vec4 _1, vec4 _2, vec4 _3)
			{
			#if BGFX_SHADER_LANGUAGE_GLSL
				return mat4(_0, _1, _2, _3);
			#else
				return transpose(mat4(_0, _1, _2, _3) );
			#endif // BGFX_SHADER_LANGUAGE_GLSL
			}
			mat3 mtxFromRows(vec3 _0, vec3 _1, vec3 _2)
			{
			#if BGFX_SHADER_LANGUAGE_GLSL
				return transpose(mat3(_0, _1, _2) );
			#else
				return mat3(_0, _1, _2);
			#endif // BGFX_SHADER_LANGUAGE_GLSL
			}
			mat3 mtxFromCols(vec3 _0, vec3 _1, vec3 _2)
			{
			#if BGFX_SHADER_LANGUAGE_GLSL
				return mat3(_0, _1, _2);
			#else
				return transpose(mat3(_0, _1, _2) );
			#endif // BGFX_SHADER_LANGUAGE_GLSL
			}

			#if BGFX_SHADER_LANGUAGE_GLSL
			#define mtxFromRows3(_0, _1, _2)     transpose(mat3(_0, _1, _2) )
			#define mtxFromRows4(_0, _1, _2, _3) transpose(mat4(_0, _1, _2, _3) )
			#define mtxFromCols3(_0, _1, _2)               mat3(_0, _1, _2)
			#define mtxFromCols4(_0, _1, _2, _3)           mat4(_0, _1, _2, _3)
			#else
			#define mtxFromRows3(_0, _1, _2)               mat3(_0, _1, _2)
			#define mtxFromRows4(_0, _1, _2, _3)           mat4(_0, _1, _2, _3)
			#define mtxFromCols3(_0, _1, _2)     transpose(mat3(_0, _1, _2) )
			#define mtxFromCols4(_0, _1, _2, _3) transpose(mat4(_0, _1, _2, _3) )
			#endif // BGFX_SHADER_LANGUAGE_GLSL

			uniform vec4  u_viewRect;
			uniform vec4  u_viewTexel;
			uniform mat4  u_view;
			uniform mat4  u_invView;
			uniform mat4  u_proj;
			uniform mat4  u_invProj;
			uniform mat4  u_viewProj;
			uniform mat4  u_invViewProj;
			uniform mat4  u_model[BGFX_CONFIG_MAX_BONES];
			uniform mat4  u_modelView;
			uniform mat4  u_modelViewProj;
			uniform vec4  u_alphaRef4;
			#define u_alphaRef u_alphaRef4.x

			#endif // __cplusplus

			#endif // BGFX_SHADER_H_HEADER_GUARD
			/*
			 * Copyright 2011-2020 Branimir Karadzic. All rights reserved.
			 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
			 */

			#ifndef __SHADERLIB_SH__
			#define __SHADERLIB_SH__

			vec4 encodeRE8(float _r)
			{
				float exponent = ceil(log2(_r) );
				return vec4(_r / exp2(exponent)
					, 0.0
					, 0.0
					, (exponent + 128.0) / 255.0
					);
			}

			float decodeRE8(vec4 _re8)
			{
				float exponent = _re8.w * 255.0 - 128.0;
				return _re8.x * exp2(exponent);
			}

			vec4 encodeRGBE8(vec3 _rgb)
			{
				vec4 rgbe8;
				float maxComponent = max(max(_rgb.x, _rgb.y), _rgb.z);
				float exponent = ceil(log2(maxComponent) );
				rgbe8.xyz = _rgb / exp2(exponent);
				rgbe8.w = (exponent + 128.0) / 255.0;
				return rgbe8;
			}

			vec3 decodeRGBE8(vec4 _rgbe8)
			{
				float exponent = _rgbe8.w * 255.0 - 128.0;
				vec3 rgb = _rgbe8.xyz * exp2(exponent);
				return rgb;
			}

			vec3 encodeNormalUint(vec3 _normal)
			{
				return _normal * 0.5 + 0.5;
			}

			vec3 decodeNormalUint(vec3 _encodedNormal)
			{
				return _encodedNormal * 2.0 - 1.0;
			}

			vec2 encodeNormalSphereMap(vec3 _normal)
			{
				return normalize(_normal.xy) * sqrt(_normal.z * 0.5 + 0.5);
			}

			vec3 decodeNormalSphereMap(vec2 _encodedNormal)
			{
				float zz = dot(_encodedNormal, _encodedNormal) * 2.0 - 1.0;
				return vec3(normalize(_encodedNormal.xy) * sqrt(1.0 - zz*zz), zz);
			}

			vec2 octahedronWrap(vec2 _val)
			{
				// Reference(s):
				// - Octahedron normal vector encoding
				//   https://web.archive.org/web/20191027010600/https://knarkowicz.wordpress.com/2014/04/16/octahedron-normal-vector-encoding/comment-page-1/
				return (1.0 - abs(_val.yx) )
					 * mix(vec2_splat(-1.0), vec2_splat(1.0), vec2(greaterThanEqual(_val.xy, vec2_splat(0.0) ) ) );
			}

			vec2 encodeNormalOctahedron(vec3 _normal)
			{
				_normal /= abs(_normal.x) + abs(_normal.y) + abs(_normal.z);
				_normal.xy = _normal.z >= 0.0 ? _normal.xy : octahedronWrap(_normal.xy);
				_normal.xy = _normal.xy * 0.5 + 0.5;
				return _normal.xy;
			}

			vec3 decodeNormalOctahedron(vec2 _encodedNormal)
			{
				_encodedNormal = _encodedNormal * 2.0 - 1.0;

				vec3 normal;
				normal.z  = 1.0 - abs(_encodedNormal.x) - abs(_encodedNormal.y);
				normal.xy = normal.z >= 0.0 ? _encodedNormal.xy : octahedronWrap(_encodedNormal.xy);
				return normalize(normal);
			}

			vec3 convertRGB2XYZ(vec3 _rgb)
			{
				// Reference(s):
				// - RGB/XYZ Matrices
				//   https://web.archive.org/web/20191027010220/http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
				vec3 xyz;
				xyz.x = dot(vec3(0.4124564, 0.3575761, 0.1804375), _rgb);
				xyz.y = dot(vec3(0.2126729, 0.7151522, 0.0721750), _rgb);
				xyz.z = dot(vec3(0.0193339, 0.1191920, 0.9503041), _rgb);
				return xyz;
			}

			vec3 convertXYZ2RGB(vec3 _xyz)
			{
				vec3 rgb;
				rgb.x = dot(vec3( 3.2404542, -1.5371385, -0.4985314), _xyz);
				rgb.y = dot(vec3(-0.9692660,  1.8760108,  0.0415560), _xyz);
				rgb.z = dot(vec3( 0.0556434, -0.2040259,  1.0572252), _xyz);
				return rgb;
			}

			vec3 convertXYZ2Yxy(vec3 _xyz)
			{
				// Reference(s):
				// - XYZ to xyY
				//   https://web.archive.org/web/20191027010144/http://www.brucelindbloom.com/index.html?Eqn_XYZ_to_xyY.html
				float inv = 1.0/dot(_xyz, vec3(1.0, 1.0, 1.0) );
				return vec3(_xyz.y, _xyz.x*inv, _xyz.y*inv);
			}

			vec3 convertYxy2XYZ(vec3 _Yxy)
			{
				// Reference(s):
				// - xyY to XYZ
				//   https://web.archive.org/web/20191027010036/http://www.brucelindbloom.com/index.html?Eqn_xyY_to_XYZ.html
				vec3 xyz;
				xyz.x = _Yxy.x*_Yxy.y/_Yxy.z;
				xyz.y = _Yxy.x;
				xyz.z = _Yxy.x*(1.0 - _Yxy.y - _Yxy.z)/_Yxy.z;
				return xyz;
			}

			vec3 convertRGB2Yxy(vec3 _rgb)
			{
				return convertXYZ2Yxy(convertRGB2XYZ(_rgb) );
			}

			vec3 convertYxy2RGB(vec3 _Yxy)
			{
				return convertXYZ2RGB(convertYxy2XYZ(_Yxy) );
			}

			vec3 convertRGB2Yuv(vec3 _rgb)
			{
				vec3 yuv;
				yuv.x = dot(_rgb, vec3(0.299, 0.587, 0.114) );
				yuv.y = (_rgb.x - yuv.x)*0.713 + 0.5;
				yuv.z = (_rgb.z - yuv.x)*0.564 + 0.5;
				return yuv;
			}

			vec3 convertYuv2RGB(vec3 _yuv)
			{
				vec3 rgb;
				rgb.x = _yuv.x + 1.403*(_yuv.y-0.5);
				rgb.y = _yuv.x - 0.344*(_yuv.y-0.5) - 0.714*(_yuv.z-0.5);
				rgb.z = _yuv.x + 1.773*(_yuv.z-0.5);
				return rgb;
			}

			vec3 convertRGB2YIQ(vec3 _rgb)
			{
				vec3 yiq;
				yiq.x = dot(vec3(0.299,     0.587,     0.114   ), _rgb);
				yiq.y = dot(vec3(0.595716, -0.274453, -0.321263), _rgb);
				yiq.z = dot(vec3(0.211456, -0.522591,  0.311135), _rgb);
				return yiq;
			}

			vec3 convertYIQ2RGB(vec3 _yiq)
			{
				vec3 rgb;
				rgb.x = dot(vec3(1.0,  0.9563,  0.6210), _yiq);
				rgb.y = dot(vec3(1.0, -0.2721, -0.6474), _yiq);
				rgb.z = dot(vec3(1.0, -1.1070,  1.7046), _yiq);
				return rgb;
			}

			vec3 toLinear(vec3 _rgb)
			{
				return pow(abs(_rgb), vec3_splat(2.2) );
			}

			vec4 toLinear(vec4 _rgba)
			{
				return vec4(toLinear(_rgba.xyz), _rgba.w);
			}

			vec3 toLinearAccurate(vec3 _rgb)
			{
				vec3 lo = _rgb / 12.92;
				vec3 hi = pow( (_rgb + 0.055) / 1.055, vec3_splat(2.4) );
				vec3 rgb = mix(hi, lo, vec3(lessThanEqual(_rgb, vec3_splat(0.04045) ) ) );
				return rgb;
			}

			vec4 toLinearAccurate(vec4 _rgba)
			{
				return vec4(toLinearAccurate(_rgba.xyz), _rgba.w);
			}

			float toGamma(float _r)
			{
				return pow(abs(_r), 1.0/2.2);
			}

			vec3 toGamma(vec3 _rgb)
			{
				return pow(abs(_rgb), vec3_splat(1.0/2.2) );
			}

			vec4 toGamma(vec4 _rgba)
			{
				return vec4(toGamma(_rgba.xyz), _rgba.w);
			}

			vec3 toGammaAccurate(vec3 _rgb)
			{
				vec3 lo  = _rgb * 12.92;
				vec3 hi  = pow(abs(_rgb), vec3_splat(1.0/2.4) ) * 1.055 - 0.055;
				vec3 rgb = mix(hi, lo, vec3(lessThanEqual(_rgb, vec3_splat(0.0031308) ) ) );
				return rgb;
			}

			vec4 toGammaAccurate(vec4 _rgba)
			{
				return vec4(toGammaAccurate(_rgba.xyz), _rgba.w);
			}

			vec3 toReinhard(vec3 _rgb)
			{
				return toGamma(_rgb/(_rgb+vec3_splat(1.0) ) );
			}

			vec4 toReinhard(vec4 _rgba)
			{
				return vec4(toReinhard(_rgba.xyz), _rgba.w);
			}

			vec3 toFilmic(vec3 _rgb)
			{
				_rgb = max(vec3_splat(0.0), _rgb - 0.004);
				_rgb = (_rgb*(6.2*_rgb + 0.5) ) / (_rgb*(6.2*_rgb + 1.7) + 0.06);
				return _rgb;
			}

			vec4 toFilmic(vec4 _rgba)
			{
				return vec4(toFilmic(_rgba.xyz), _rgba.w);
			}

			vec3 toAcesFilmic(vec3 _rgb)
			{
				// Reference(s):
				// - ACES Filmic Tone Mapping Curve
				//   https://web.archive.org/web/20191027010704/https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
				float aa = 2.51f;
				float bb = 0.03f;
				float cc = 2.43f;
				float dd = 0.59f;
				float ee = 0.14f;
				return saturate( (_rgb*(aa*_rgb + bb) )/(_rgb*(cc*_rgb + dd) + ee) );
			}

			vec4 toAcesFilmic(vec4 _rgba)
			{
				return vec4(toAcesFilmic(_rgba.xyz), _rgba.w);
			}

			vec3 luma(vec3 _rgb)
			{
				float yy = dot(vec3(0.2126729, 0.7151522, 0.0721750), _rgb);
				return vec3_splat(yy);
			}

			vec4 luma(vec4 _rgba)
			{
				return vec4(luma(_rgba.xyz), _rgba.w);
			}

			vec3 conSatBri(vec3 _rgb, vec3 _csb)
			{
				vec3 rgb = _rgb * _csb.z;
				rgb = mix(luma(rgb), rgb, _csb.y);
				rgb = mix(vec3_splat(0.5), rgb, _csb.x);
				return rgb;
			}

			vec4 conSatBri(vec4 _rgba, vec3 _csb)
			{
				return vec4(conSatBri(_rgba.xyz, _csb), _rgba.w);
			}

			vec3 posterize(vec3 _rgb, float _numColors)
			{
				return floor(_rgb*_numColors) / _numColors;
			}

			vec4 posterize(vec4 _rgba, float _numColors)
			{
				return vec4(posterize(_rgba.xyz, _numColors), _rgba.w);
			}

			vec3 sepia(vec3 _rgb)
			{
				vec3 color;
				color.x = dot(_rgb, vec3(0.393, 0.769, 0.189) );
				color.y = dot(_rgb, vec3(0.349, 0.686, 0.168) );
				color.z = dot(_rgb, vec3(0.272, 0.534, 0.131) );
				return color;
			}

			vec4 sepia(vec4 _rgba)
			{
				return vec4(sepia(_rgba.xyz), _rgba.w);
			}

			vec3 blendOverlay(vec3 _base, vec3 _blend)
			{
				vec3 lt = 2.0 * _base * _blend;
				vec3 gte = 1.0 - 2.0 * (1.0 - _base) * (1.0 - _blend);
				return mix(lt, gte, step(vec3_splat(0.5), _base) );
			}

			vec4 blendOverlay(vec4 _base, vec4 _blend)
			{
				return vec4(blendOverlay(_base.xyz, _blend.xyz), _base.w);
			}

			vec3 adjustHue(vec3 _rgb, float _hue)
			{
				vec3 yiq = convertRGB2YIQ(_rgb);
				float angle = _hue + atan2(yiq.z, yiq.y);
				float len = length(yiq.yz);
				return convertYIQ2RGB(vec3(yiq.x, len*cos(angle), len*sin(angle) ) );
			}

			vec4 packFloatToRgba(float _value)
			{
				const vec4 shift = vec4(256 * 256 * 256, 256 * 256, 256, 1.0);
				const vec4 mask = vec4(0, 1.0 / 256.0, 1.0 / 256.0, 1.0 / 256.0);
				vec4 comp = fract(_value * shift);
				comp -= comp.xxyz * mask;
				return comp;
			}

			float unpackRgbaToFloat(vec4 _rgba)
			{
				const vec4 shift = vec4(1.0 / (256.0 * 256.0 * 256.0), 1.0 / (256.0 * 256.0), 1.0 / 256.0, 1.0);
				return dot(_rgba, shift);
			}

			vec2 packHalfFloat(float _value)
			{
				const vec2 shift = vec2(256, 1.0);
				const vec2 mask = vec2(0, 1.0 / 256.0);
				vec2 comp = fract(_value * shift);
				comp -= comp.xx * mask;
				return comp;
			}

			float unpackHalfFloat(vec2 _rg)
			{
				const vec2 shift = vec2(1.0 / 256.0, 1.0);
				return dot(_rg, shift);
			}

			float random(vec2 _uv)
			{
				return fract(sin(dot(_uv.xy, vec2(12.9898, 78.233) ) ) * 43758.5453);
			}

			vec3 fixCubeLookup(vec3 _v, float _lod, float _topLevelCubeSize)
			{
				// Reference(s):
				// - Seamless cube-map filtering
				//   https://web.archive.org/web/20190411181934/http://the-witness.net/news/2012/02/seamless-cube-map-filtering/
				float ax = abs(_v.x);
				float ay = abs(_v.y);
				float az = abs(_v.z);
				float vmax = max(max(ax, ay), az);
				float scale = 1.0 - exp2(_lod) / _topLevelCubeSize;
				if (ax != vmax) { _v.x *= scale; }
				if (ay != vmax) { _v.y *= scale; }
				if (az != vmax) { _v.z *= scale; }
				return _v;
			}

			vec2 texture2DBc5(sampler2D _sampler, vec2 _uv)
			{
			#if BGFX_SHADER_LANGUAGE_HLSL && BGFX_SHADER_LANGUAGE_HLSL <= 3
				return texture2D(_sampler, _uv).yx;
			#else
				return texture2D(_sampler, _uv).xy;
			#endif
			}

			mat3 cofactor(mat4 _m)
			{
				// Reference:
				// Cofactor of matrix. Use to transform normals. The code assumes the last column of _m is [0,0,0,1].
				// https://www.shadertoy.com/view/3s33zj
				// https://github.com/graphitemaster/normals_revisited
				return mat3(
					_m[1][1]*_m[2][2]-_m[1][2]*_m[2][1],
					_m[1][2]*_m[2][0]-_m[1][0]*_m[2][2],
					_m[1][0]*_m[2][1]-_m[1][1]*_m[2][0],
					_m[0][2]*_m[2][1]-_m[0][1]*_m[2][2],
					_m[0][0]*_m[2][2]-_m[0][2]*_m[2][0],
					_m[0][1]*_m[2][0]-_m[0][0]*_m[2][1],
					_m[0][1]*_m[1][2]-_m[0][2]*_m[1][1],
					_m[0][2]*_m[1][0]-_m[0][0]*_m[1][2],
					_m[0][0]*_m[1][1]-_m[0][1]*_m[1][0]
					);
			}

			#endif // __SHADERLIB_SH__
		"""
	}
}
