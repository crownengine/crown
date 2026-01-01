/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/strings/string_id.inl"
#include "core/types.h"
#include "device/pipeline.h"
#include "world/shader_manager.h"
#include "core/math/matrix4x4.inl"
#include <bx/math.h>

namespace crown
{
/*
 * Copyright 2011-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */
struct PosTexCoord0Vertex
{
	float m_x;
	float m_y;
	float m_z;
	float m_u;
	float m_v;

	static void init()
	{
		ms_layout.begin();
		ms_layout.add(bgfx::Attrib::Position,  3, bgfx::AttribType::Float);
		ms_layout.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float);
		ms_layout.end();
	}

	static bgfx::VertexLayout ms_layout;
};

bgfx::VertexLayout PosTexCoord0Vertex::ms_layout;

/*
 * Copyright 2011-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */
void screenSpaceQuad(float _textureWidth, float _textureHeight, float _texelHalf, bool _originBottomLeft, float _width = 1.0f, float _height = 1.0f)
{
	if (3 == bgfx::getAvailTransientVertexBuffer(3, PosTexCoord0Vertex::ms_layout)) {
		bgfx::TransientVertexBuffer tvb;
		bgfx::allocTransientVertexBuffer(&tvb, 3, PosTexCoord0Vertex::ms_layout);
		PosTexCoord0Vertex *vertex = (PosTexCoord0Vertex *)tvb.data;

		const float minx = -_width;
		const float maxx =  _width;
		const float miny =  _height;
		const float maxy = -_height;

		const float texelHalfW = _texelHalf/_textureWidth;
		const float texelHalfH = _texelHalf/_textureHeight;
		const float minu = -1.0f + texelHalfW;
		const float maxu =  1.0f + texelHalfH;

		const float zz = 0.0f;

		float minv = texelHalfH;
		float maxv = 2.0f + texelHalfH;

		if (_originBottomLeft) {
			float temp = minv;
			minv = maxv;
			maxv = temp;

			minv -= 1.0f;
			maxv -= 1.0f;
		}

		vertex[0].m_x = maxx;
		vertex[0].m_y = maxy;
		vertex[0].m_z = zz;
		vertex[0].m_u = maxu;
		vertex[0].m_v = maxv;

		vertex[1].m_x = maxx;
		vertex[1].m_y = miny;
		vertex[1].m_z = zz;
		vertex[1].m_u = maxu;
		vertex[1].m_v = minv;

		vertex[2].m_x = minx;
		vertex[2].m_y = miny;
		vertex[2].m_z = zz;
		vertex[2].m_u = minu;
		vertex[2].m_v = minv;

		bgfx::setVertexBuffer(0, &tvb);
	}
}

struct PosVertex
{
	float x;
	float y;
	float z;

	static void init()
	{
		pos_layout.begin();
		pos_layout.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);
		pos_layout.end();
	}

	static bgfx::VertexLayout pos_layout;
};

bgfx::VertexLayout PosVertex::pos_layout;

static u32 best_square_size(u32 width, u32 height, u32 num_tiles)
{
	CE_ENSURE(num_tiles > 0);

	u32 low  = 1;
	u32 high = (width < height ? width : height);
	u32 best = 0;

	while (low <= high) {
		u32 mid = low + ((high - low) >> 1);
		u64 num = (u64)(width / mid) * (u64)(height / mid);

		if (num >= num_tiles) {
			best = mid;
			low = mid + 1;
		} else {
			high = mid - 1;
		}
	}

	return best;
}

static void lookup_default_shaders(Pipeline &pl)
{
	pl._blit_shader = pl._shader_manager->shader(STRING_ID_32("blit", UINT32_C(0x045f02bb)));
	pl._gui_shader = pl._shader_manager->shader(STRING_ID_32("gui", UINT32_C(0x66dbf9a2)));
	pl._gui_3d_shader = pl._shader_manager->shader(STRING_ID_32("gui+DEPTH_ENABLED", UINT32_C(0xd594a1a5)));
	pl._debug_line_depth_enabled_shader = pl._shader_manager->shader(STRING_ID_32("debug_line+DEPTH_ENABLED", UINT32_C(0x8819e848)));
	pl._debug_line_shader = pl._shader_manager->shader(STRING_ID_32("debug_line", UINT32_C(0xbc06e973)));
	pl._outline_shader = pl._shader_manager->shader(STRING_ID_32("outline", UINT32_C(0xb6b58d80)));
	pl._selection_shader = pl._shader_manager->shader(STRING_ID_32("selection", UINT32_C(0x17c0bc11)));
	pl._blit_blend_shader = pl._shader_manager->shader(STRING_ID_32("blit+BLEND_ENABLED", UINT32_C(0xb4fe5db5)));
	pl._shadow_shader = pl._shader_manager->shader(STRING_ID_32("shadow", UINT32_C(0xaceb94a8)));
	pl._shadow_skinning_shader = pl._shader_manager->shader(STRING_ID_32("shadow+SKINNING", UINT32_C(0x34005875)));
	pl._skydome_shader = pl._shader_manager->shader(STRING_ID_32("skydome", UINT32_C(0x524dca1c)));
	pl._bloom_downsample_shader = pl._shader_manager->shader(STRING_ID_32("bloom_downsample", UINT32_C(0x2399e6ad)));
	pl._bloom_upsample_shader = pl._shader_manager->shader(STRING_ID_32("bloom_upsample", UINT32_C(0x26773c9c)));
	pl._bloom_combine_shader = pl._shader_manager->shader(STRING_ID_32("bloom_combine", UINT32_C(0x4413efa4)));
	pl._tonemap_shader = pl._shader_manager->shader(STRING_ID_32("tonemap", UINT32_C(0x7089b06b)));
}

