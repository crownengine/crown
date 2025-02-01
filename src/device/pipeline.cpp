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
}

Pipeline::Pipeline(ShaderManager &sm)
	: _shader_manager(&sm)
	, _main_color_texture(BGFX_INVALID_HANDLE)
	, _main_depth_texture(BGFX_INVALID_HANDLE)
	, _main_frame_buffer(BGFX_INVALID_HANDLE)
	, _main_color_texture_sampler(BGFX_INVALID_HANDLE)
	, _main_depth_texture_sampler(BGFX_INVALID_HANDLE)
	, _selection_texture(BGFX_INVALID_HANDLE)
	, _selection_depth_texture(BGFX_INVALID_HANDLE)
	, _selection_frame_buffer(BGFX_INVALID_HANDLE)
	, _selection_texture_sampler(BGFX_INVALID_HANDLE)
	, _selection_depth_texture_sampler(BGFX_INVALID_HANDLE)
	, _outline_color_uniform(BGFX_INVALID_HANDLE)
{
	lookup_default_shaders(*this);
}

void Pipeline::create(uint16_t width, uint16_t height)
{
	reset(width, height);

	_main_color_texture_sampler = bgfx::createUniform("s_color", bgfx::UniformType::Sampler);
	_main_depth_texture_sampler = bgfx::createUniform("s_main_depth", bgfx::UniformType::Sampler);

	_selection_texture_sampler = bgfx::createUniform("s_selection", bgfx::UniformType::Sampler);
	_selection_depth_texture_sampler = bgfx::createUniform("s_selection_depth", bgfx::UniformType::Sampler);

	_outline_color_uniform = bgfx::createUniform("u_outline_color", bgfx::UniformType::Vec4);

	PosTexCoord0Vertex::init();
}

void Pipeline::destroy()
{
	bgfx::destroy(_outline_color_uniform);

	bgfx::destroy(_selection_depth_texture_sampler);
	bgfx::destroy(_selection_texture_sampler);

	bgfx::destroy(_main_depth_texture_sampler);
	bgfx::destroy(_main_color_texture_sampler);

	bgfx::destroy(_selection_frame_buffer);
	bgfx::destroy(_selection_depth_texture);
	bgfx::destroy(_selection_texture);

	bgfx::destroy(_main_frame_buffer);
	bgfx::destroy(_main_depth_texture);
	bgfx::destroy(_main_color_texture);
}

void Pipeline::reset(u16 width, u16 height)
{
	// Create main frame buffer.
	if (bgfx::isValid(_main_color_texture))
		bgfx::destroy(_main_color_texture);
	_main_color_texture = bgfx::createTexture2D(width
		, height
		, false
		, 1
		, bgfx::TextureFormat::BGRA8
		, BGFX_TEXTURE_RT
		);
	if (bgfx::isValid(_main_depth_texture))
		bgfx::destroy(_main_depth_texture);
	_main_depth_texture = bgfx::createTexture2D(width
		, height
		, false
		, 1
		, bgfx::TextureFormat::D24S8
		, BGFX_TEXTURE_RT
		);
	const bgfx::TextureHandle _main_frame_buffer_attachments[] =
	{
		_main_color_texture,
		_main_depth_texture
	};
	if (bgfx::isValid(_main_frame_buffer))
		bgfx::destroy(_main_frame_buffer);
	_main_frame_buffer = bgfx::createFrameBuffer(countof(_main_frame_buffer_attachments), _main_frame_buffer_attachments);

	// Create outline frame buffer.
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
}

void Pipeline::render(u16 width, u16 height)
{
	const bgfx::Caps *caps = bgfx::getCaps();

	f32 ortho[16];
	bx::mtxOrtho(ortho, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 100.0f, 0.0f, caps->homogeneousDepth, bx::Handedness::Right);

	bgfx::setViewRect(VIEW_BLIT, 0, 0, width, height);
	bgfx::setViewTransform(VIEW_BLIT, NULL, ortho);

	const u32 samplerFlags = 0
		| BGFX_SAMPLER_MIN_POINT
		| BGFX_SAMPLER_MAG_POINT
		| BGFX_SAMPLER_MIP_POINT
		| BGFX_SAMPLER_U_CLAMP
		| BGFX_SAMPLER_V_CLAMP
		;

	bgfx::setTexture(0, _main_color_texture_sampler, _main_color_texture, samplerFlags);
	screenSpaceQuad(width, height, 0.0f, caps->originBottomLeft);
	bgfx::setState(_blit_shader.state);
	bgfx::submit(VIEW_BLIT, _blit_shader.program);

#if !CROWN_PLATFORM_EMSCRIPTEN
	bgfx::setTexture(0, _selection_texture_sampler, _selection_texture, samplerFlags);
	bgfx::setTexture(1, _selection_depth_texture_sampler, _selection_depth_texture, samplerFlags);
	bgfx::setTexture(2, _main_depth_texture_sampler, _main_depth_texture, samplerFlags);
	bgfx::setViewRect(VIEW_OUTLINE, 0, 0, width, height);
	bgfx::setViewTransform(VIEW_OUTLINE, NULL, ortho);
	screenSpaceQuad(width, height, 0.0f, caps->originBottomLeft);
	const f32 outline_color[] = { 1.0f, 0.37f, 0.05f, 1.0f };
	bgfx::setUniform(_outline_color_uniform, outline_color);
	bgfx::setState(_outline_shader.state);
	bgfx::submit(VIEW_OUTLINE, _outline_shader.program);
#endif
}

} // namespace crown
