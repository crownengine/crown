/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

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
		ms_layout
			.begin()
			.add(bgfx::Attrib::Position,  3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			.end();
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
	if (3 == bgfx::getAvailTransientVertexBuffer(3, PosTexCoord0Vertex::ms_layout) )
	{
		bgfx::TransientVertexBuffer tvb;
		bgfx::allocTransientVertexBuffer(&tvb, 3, PosTexCoord0Vertex::ms_layout);
		PosTexCoord0Vertex* vertex = (PosTexCoord0Vertex*)tvb.data;

		const float minx = -_width;
		const float maxx =  _width;
		const float miny = 0.0f;
		const float maxy = _height*2.0f;

		const float texelHalfW = _texelHalf/_textureWidth;
		const float texelHalfH = _texelHalf/_textureHeight;
		const float minu = -1.0f + texelHalfW;
		const float maxu =  1.0f + texelHalfH;

		const float zz = 0.0f;

		float minv = texelHalfH;
		float maxv = 2.0f + texelHalfH;

		if (_originBottomLeft)
		{
			float temp = minv;
			minv = maxv;
			maxv = temp;

			minv -= 1.0f;
			maxv -= 1.0f;
		}

		vertex[0].m_x = minx;
		vertex[0].m_y = miny;
		vertex[0].m_z = zz;
		vertex[0].m_u = minu;
		vertex[0].m_v = minv;

		vertex[1].m_x = maxx;
		vertex[1].m_y = miny;
		vertex[1].m_z = zz;
		vertex[1].m_u = maxu;
		vertex[1].m_v = minv;

		vertex[2].m_x = maxx;
		vertex[2].m_y = maxy;
		vertex[2].m_z = zz;
		vertex[2].m_u = maxu;
		vertex[2].m_v = maxv;

		bgfx::setVertexBuffer(0, &tvb);
	}
}

Pipeline::Pipeline()
	: _frame_buffer(BGFX_INVALID_HANDLE)
{
	for (u32 i = 0; i < countof(_buffers); ++i)
		_buffers[i] = BGFX_INVALID_HANDLE;
}

void Pipeline::create(uint16_t width, uint16_t height)
{
	PosTexCoord0Vertex::init();
	_tex_color = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);

	reset(width, height);
}

void Pipeline::destroy()
{
	bgfx::destroy(_frame_buffer);
	bgfx::destroy(_buffers[1]);
	bgfx::destroy(_buffers[0]);
	bgfx::destroy(_tex_color);
}

void Pipeline::reset(u16 width, u16 height)
{
	for (u32 i = 0; i < countof(_buffers); ++i)
	{
		if (bgfx::isValid(_buffers[i]))
			bgfx::destroy(_buffers[i]);
	}

	_buffers[0] = bgfx::createTexture2D(width
		, height
		, false
		, 1
		, bgfx::TextureFormat::BGRA8
		, BGFX_TEXTURE_RT
		);
	_buffers[1] = bgfx::createTexture2D(width
		, height
		, false
		, 1
		, bgfx::TextureFormat::D24S8
		, BGFX_TEXTURE_RT
		);

	if (bgfx::isValid(_frame_buffer))
		bgfx::destroy(_frame_buffer);

	_frame_buffer = bgfx::createFrameBuffer(countof(_buffers), _buffers);
}

void Pipeline::render(ShaderManager& sm, StringId32 program, uint8_t view, uint16_t width, uint16_t height)
{
	const bgfx::Caps* caps = bgfx::getCaps();

	f32 ortho[16];
	bx::mtxOrtho(ortho, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 100.0f, 0.0f, caps->homogeneousDepth);

	bgfx::setViewClear(view
		, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL
		, 0x353839ff
		, 1.0f
		, 0
		);

	bgfx::setViewFrameBuffer(view, BGFX_INVALID_HANDLE);
	bgfx::setViewRect(view, 0, 0, width, height);
	bgfx::setViewTransform(view, NULL, ortho);

	const u32 samplerFlags = 0
		| BGFX_SAMPLER_MIN_POINT
		| BGFX_SAMPLER_MAG_POINT
		| BGFX_SAMPLER_MIP_POINT
		| BGFX_SAMPLER_U_CLAMP
		| BGFX_SAMPLER_V_CLAMP
		;
	bgfx::setTexture(0, _tex_color, _buffers[0], samplerFlags);
	screenSpaceQuad(width, height, 0.0f, caps->originBottomLeft);
	sm.submit(program, view, 0, BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
}

} // namespace crown