Pipeline::Pipeline(ShaderManager &sm)
	: _shader_manager(&sm)
	, _depth_texture(BGFX_INVALID_HANDLE)
	, _color_map(BGFX_INVALID_HANDLE)
	, _depth_map(BGFX_INVALID_HANDLE)
	, _selection_texture(BGFX_INVALID_HANDLE)
	, _selection_depth_texture(BGFX_INVALID_HANDLE)
	, _selection_frame_buffer(BGFX_INVALID_HANDLE)
	, _selection_map(BGFX_INVALID_HANDLE)
	, _selection_depth_map(BGFX_INVALID_HANDLE)
	, _outline_color_texture(BGFX_INVALID_HANDLE)
	, _outline_frame_buffer(BGFX_INVALID_HANDLE)
	, _outline_color_map(BGFX_INVALID_HANDLE)
	, _outline_color(BGFX_INVALID_HANDLE)
	, _unit_id(BGFX_INVALID_HANDLE)
	, _sun_shadow_map_texture(BGFX_INVALID_HANDLE)
	, _sun_shadow_map_frame_buffer(BGFX_INVALID_HANDLE)
	, _local_lights_shadow_map_texture(BGFX_INVALID_HANDLE)
	, _local_lights_shadow_map_frame_buffer(BGFX_INVALID_HANDLE)
	, _bloom_map(BGFX_INVALID_HANDLE)
	, _map_pixel_size(BGFX_INVALID_HANDLE)
	, _bloom_params(BGFX_INVALID_HANDLE)
	, _tonemap_type(BGFX_INVALID_HANDLE)
{
	for (u32 i = 0; i < countof(_color_textures); ++i)
		_color_textures[i] = BGFX_INVALID_HANDLE;

	for (u32 i = 0; i < countof(_colors); ++i)
		_colors[i] = BGFX_INVALID_HANDLE;

	// Bloom.
	for (u32 i = 0; i < countof(_bloom_frame_buffers); ++i)
		_bloom_frame_buffers[i] = BGFX_INVALID_HANDLE;

	lookup_default_shaders(*this);
}

void Pipeline::create(u16 width, u16 height, const RenderSettings &render_settings)
{
	_render_settings = render_settings;

	reset(width, height);

	_color_map = bgfx::createUniform("s_color_map", bgfx::UniformType::Sampler);
	_depth_map = bgfx::createUniform("s_depth_map", bgfx::UniformType::Sampler);

	_selection_map = bgfx::createUniform("s_selection_map", bgfx::UniformType::Sampler);
	_selection_depth_map = bgfx::createUniform("s_selection_depth_map", bgfx::UniformType::Sampler);

	_outline_color_map = bgfx::createUniform("s_color_map", bgfx::UniformType::Sampler);
	_outline_color = bgfx::createUniform("u_outline_color", bgfx::UniformType::Vec4);
	_unit_id = bgfx::createUniform("u_unit_id", bgfx::UniformType::Vec4);

	_u_cascaded_shadow_map = bgfx::createUniform("u_cascaded_shadow_map", bgfx::UniformType::Sampler);
	_u_cascaded_lights = bgfx::createUniform("u_cascaded_lights", bgfx::UniformType::Mat4, MAX_NUM_CASCADES);
	_u_shadow_maps_texel_sizes = bgfx::createUniform("u_shadow_maps_texel_sizes", bgfx::UniformType::Vec4);

	// Create cascaded shadow map resources.
	if (bgfx::isValid(_sun_shadow_map_texture))
		bgfx::destroy(_sun_shadow_map_texture);
	_sun_shadow_map_texture = bgfx::createTexture2D(_render_settings.sun_shadow_map_size.x
		, _render_settings.sun_shadow_map_size.y
		, false
		, 1
		, bgfx::TextureFormat::D32F
		, BGFX_TEXTURE_RT | BGFX_SAMPLER_COMPARE_LEQUAL
		);
	const bgfx::TextureHandle fbtextures[] =
	{
		_sun_shadow_map_texture
	};
	if (bgfx::isValid(_sun_shadow_map_frame_buffer))
		bgfx::destroy(_sun_shadow_map_frame_buffer);
	_sun_shadow_map_frame_buffer = bgfx::createFrameBuffer(countof(fbtextures), fbtextures);

	// Create local-lights shadow map resources.
	if (bgfx::isValid(_local_lights_shadow_map_texture))
		bgfx::destroy(_local_lights_shadow_map_texture);
	_local_lights_shadow_map_texture = bgfx::createTexture2D(_render_settings.local_lights_shadow_map_size.x
		, _render_settings.local_lights_shadow_map_size.y
		, false
		, 1
		, bgfx::TextureFormat::D24S8
		, BGFX_TEXTURE_RT | BGFX_SAMPLER_COMPARE_LEQUAL
		);
	const bgfx::TextureHandle llfbtextures[] =
	{
		_local_lights_shadow_map_texture
	};
	if (bgfx::isValid(_local_lights_shadow_map_frame_buffer))
		bgfx::destroy(_local_lights_shadow_map_frame_buffer);
	_local_lights_shadow_map_frame_buffer = bgfx::createFrameBuffer(countof(llfbtextures), llfbtextures);

	// FIXME: this is a pretty dumb allocation scheme but it's fine for now.
	_local_lights_tile_size = best_square_size(_render_settings.local_lights_shadow_map_size.x
		, _render_settings.local_lights_shadow_map_size.y
		, LOCAL_LIGHTS_MAX_SHADOW_CASTERS
		);
	_local_lights_tile_cols = _render_settings.local_lights_shadow_map_size.x / _local_lights_tile_size;

	_u_local_lights_shadow_map = bgfx::createUniform("u_local_lights_shadow_map", bgfx::UniformType::Sampler);
	_u_local_lights_params = bgfx::createUniform("u_local_lights_params", bgfx::UniformType::Vec4);

	_lights_num = bgfx::createUniform("u_lights_num", bgfx::UniformType::Vec4, 1);
	_lights_data = bgfx::createUniform("u_lights_data", bgfx::UniformType::Sampler);
	_lights_data_texture = bgfx::createTexture2D(MAX_NUM_LIGHTS * LIGHT_SIZE
		, 1
		, false
		, 1
		, bgfx::TextureFormat::RGBA32F
		, BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT
		);

	_fog_data = bgfx::createUniform("u_fog_data", bgfx::UniformType::Vec4, 2);
	_lighting_params = bgfx::createUniform("u_lighting_params", bgfx::UniformType::Vec4);

	_bloom_map = bgfx::createUniform("s_bloom_map", bgfx::UniformType::Sampler);
	_map_pixel_size = bgfx::createUniform("u_map_pixel_size", bgfx::UniformType::Vec4);
	_bloom_params = bgfx::createUniform("u_bloom_params", bgfx::UniformType::Vec4);

	_tonemap_type = bgfx::createUniform("u_tonemap_type", bgfx::UniformType::Vec4);

#if CROWN_PLATFORM_EMSCRIPTEN
	_html5_default_sampler = bgfx::createUniform("s_webgl_hack", bgfx::UniformType::Sampler);
	_html5_default_texture = bgfx::createTexture2D(1, 1, false, 1, bgfx::TextureFormat::R8);
#endif

	_u_uv_scale = bgfx::createUniform("u_uv_scale", bgfx::UniformType::Vec4);
	_u_uv_offset = bgfx::createUniform("u_uv_offset", bgfx::UniformType::Vec4);

	PosTexCoord0Vertex::init();
	PosVertex::init();
}

