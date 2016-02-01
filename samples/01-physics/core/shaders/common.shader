sampler_states = {
	clamp_point = {
		wrap_u = "clamp"
		wrap_v = "clamp"
		wrap_w = "clamp"
		min_filter = "point"
		mag_filter = "point"
	}

	clamp_anisotropic = {
		wrap_u = "clamp"
		wrap_v = "clamp"
		wrap_w = "clamp"
		min_filter = "anisotropic"
		mag_filter = "anisotropic"
	}

	mirror_point = {
		wrap_u = "mirror"
		wrap_v = "mirror"
		wrap_w = "mirror"
		min_filter = "point"
		mag_filter = "point"
	}

	mirror_anisotropic = {
		wrap_u = "mirror"
		wrap_v = "mirror"
		wrap_w = "mirror"
		min_filter = "anisotropic"
		mag_filter = "anisotropic"
	}
}

bgfx_shaders = {
	common = {
		code = "
			/*
			 * Copyright 2011-2015 Branimir Karadzic. All rights reserved.
			 * License: http://www.opensource.org/licenses/BSD-2-Clause
			 */

			#if !defined(BGFX_CONFIG_MAX_BONES)
			#	define BGFX_CONFIG_MAX_BONES 32
			#endif // !defined(BGFX_CONFIG_MAX_BONES)

			#ifndef __cplusplus

			#if BGFX_SHADER_LANGUAGE_HLSL
			#	define dFdx(_x) ddx(_x)
			#	define dFdy(_y) ddy(-_y)
			#	define inversesqrt(_x) rsqrt(_x)
			#	define fract(_x) frac(_x)

			#	define bvec2 bool2
			#	define bvec3 bool3
			#	define bvec4 bool4

			#	if BGFX_SHADER_LANGUAGE_HLSL > 3
			struct BgfxSampler2D
			{
				SamplerState m_sampler;
				Texture2D m_texture;
			};

			vec4 bgfxTexture2D(BgfxSampler2D _sampler, vec2 _coord)
			{
				return _sampler.m_texture.Sample(_sampler.m_sampler, _coord);
			}

			vec4 bgfxTexture2DLod(BgfxSampler2D _sampler, vec2 _coord, float _level)
			{
				return _sampler.m_texture.SampleLevel(_sampler.m_sampler, _coord, _level);
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

			struct BgfxSampler2DShadow
			{
				SamplerComparisonState m_sampler;
				Texture2D m_texture;
			};

			float bgfxShadow2D(BgfxSampler2DShadow _sampler, vec3 _coord)
			{
				return _sampler.m_texture.SampleCmpLevelZero(_sampler.m_sampler, _coord.xy, _coord.z * 2.0 - 1.0);
			}

			float bgfxShadow2DProj(BgfxSampler2DShadow _sampler, vec4 _coord)
			{
				vec3 coord = _coord.xyz * rcp(_coord.w);
				return _sampler.m_texture.SampleCmpLevelZero(_sampler.m_sampler, coord.xy, coord.z * 2.0 - 1.0);
			}

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
				ivec3 size;
				_sampler.m_texture.GetDimensions(size.x, size.y, size.z);
				return _sampler.m_texture.Load(ivec4(_coord * size, 0) );
			}

			uvec4 bgfxTexture3D(BgfxUSampler3D _sampler, vec3 _coord)
			{
				uvec3 size;
				_sampler.m_texture.GetDimensions(size.x, size.y, size.z);
				return _sampler.m_texture.Load(uvec4(_coord * size, 0) );
			}

			struct BgfxSamplerCube
			{
				SamplerState m_sampler;
				TextureCube m_texture;
			};

			vec4 bgfxTextureCube(BgfxSamplerCube _sampler, vec3 _coord)
			{
				return _sampler.m_texture.Sample(_sampler.m_sampler, _coord);
			}

			vec4 bgfxTextureCubeLod(BgfxSamplerCube _sampler, vec3 _coord, float _level)
			{
				return _sampler.m_texture.SampleLevel(_sampler.m_sampler, _coord, _level);
			}

			#		define SAMPLER2D(_name, _reg) \\
						uniform SamplerState _name ## Sampler : register(s[_reg]); \\
						uniform Texture2D _name ## Texture : register(t[_reg]); \\
						static BgfxSampler2D _name = { _name ## Sampler, _name ## Texture }
			#		define sampler2D BgfxSampler2D
			#		define texture2D(_sampler, _coord) bgfxTexture2D(_sampler, _coord)
			#		define texture2DLod(_sampler, _coord, _level) bgfxTexture2DLod(_sampler, _coord, _level)
			#		define texture2DProj(_sampler, _coord) bgfxTexture2DProj(_sampler, _coord)

			#		define SAMPLER2DSHADOW(_name, _reg) \\
						uniform SamplerComparisonState _name ## Sampler : register(s[_reg]); \\
						uniform Texture2D _name ## Texture : register(t[_reg]); \\
						static BgfxSampler2DShadow _name = { _name ## Sampler, _name ## Texture }
			#		define sampler2DShadow BgfxSampler2DShadow
			#		define shadow2D(_sampler, _coord) bgfxShadow2D(_sampler, _coord)
			#		define shadow2DProj(_sampler, _coord) bgfxShadow2DProj(_sampler, _coord)

			#		define SAMPLER3D(_name, _reg) \\
						uniform SamplerState _name ## Sampler : register(s[_reg]); \\
						uniform Texture3D _name ## Texture : register(t[_reg]); \\
						static BgfxSampler3D _name = { _name ## Sampler, _name ## Texture }
			#		define ISAMPLER3D(_name, _reg) \\
						uniform Texture3D<ivec4> _name ## Texture : register(t[_reg]); \\
						static BgfxISampler3D _name = { _name ## Texture }
			#		define USAMPLER3D(_name, _reg) \\
						uniform Texture3D<uvec4> _name ## Texture : register(t[_reg]); \\
						static BgfxUSampler3D _name = { _name ## Texture }
			#		define sampler3D BgfxSampler3D
			#		define texture3D(_sampler, _coord) bgfxTexture3D(_sampler, _coord)
			#		define texture3DLod(_sampler, _coord, _level) bgfxTexture3DLod(_sampler, _coord, _level)

			#		define SAMPLERCUBE(_name, _reg) \\
						uniform SamplerState _name ## Sampler : register(s[_reg]); \\
						uniform TextureCube _name ## Texture : register(t[_reg]); \\
						static BgfxSamplerCube _name = { _name ## Sampler, _name ## Texture }
			#		define samplerCube BgfxSamplerCube
			#		define textureCube(_sampler, _coord) bgfxTextureCube(_sampler, _coord)
			#		define textureCubeLod(_sampler, _coord, _level) bgfxTextureCubeLod(_sampler, _coord, _level)
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
				return step(_coord.z * 2.0 - 1.0, occluder);
			#else
				return tex2Dproj(_sampler, vec4(_coord.xy, _coord.z * 2.0 - 1.0, 1.0) ).x;
			#endif // 0
			}

			float bgfxShadow2DProj(sampler2DShadow _sampler, vec4 _coord)
			{
			#if 0
				vec3 coord = _coord.xyz * rcp(_coord.w);
				float occluder = tex2D(_sampler, coord.xy).x;
				return step(coord.z * 2.0 - 1.0, occluder);
			#else
				return tex2Dproj(_sampler, _coord).x;
			#endif // 0
			}

			#		define SAMPLER2D(_name, _reg) uniform sampler2D _name : register(s ## _reg)
			#		define texture2D(_sampler, _coord) tex2D(_sampler, _coord)
			#		define texture2DProj(_sampler, _coord) bgfxTexture2DProj(_sampler, _coord)

			#		define SAMPLER2DSHADOW(_name, _reg) uniform sampler2DShadow _name : register(s ## _reg)
			#		define shadow2D(_sampler, _coord) bgfxShadow2D(_sampler, _coord)
			#		define shadow2DProj(_sampler, _coord) bgfxShadow2DProj(_sampler, _coord)

			#		define SAMPLER3D(_name, _reg) uniform sampler3D _name : register(s ## _reg)
			#		define texture3D(_sampler, _coord) tex3D(_sampler, _coord)

			#		define SAMPLERCUBE(_name, _reg) uniform samplerCUBE _name : register(s[_reg])
			#		define textureCube(_sampler, _coord) texCUBE(_sampler, _coord)

			#		if BGFX_SHADER_LANGUAGE_HLSL == 2
			#			define texture2DLod(_sampler, _coord, _level) tex2D(_sampler, (_coord).xy)
			#			define texture3DLod(_sampler, _coord, _level) tex3D(_sampler, (_coord).xyz)
			#			define textureCubeLod(_sampler, _coord, _level) texCUBE(_sampler, (_coord).xyz)
			#		else
			#			define texture2DLod(_sampler, _coord, _level) tex2Dlod(_sampler, vec4( (_coord).xy, 0.0, _level) )
			#			define texture3DLod(_sampler, _coord, _level) tex3Dlod(_sampler, vec4( (_coord).xyz, _level) )
			#			define textureCubeLod(_sampler, _coord, _level) texCUBElod(_sampler, vec4( (_coord).xyz, _level) )
			#		endif // BGFX_SHADER_LANGUAGE_HLSL == 2

			#	endif // BGFX_SHADER_LANGUAGE_HLSL > 3

			vec2 vec2_splat(float _x) { return vec2(_x, _x); }
			vec3 vec3_splat(float _x) { return vec3(_x, _x, _x); }
			vec4 vec4_splat(float _x) { return vec4(_x, _x, _x, _x); }

			uvec2 uvec2_splat(uint _x) { return uvec2(_x, _x); }
			uvec3 uvec3_splat(uint _x) { return uvec3(_x, _x, _x); }
			uvec4 uvec4_splat(uint _x) { return uvec4(_x, _x, _x, _x); }

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
			#	define atan2(_x, _y) atan(_x, _y)
			#	define mul(_a, _b) ( (_a) * (_b) )
			#	define saturate(_x) clamp(_x, 0.0, 1.0)
			#	define SAMPLER2D(_name, _reg) uniform sampler2D _name
			#	define SAMPLER3D(_name, _reg) uniform sampler3D _name
			#	define SAMPLERCUBE(_name, _reg) uniform samplerCube _name
			#	define SAMPLER2DSHADOW(_name, _reg) uniform sampler2DShadow _name
			#	define vec2_splat(_x) vec2(_x)
			#	define vec3_splat(_x) vec3(_x)
			#	define vec4_splat(_x) vec4(_x)
			#	define uvec2_splat(_x) uvec2(_x)
			#	define uvec3_splat(_x) uvec3(_x)
			#	define uvec4_splat(_x) uvec4(_x)

			#	if BGFX_SHADER_LANGUAGE_GLSL >= 130
			#		define ISAMPLER3D(_name, _reg) uniform isampler3D _name
			#		define USAMPLER3D(_name, _reg) uniform usampler3D _name
			ivec4 texture3D(isampler3D _sampler, vec3 _coord) { return texture(_sampler, _coord); }
			uvec4 texture3D(usampler3D _sampler, vec3 _coord) { return texture(_sampler, _coord); }
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

			// Reference:
			// Octahedron normal vector encoding
			// http://kriscg.blogspot.com/2014/04/octahedron-normal-vector-encoding.html
			vec2 octahedronWrap(vec2 _val)
			{
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

			// Reference:
			// RGB/XYZ Matrices
			// http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
			vec3 convertRGB2XYZ(vec3 _rgb)
			{
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
				// Reference:
				// http://www.brucelindbloom.com/index.html?Eqn_XYZ_to_xyY.html
				float inv = 1.0/dot(_xyz, vec3(1.0, 1.0, 1.0) );
				return vec3(_xyz.y, _xyz.x*inv, _xyz.y*inv);
			}

			vec3 convertYxy2XYZ(vec3 _Yxy)
			{
				// Reference:
				// http://www.brucelindbloom.com/index.html?Eqn_xyY_to_XYZ.html
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
		"
	}
}
