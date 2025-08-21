/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/strings/string_id.inl"
#include "core/types.h"
#include "device/pipeline.h"
#include "world/shader_manager.h"
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
}

Pipeline::Pipeline(ShaderManager &sm)
	: _shader_manager(&sm)
	, _color_texture(BGFX_INVALID_HANDLE)
	, _depth_texture(BGFX_INVALID_HANDLE)
	, _frame_buffer(BGFX_INVALID_HANDLE)
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
	, _sun_shadow_map_texture(BGFX_INVALID_HANDLE)
	, _sun_shadow_map_frame_buffer(BGFX_INVALID_HANDLE)
{
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

	_u_cascaded_shadow_map = bgfx::createUniform("u_cascaded_shadow_map", bgfx::UniformType::Sampler);
	_u_cascaded_texel_size = bgfx::createUniform("u_cascaded_texel_size", bgfx::UniformType::Vec4);
	_u_cascaded_lights = bgfx::createUniform("u_cascaded_lights", bgfx::UniformType::Mat4, MAX_NUM_CASCADES);

	// Create cascaded shadow map frame buffer.
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

	_u_local_lights_params = bgfx::createUniform("u_local_lights_params", bgfx::UniformType::Vec4);

	_u_lighting_params = bgfx::createUniform("u_lighting_params", bgfx::UniformType::Vec4);

#if CROWN_PLATFORM_EMSCRIPTEN
	_html5_default_sampler = bgfx::createUniform("s_webgl_hack", bgfx::UniformType::Sampler);
	_html5_default_texture = bgfx::createTexture2D(1, 1, false, 1, bgfx::TextureFormat::R8);
#endif

	PosTexCoord0Vertex::init();
}

void Pipeline::destroy()
{
#if CROWN_PLATFORM_EMSCRIPTEN
	bgfx::destroy(_html5_default_sampler);
	_html5_default_sampler = BGFX_INVALID_HANDLE;
	bgfx::destroy(_html5_default_texture);
	_html5_default_texture = BGFX_INVALID_HANDLE;
#endif
	bgfx::destroy(_u_lighting_params);
	_u_lighting_params = BGFX_INVALID_HANDLE;

	bgfx::destroy(_u_local_lights_params);
	_u_local_lights_params = BGFX_INVALID_HANDLE;

	// Destroy cascaded shadow map resources.
	bgfx::destroy(_u_cascaded_lights);
	_u_cascaded_lights = BGFX_INVALID_HANDLE;
	bgfx::destroy(_u_cascaded_texel_size);
	_u_cascaded_texel_size = BGFX_INVALID_HANDLE;
	bgfx::destroy(_u_cascaded_shadow_map);
	_u_cascaded_shadow_map = BGFX_INVALID_HANDLE;
	bgfx::destroy(_sun_shadow_map_frame_buffer);
	_sun_shadow_map_frame_buffer = BGFX_INVALID_HANDLE;
	bgfx::destroy(_sun_shadow_map_texture);
	_sun_shadow_map_texture = BGFX_INVALID_HANDLE;

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

	bgfx::destroy(_frame_buffer);
	_frame_buffer = BGFX_INVALID_HANDLE;
	bgfx::destroy(_depth_texture);
	_depth_texture = BGFX_INVALID_HANDLE;
	bgfx::destroy(_color_texture);
	_color_texture = BGFX_INVALID_HANDLE;
}

void Pipeline::reset(u16 width, u16 height)
{
	// Create main frame buffer.
	if (bgfx::isValid(_color_texture))
		bgfx::destroy(_color_texture);
	_color_texture = bgfx::createTexture2D(width
		, height
		, false
		, 1
		, bgfx::TextureFormat::BGRA8
		, BGFX_TEXTURE_RT
		);
	if (bgfx::isValid(_depth_texture))
		bgfx::destroy(_depth_texture);
	_depth_texture = bgfx::createTexture2D(width
		, height
		, false
		, 1
		, bgfx::TextureFormat::D24S8
		, BGFX_TEXTURE_RT
		);
	const bgfx::TextureHandle _main_frame_buffer_attachments[] =
	{
		_color_texture,
		_depth_texture
	};
	if (bgfx::isValid(_frame_buffer))
		bgfx::destroy(_frame_buffer);
	_frame_buffer = bgfx::createFrameBuffer(countof(_main_frame_buffer_attachments), _main_frame_buffer_attachments);

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
}

void Pipeline::render(u16 width, u16 height)
{
	const bgfx::Caps *caps = bgfx::getCaps();

	f32 ortho[16];
	bx::mtxOrtho(ortho, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 100.0f, 0.0f, caps->homogeneousDepth, bx::Handedness::Right);

	bgfx::setViewRect(View::BLIT, 0, 0, width, height);
	bgfx::setViewTransform(View::BLIT, NULL, ortho);

	const u32 samplerFlags = 0
		| BGFX_SAMPLER_MIN_POINT
		| BGFX_SAMPLER_MAG_POINT
		| BGFX_SAMPLER_MIP_POINT
		| BGFX_SAMPLER_U_CLAMP
		| BGFX_SAMPLER_V_CLAMP
		;

	bgfx::setTexture(0, _color_map, _color_texture, samplerFlags);
	screenSpaceQuad(width, height, 0.0f, caps->originBottomLeft);
	bgfx::setState(_blit_shader.state);
	bgfx::submit(View::BLIT, _blit_shader.program);

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

	bgfx::setTexture(0, _outline_color_map, _outline_color_texture, samplerFlags);
	bgfx::setViewRect(View::OUTLINE_BLIT, 0, 0, width, height);
	bgfx::setViewTransform(View::OUTLINE_BLIT, NULL, ortho);
	screenSpaceQuad(width, height, 0.0f, caps->originBottomLeft);
	bgfx::setState(_blit_blend_shader.state);
	bgfx::submit(View::OUTLINE_BLIT, _blit_blend_shader.program);
#endif // if !CROWN_PLATFORM_EMSCRIPTEN
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

	bgfx::setUniform(_u_lighting_params, &params, sizeof(params)/sizeof(Vector4));
}

} // namespace crown