void Pipeline::destroy()
{
	bgfx::destroy(_u_uv_offset);
	bgfx::destroy(_u_uv_scale);

#if CROWN_PLATFORM_EMSCRIPTEN
	bgfx::destroy(_html5_default_sampler);
	_html5_default_sampler = BGFX_INVALID_HANDLE;
	bgfx::destroy(_html5_default_texture);
	_html5_default_texture = BGFX_INVALID_HANDLE;
#endif

	bgfx::destroy(_lighting_params);
	_lighting_params = BGFX_INVALID_HANDLE;
	bgfx::destroy(_fog_data);
	_fog_data = BGFX_INVALID_HANDLE;
	bgfx::destroy(_lights_data_texture);
	_lights_data_texture = BGFX_INVALID_HANDLE;
	bgfx::destroy(_lights_data);
	_lights_data = BGFX_INVALID_HANDLE;
	bgfx::destroy(_lights_num);
	_lights_num = BGFX_INVALID_HANDLE;

	// Destroy local-lights shadow map resources.
	bgfx::destroy(_u_local_lights_params);
	_u_local_lights_params = BGFX_INVALID_HANDLE;
	bgfx::destroy(_u_local_lights_shadow_map);
	_u_local_lights_shadow_map = BGFX_INVALID_HANDLE;
	bgfx::destroy(_local_lights_shadow_map_frame_buffer);
	_local_lights_shadow_map_frame_buffer = BGFX_INVALID_HANDLE;
	bgfx::destroy(_local_lights_shadow_map_texture);
	_local_lights_shadow_map_texture = BGFX_INVALID_HANDLE;

	// Destroy cascaded shadow map resources.
	bgfx::destroy(_u_cascaded_lights);
	_u_cascaded_lights = BGFX_INVALID_HANDLE;
	bgfx::destroy(_u_shadow_maps_texel_sizes);
	_u_shadow_maps_texel_sizes = BGFX_INVALID_HANDLE;
	bgfx::destroy(_u_cascaded_shadow_map);
	_u_cascaded_shadow_map = BGFX_INVALID_HANDLE;
	bgfx::destroy(_sun_shadow_map_frame_buffer);
	_sun_shadow_map_frame_buffer = BGFX_INVALID_HANDLE;
	bgfx::destroy(_sun_shadow_map_texture);
	_sun_shadow_map_texture = BGFX_INVALID_HANDLE;

	// Destroy tonemap resources.
	bgfx::destroy(_tonemap_type);
	_tonemap_type = BGFX_INVALID_HANDLE;

	// Destroy bloom resources.
	bgfx::destroy(_bloom_params);
	_bloom_params = BGFX_INVALID_HANDLE;

	bgfx::destroy(_map_pixel_size);
	_map_pixel_size = BGFX_INVALID_HANDLE;

	bgfx::destroy(_bloom_map);
	_bloom_map = BGFX_INVALID_HANDLE;

	for (u32 i = 0; i < countof(_bloom_frame_buffers); ++i) {
		if (bgfx::isValid(_bloom_frame_buffers[i]))
			bgfx::destroy(_bloom_frame_buffers[i]);
		_bloom_frame_buffers[i] = BGFX_INVALID_HANDLE;
	}

	bgfx::destroy(_unit_id);
	_unit_id = BGFX_INVALID_HANDLE;
	bgfx::destroy(_outline_color);
	_outline_color = BGFX_INVALID_HANDLE;
	bgfx::destroy(_outline_color_map);
	_outline_color_map = BGFX_INVALID_HANDLE;

	bgfx::destroy(_outline_frame_buffer);
	_outline_frame_buffer = BGFX_INVALID_HANDLE;
	bgfx::destroy(_outline_color_texture);
	_outline_color_texture = BGFX_INVALID_HANDLE;

	bgfx::destroy(_selection_depth_map);
	_selection_depth_map = BGFX_INVALID_HANDLE;
	bgfx::destroy(_selection_map);
	_selection_map = BGFX_INVALID_HANDLE;

	bgfx::destroy(_depth_map);
	_depth_map = BGFX_INVALID_HANDLE;
	bgfx::destroy(_color_map);
	_color_map = BGFX_INVALID_HANDLE;

	bgfx::destroy(_selection_frame_buffer);
	_selection_frame_buffer = BGFX_INVALID_HANDLE;
	bgfx::destroy(_selection_depth_texture);
	_selection_depth_texture = BGFX_INVALID_HANDLE;
	bgfx::destroy(_selection_texture);
	_selection_texture = BGFX_INVALID_HANDLE;

	for (u32 i = 0; i < countof(_colors); ++i) {
		bgfx::destroy(_colors[i]);
		_colors[i] = BGFX_INVALID_HANDLE;
	}

	bgfx::destroy(_depth_texture);
	_depth_texture = BGFX_INVALID_HANDLE;

	for (u32 i = 0; i < countof(_color_textures); ++i) {
		bgfx::destroy(_color_textures[i]);
		_color_textures[i] = BGFX_INVALID_HANDLE;
	}
}

void Pipeline::reset(u16 width, u16 height)
{
	u64 depth_texture_flags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
	u64 color_texture_flags = BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
	if ((_render_settings.flags & RenderSettingsFlags::MSAA) != 0) {
		u64 msaa_flags = u64(1 + _render_settings.msaa_quality) << BGFX_TEXTURE_RT_MSAA_SHIFT;
		depth_texture_flags |= msaa_flags | BGFX_TEXTURE_RT_WRITE_ONLY;
		color_texture_flags |= msaa_flags;
	} else {
		depth_texture_flags |= BGFX_TEXTURE_RT;
		color_texture_flags |= BGFX_TEXTURE_RT;
	}

	// Create main frame buffers.
	if (bgfx::isValid(_depth_texture))
		bgfx::destroy(_depth_texture);
	_depth_texture = bgfx::createTexture2D(width
		, height
		, false
		, 1
		, bgfx::TextureFormat::D24S8
		, depth_texture_flags
		);

	for (u32 i = 0; i < countof(_color_textures); ++i) {
		if (bgfx::isValid(_color_textures[i]))
			bgfx::destroy(_color_textures[i]);
		_color_textures[i] = bgfx::createTexture2D(width
			, height
			, false
			, 1
			, bgfx::TextureFormat::RGBA16F
			, color_texture_flags
			);
		const bgfx::TextureHandle _main_frame_buffer_attachments[] =
		{
			_color_textures[i],
			_depth_texture
		};
		if (bgfx::isValid(_colors[i]))
			bgfx::destroy(_colors[i]);
		_colors[i] = bgfx::createFrameBuffer(countof(_main_frame_buffer_attachments), _main_frame_buffer_attachments);
	}

	// Create selection frame buffer.
	if (bgfx::isValid(_selection_texture))
		bgfx::destroy(_selection_texture);
	_selection_texture = bgfx::createTexture2D(width
		, height
		, false
		, 1
		, bgfx::TextureFormat::R32U
		, BGFX_TEXTURE_RT
		);
	if (bgfx::isValid(_selection_depth_texture))
		bgfx::destroy(_selection_depth_texture);
	_selection_depth_texture = bgfx::createTexture2D(width
		, height
		, false
		, 1
		, bgfx::TextureFormat::D24
		, BGFX_TEXTURE_RT
		);
	const bgfx::TextureHandle _selection_frame_buffer_attachments[] =
	{
		_selection_texture,
		_selection_depth_texture
	};
	if (bgfx::isValid(_selection_frame_buffer))
		bgfx::destroy(_selection_frame_buffer);
	_selection_frame_buffer = bgfx::createFrameBuffer(countof(_selection_frame_buffer_attachments), _selection_frame_buffer_attachments);

	// Create outline frame buffer.
	if (bgfx::isValid(_outline_color_texture))
		bgfx::destroy(_outline_color_texture);
	_outline_color_texture = bgfx::createTexture2D(width
		, height
		, false
		, 1
		, bgfx::TextureFormat::BGRA8
		, BGFX_TEXTURE_RT
		);

	const bgfx::TextureHandle _outline_frame_buffer_attachments[] =
	{
		_outline_color_texture
	};
	if (bgfx::isValid(_outline_frame_buffer))
		bgfx::destroy(_outline_frame_buffer);
	_outline_frame_buffer = bgfx::createFrameBuffer(countof(_outline_frame_buffer_attachments), _outline_frame_buffer_attachments);

	// Bloom.
	if ((_render_settings.flags & RenderSettingsFlags::BLOOM) != 0) {
		for (u32 i = 0; i < countof(_bloom_frame_buffers); ++i) {
			if (bgfx::isValid(_bloom_frame_buffers[i]))
				bgfx::destroy(_bloom_frame_buffers[i]);

			_bloom_frame_buffers[i] = bgfx::createFrameBuffer(width >> i
				, height >> i
				, bgfx::TextureFormat::RGBA16F
				, BGFX_TEXTURE_RT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP
				);
		}
	}
}

void Pipeline::render(u16 width, u16 height, const Matrix4x4 &view, const Matrix4x4 &proj)
{
	const bgfx::Caps *caps = bgfx::getCaps();

	f32 ortho[16];
	bx::mtxOrtho(ortho, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 100.0f, 0.0f, caps->homogeneousDepth, bx::Handedness::Right);

	const u32 samplerFlags = 0
		| BGFX_SAMPLER_MIN_POINT
		| BGFX_SAMPLER_MAG_POINT
		| BGFX_SAMPLER_MIP_POINT
		| BGFX_SAMPLER_U_CLAMP
		| BGFX_SAMPLER_V_CLAMP
		;

	const u32 bloom_sampler_flags = 0
		| BGFX_SAMPLER_MIN_ANISOTROPIC
		| BGFX_SAMPLER_MAG_ANISOTROPIC
		| BGFX_SAMPLER_MIP_POINT
		| BGFX_SAMPLER_U_CLAMP
		| BGFX_SAMPLER_V_CLAMP
		;

	for (u32 id = 0; id < View::COUNT; ++id) {
		const char *view_name;

		if (id >= View::SPRITE_0 && id < View::SPRITE_LAST) {
			view_name = "sprite";
			bgfx::setViewTransform(id, to_float_ptr(view), to_float_ptr(proj));
			bgfx::setViewRect(id, 0, 0, width, height);
			bgfx::setViewMode(id, bgfx::ViewMode::DepthAscending);
			bgfx::setViewFrameBuffer(id, _colors[0]);
			bgfx::touch(id);
		} else if (id == View::CASCADE_CLEAR) {
			view_name = "sm_cascade_clear";
			bgfx::setViewFrameBuffer(id, _sun_shadow_map_frame_buffer);
			bgfx::setViewRect(id, 0, 0, (u16)_render_settings.sun_shadow_map_size.x, (u16)_render_settings.sun_shadow_map_size.y);
			bgfx::setViewClear(id, BGFX_CLEAR_DEPTH, 0xffffffff, 1.0f, 0);
			bgfx::touch(id);
		} else if (id >= View::CASCADE_0 && id < View::CASCADE_LAST) {
			view_name = "sm_cascade";
		} else if (id == View::SM_LOCAL_CLEAR) {
			view_name = "sm_local_lights_clear";
			Matrix4x4 screen_proj;
			bx::mtxOrtho(to_float_ptr(screen_proj)
				, 0.0f
				, 1.0f
				, 1.0f
				, 0.0f
				, 0.0f
				, 100.0f
				, 0.0f
				, caps->homogeneousDepth
				);
			bgfx::setViewClear(id, BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, 0, 1.0f, 0);
			bgfx::setViewFrameBuffer(id, _local_lights_shadow_map_frame_buffer);
			bgfx::setViewRect(id, 0, 0, (u16)_render_settings.local_lights_shadow_map_size.x, (u16)_render_settings.local_lights_shadow_map_size.y);
			bgfx::setViewTransform(id, to_float_ptr(MATRIX4X4_IDENTITY), to_float_ptr(screen_proj));

			// Draw stencil "hourglass" pattern for omni lights.
			const u16 sm_w = (u16)_render_settings.local_lights_shadow_map_size.x;
			const f32 step = f32(_local_lights_tile_size) / f32(sm_w) * 0.5;
			const s32 num_cols = sm_w / _local_lights_tile_size;
			const s32 num_rows = num_cols;
			const s32 num_pins = num_cols + 1;
			const s32 num_necks = num_pins - 1;
			const u32 num_vertices = num_pins*num_pins + num_necks*num_necks;
			const u32 num_triangles = num_necks*num_necks * 2;
			const u32 num_indices = num_triangles * 3;

			if (bgfx::getAvailTransientVertexBuffer(num_vertices, PosVertex::pos_layout) == num_vertices
				&& bgfx::getAvailTransientIndexBuffer(num_indices) == num_indices) {
				// Build vertex buffer.
				bgfx::TransientVertexBuffer vb;
				bgfx::allocTransientVertexBuffer(&vb, num_vertices, PosVertex::pos_layout);
				PosVertex *v = (PosVertex *)vb.data;

				for (s32 h = 0; h < num_pins + num_necks; ++h) {
					s32 start_w = h % 2;
					for (s32 w = start_w; w < num_pins + num_necks; w += 2) {
						const f32 xi = w * step;
						const f32 yi = h * step;
						*v++ = { xi, yi, 0.0f };
					}
				}

				// Build index buffer.
				bgfx::TransientIndexBuffer ib;
				bgfx::allocTransientIndexBuffer(&ib, num_indices);
				u16 *ind = (u16 *)ib.data;

				const s32 gap = num_cols + 1;
				const s32 row_stride = 2 * gap - 1;

				for (s32 r = 0; r < num_rows; ++r) {
					for (s32 c = 0; c < num_cols; ++c) {
						const s32 t = r * row_stride + c;
						// Top triangle.
						*ind++ = t;
						*ind++ = t + 1;
						*ind++ = t + gap;
						// Bottom triangle.
						*ind++ = t + gap;
						*ind++ = t + 2 * gap;
						*ind++ = t + 2 * gap - 1;
					}
				}

				bgfx::setState(0);
				bgfx::setStencil(BGFX_STENCIL_TEST_ALWAYS
					| BGFX_STENCIL_FUNC_REF(1)
					| BGFX_STENCIL_FUNC_RMASK(0xff)
					| BGFX_STENCIL_OP_FAIL_S_REPLACE
					| BGFX_STENCIL_OP_FAIL_Z_REPLACE
					| BGFX_STENCIL_OP_PASS_Z_REPLACE
					);
				bgfx::setVertexBuffer(0, &vb);
				bgfx::setIndexBuffer(&ib);
				bgfx::submit(id, _shadow_shader.program);
			}
		} else if (id >= View::SM_LOCAL_0 && id < View::SM_LOCAL_LAST) {
			view_name = "sm_local_lights";
		} else if (id == View::LIGHTS) {
			view_name = "lights_data";
		} else if (id == View::MESH) {
			view_name = "mesh";
			bgfx::setViewTransform(id, to_float_ptr(view), to_float_ptr(proj));
			bgfx::setViewRect(id, 0, 0, width, height);
			bgfx::setViewFrameBuffer(id, _colors[0]);
			bgfx::touch(id);
		} else if (id == View::BLOOM_COPY) {
			view_name = "bloom_copy";
			bgfx::setViewRect(id, 0, 0, width, height);
			bgfx::touch(id);
		} else if (id >= View::BLOOM_DOWNSAMPLE_0 && id < View::BLOOM_DOWNSAMPLE_LAST) {
			view_name = "bloom_downsample";
			bgfx::touch(id);
		} else if (id >= View::BLOOM_UPSAMPLE_0 && id < View::BLOOM_UPSAMPLE_LAST) {
			view_name = "bloom_upsample";
			bgfx::touch(id);
		} else if (id == View::BLOOM_COMBINE) {
			view_name = "bloom_combine";
			bgfx::touch(id);
		} else if (id == View::TONEMAP) {
			view_name = "tonemap";
			bgfx::touch(id);
		} else if (id == View::WORLD_GUI) {
			view_name = "world_gui";
			bgfx::setViewTransform(id, to_float_ptr(view), to_float_ptr(proj));
			bgfx::setViewRect(id, 0, 0, width, height);
			bgfx::setViewMode(id, bgfx::ViewMode::DepthDescending);
			bgfx::setViewFrameBuffer(id, _colors[0]);
			bgfx::touch(id);
		} else if (id == View::SELECTION) {
			view_name = "selection";
#if !CROWN_PLATFORM_EMSCRIPTEN
			bgfx::setViewClear(id, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, UNIT_INVALID._idx, 1.0f, 0);
			bgfx::setViewTransform(id, to_float_ptr(view), to_float_ptr(proj));
			bgfx::setViewRect(id, 0, 0, width, height);
			bgfx::setViewFrameBuffer(id, _selection_frame_buffer);
			bgfx::touch(id);
#endif
		} else if (id == View::OUTLINE) {
			view_name = "outline";
#if !CROWN_PLATFORM_EMSCRIPTEN
			bgfx::setViewClear(id, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0xffffffff);
			bgfx::setViewFrameBuffer(id, _outline_frame_buffer);
			bgfx::setViewRect(id, 0, 0, width, height);
			bgfx::touch(id);
#endif
		} else if (id == View::OUTLINE_BLIT) {
			view_name = "outline_blit";
#if !CROWN_PLATFORM_EMSCRIPTEN
			bgfx::setViewRect(id, 0, 0, width, height);
			bgfx::touch(id);
#endif
		} else if (id == View::DEBUG) {
			view_name = "debug";
			bgfx::setViewTransform(id, to_float_ptr(view), to_float_ptr(proj));
			bgfx::setViewRect(id, 0, 0, width, height);
			bgfx::setViewFrameBuffer(id, _colors[0]);
			bgfx::touch(id);
		} else if (id == View::SCREEN_GUI) {
			view_name = "screen_gui";
			f32 bx_ortho[16];
			bx::mtxOrtho(bx_ortho
				, 0
				, width
				, 0
				, height
				, 0.0f
				, 1.0f
				, 0.0f
				, caps->homogeneousDepth
				, bx::Handedness::Right
				);
			Matrix4x4 ortho_proj = from_array(bx_ortho);
			bgfx::setViewTransform(id, to_float_ptr(MATRIX4X4_IDENTITY), to_float_ptr(ortho_proj));
			bgfx::setViewRect(id, 0, 0, width, height);
			bgfx::setViewMode(id, bgfx::ViewMode::DepthDescending);
			bgfx::setViewFrameBuffer(id, _colors[0]);
			bgfx::touch(id);
		} else if (id == View::GRAPH) {
			view_name = "graph";
			f32 graph_ortho[16];
			bx::mtxOrtho(graph_ortho
				, -width / 2.0f
				,  width / 2.0f
				, -height / 2.0f
				,  height / 2.0f
				, 0.0f
				, 1.0f
				, 0.0f
				, caps->homogeneousDepth
				, bx::Handedness::Right
				);
			bgfx::setViewTransform(id, to_float_ptr(MATRIX4X4_IDENTITY), to_float_ptr(from_array(graph_ortho)));
			bgfx::setViewRect(id, 0, 0, width, height);
			bgfx::setViewFrameBuffer(id, _colors[0]);
			bgfx::touch(id);
		} else if (id == View::BLIT) {
			view_name = "blit";
			bgfx::setViewMode(id, bgfx::ViewMode::Sequential);
			bgfx::setViewFrameBuffer(id, BGFX_INVALID_HANDLE);
			bgfx::touch(id);
		} else if (id == View::IMGUI) {
			view_name = "imgui";
		} else {
			view_name = "unknown";
		}

		bgfx::setViewName(id, view_name);
	}

	// Clear main color frame buffers.
	bgfx::setViewFrameBuffer(View::COLOR_0, _colors[0]);
	bgfx::setViewClear(View::COLOR_0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x080808ff, 1.0f, 0);
	bgfx::setViewRect(View::COLOR_0, 0, 0, width, height);
	bgfx::setViewName(View::COLOR_0, "color0");
	bgfx::touch(View::COLOR_0);

	bgfx::setViewFrameBuffer(View::COLOR_1, _colors[1]);
	bgfx::setViewClear(View::COLOR_1, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x080808ff, 1.0f, 0);
	bgfx::setViewRect(View::COLOR_1, 0, 0, width, height);
	bgfx::setViewName(View::COLOR_1, "color1");
	bgfx::touch(View::COLOR_1);

	// Render bloom.
	if ((_render_settings.flags & RenderSettingsFlags::BLOOM) != 0 && _bloom.enabled) {
		Vector4 bloom_params;
		bloom_params.x = _bloom.weight;
		bloom_params.y = _bloom.intensity;
		bloom_params.z = _bloom.threshold;

		// Copy color buffer to first bloom mip.
		bgfx::setViewFrameBuffer(View::BLOOM_COPY, _bloom_frame_buffers[0]);
		bgfx::setViewTransform(View::BLOOM_COPY, NULL, ortho);
		bgfx::setTexture(0, _color_map, bgfx::getTexture(_colors[0]), bloom_sampler_flags);
		screenSpaceQuad(width, height, 0.0f, caps->originBottomLeft);
		bgfx::setState(_blit_shader.state);
		bgfx::submit(View::BLOOM_COPY, _blit_shader.program);

		// Downsample.
		for (u32 i = 0; i < countof(_bloom_frame_buffers) - 1; ++i) {
			const u16 shift = i + 1;
			const u16 w = width >> shift;
			const u16 h = height >> shift;
			Vector4 pixel_size = { 1.0f/w, 1.0f/h, 0.0f, 0.0f };

			bgfx::setViewTransform(View::BLOOM_DOWNSAMPLE_0 + i, NULL, ortho);
			bgfx::setViewRect(View::BLOOM_DOWNSAMPLE_0 + i, 0, 0, w, h);
			bgfx::setViewFrameBuffer(View::BLOOM_DOWNSAMPLE_0 + i, _bloom_frame_buffers[shift]);
			bgfx::setTexture(0, _color_map, bgfx::getTexture(_bloom_frame_buffers[i]), bloom_sampler_flags);
			bgfx::setUniform(_map_pixel_size, &pixel_size, sizeof(pixel_size)/sizeof(Vector4));
			screenSpaceQuad(w, h, 0.0f, caps->originBottomLeft);
			bgfx::setState(_bloom_downsample_shader.state);
			bgfx::submit(View::BLOOM_DOWNSAMPLE_0 + i, _bloom_downsample_shader.program);
		}

		// Upsample.
		for (u32 i = 0; i < countof(_bloom_frame_buffers) - 1; ++i) {
			const u16 shift = countof(_bloom_frame_buffers) - 2 - i;
			const u16 w = width >> shift;
			const u16 h = height >> shift;
			Vector4 pixel_size = { 1.0f/w, 1.0f/h, 0.0f, 0.0f };

			bgfx::setViewTransform(View::BLOOM_UPSAMPLE_0 + i, NULL, ortho);
			bgfx::setViewRect(View::BLOOM_UPSAMPLE_0 + i, 0, 0, w, h);
			bgfx::setViewFrameBuffer(View::BLOOM_UPSAMPLE_0 + i, _bloom_frame_buffers[shift]);
			bgfx::setTexture(0, _color_map, bgfx::getTexture(_bloom_frame_buffers[shift + 1]), bloom_sampler_flags);
			bgfx::setUniform(_map_pixel_size, &pixel_size, sizeof(pixel_size)/sizeof(Vector4));
			bgfx::setUniform(_bloom_params, &bloom_params, sizeof(bloom_params)/sizeof(Vector4));
			screenSpaceQuad(w, h, 0.0f, caps->originBottomLeft);
			bgfx::setState(_bloom_upsample_shader.state);
			bgfx::submit(View::BLOOM_UPSAMPLE_0 + i, _bloom_upsample_shader.program);
		}

		// Combine first bloom mip with main color texture.
		bgfx::setViewFrameBuffer(View::BLOOM_COMBINE, _colors[1]);
		bgfx::setViewTransform(View::BLOOM_COMBINE, NULL, ortho);
		bgfx::setViewRect(View::BLOOM_COMBINE, 0, 0, width, height);
		bgfx::setTexture(0, _color_map, bgfx::getTexture(_colors[0]), samplerFlags);
		bgfx::setTexture(1, _bloom_map, bgfx::getTexture(_bloom_frame_buffers[0]), samplerFlags);
		bgfx::setUniform(_bloom_params, &bloom_params, sizeof(bloom_params)/sizeof(Vector4));
		screenSpaceQuad(width, height, 0.0f, caps->originBottomLeft);
		bgfx::setState(_bloom_combine_shader.state);
		bgfx::submit(View::BLOOM_COMBINE, _bloom_combine_shader.program);
	} else {
		// Do a dummy copy to color1.
		bgfx::setViewFrameBuffer(View::DUMMY_BLIT, _colors[1]);
		bgfx::setViewTransform(View::DUMMY_BLIT, NULL, ortho);
		bgfx::setViewRect(View::DUMMY_BLIT, 0, 0, width, height);
		bgfx::setTexture(0, _color_map, bgfx::getTexture(_colors[0]), samplerFlags);
		screenSpaceQuad(width, height, 0.0f, caps->originBottomLeft);
		bgfx::setState(_blit_shader.state);
		bgfx::submit(View::DUMMY_BLIT, _blit_shader.program);
	}

	// Tonemapping.
	bgfx::setViewFrameBuffer(View::TONEMAP, _colors[0]);
	bgfx::setViewTransform(View::TONEMAP, NULL, ortho);
	bgfx::setViewRect(View::TONEMAP, 0, 0, width, height);
	bgfx::setTexture(0, _color_map, bgfx::getTexture(_colors[1]), samplerFlags);
	bgfx::setUniform(_tonemap_type, &_tonemap, sizeof(_tonemap)/sizeof(Vector4));
	screenSpaceQuad(width, height, 0.0f, caps->originBottomLeft);
	bgfx::setState(_tonemap_shader.state);
	bgfx::submit(View::TONEMAP, _tonemap_shader.program);

#if !CROWN_PLATFORM_EMSCRIPTEN
	bgfx::setTexture(0, _selection_map, _selection_texture, samplerFlags);
	bgfx::setTexture(1, _selection_depth_map, _selection_depth_texture, samplerFlags);
	bgfx::setTexture(2, _depth_map, _depth_texture, samplerFlags);
	bgfx::setViewRect(View::OUTLINE, 0, 0, width, height);
	bgfx::setViewTransform(View::OUTLINE, NULL, ortho);
	screenSpaceQuad(width, height, 0.0f, caps->originBottomLeft);
	const f32 outline_color[] = { 1.0f, 0.37f, 0.05f, 1.0f };
	bgfx::setUniform(_outline_color, outline_color);
	bgfx::setState(_outline_shader.state);
	bgfx::submit(View::OUTLINE, _outline_shader.program);

	bgfx::setViewFrameBuffer(View::OUTLINE_BLIT, _colors[0]);
	bgfx::setTexture(0, _outline_color_map, _outline_color_texture, samplerFlags);
	bgfx::setViewRect(View::OUTLINE_BLIT, 0, 0, width, height);
	bgfx::setViewTransform(View::OUTLINE_BLIT, NULL, ortho);
	screenSpaceQuad(width, height, 0.0f, caps->originBottomLeft);
	bgfx::setState(_blit_blend_shader.state);
	bgfx::submit(View::OUTLINE_BLIT, _blit_blend_shader.program);
#endif // if !CROWN_PLATFORM_EMSCRIPTEN

	// Blit to backbuffer.
	bgfx::setViewFrameBuffer(View::BLIT, BGFX_INVALID_HANDLE);
	bgfx::setViewRect(View::BLIT, 0, 0, width, height);
	bgfx::setViewTransform(View::BLIT, NULL, ortho);
	bgfx::setTexture(0, _color_map, bgfx::getTexture(_colors[0]), samplerFlags);
	screenSpaceQuad(width, height, 0.0f, caps->originBottomLeft);
	bgfx::setState(_blit_shader.state);
	bgfx::submit(View::BLIT, _blit_shader.program);
}

void Pipeline::reload_shaders(const ShaderResource *old_resource, const ShaderResource *new_resource)
{
	CE_UNUSED_2(old_resource, new_resource);
	lookup_default_shaders(*this);
}

void Pipeline::set_local_lights_params_uniform()
{
	Vector4 params;
	params.x = _render_settings.flags & RenderSettingsFlags::LOCAL_LIGHTS_DISTANCE_CULLING;
	params.y = _render_settings.local_lights_distance_culling_fade;
	params.z = _render_settings.local_lights_distance_culling_cutoff;

	bgfx::setUniform(_u_local_lights_params, &params, sizeof(params)/sizeof(Vector4));
}

void Pipeline::set_global_lighting_params(GlobalLightingDesc *global_lighting)
{
	Vector4 params;
	params.x = global_lighting->ambient_color.x;
	params.y = global_lighting->ambient_color.y;
	params.z = global_lighting->ambient_color.z;

	bgfx::setUniform(_lighting_params, &params, sizeof(params)/sizeof(Vector4));
}

} // namespace crown
